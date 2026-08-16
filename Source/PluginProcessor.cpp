#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace ianiboy;

IaniboyAudioProcessor::IaniboyAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "IaniboyState", createParameterLayout())
{
    // seed both A/B slots with the initial state
    abState[0] = apvts.copyState().createCopy();
    abState[1] = apvts.copyState().createCopy();
}

bool IaniboyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void IaniboyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;
    lastBlockSize  = samplesPerBlock;
    lastChannels   = getTotalNumOutputChannels();

    const int osChoice = (int) apvts.getRawParameterValue (pid::oversamp)->load();
    lastOsChoice = osChoice;

    engine.prepare (sampleRate, samplesPerBlock, lastChannels, osChoice);
    setLatencySamples (engine.getLatencySamples());
    updateEngineParams();
}

void IaniboyAudioProcessor::updateEngineParams()
{
    auto raw = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    auto dbToLin = [] (float db) { return juce::Decibels::decibelsToGain (db); };

    EngineParams p;
    p.inGain     = dbToLin (raw (pid::inGain));
    p.outGain    = dbToLin (raw (pid::outGain));
    p.dryMix     = raw (pid::dryMix);

    p.lowCross   = raw (pid::lowCross);
    p.highCross  = raw (pid::highCross);

    p.lowMode    = (int) raw (pid::lowMode);
    p.lowDrive   = raw (pid::lowDrive);
    p.lowMono    = raw (pid::lowMono) > 0.5f;
    p.lowGain    = dbToLin (raw (pid::lowGain));

    p.midType    = (int) raw (pid::midType);
    p.midDrive   = raw (pid::midDrive);
    p.accentFreq = raw (pid::accentFreq);
    p.accentDb   = raw (pid::accentDb);
    p.midGain    = dbToLin (raw (pid::midGain));
    p.tameAmount = raw (pid::tame);
    p.tameFreq   = raw (pid::tameFreq);
    p.crushBits  = raw (pid::crushBits);
    p.crushRate  = raw (pid::crushRate);

    p.highType   = (int) raw (pid::highType);
    p.highDrive  = raw (pid::highDrive);
    p.airDb      = raw (pid::air);
    p.highGain   = dbToLin (raw (pid::highGain));

    p.clipOn     = raw (pid::clipOn) > 0.5f;
    p.clipCeil   = dbToLin (raw (pid::clipCeil));

    // resolve solo / mute into per-band enable flags
    const bool lS = raw (pid::lowSolo)  > 0.5f;
    const bool mS = raw (pid::midSolo)  > 0.5f;
    const bool hS = raw (pid::highSolo) > 0.5f;
    const bool anySolo = lS || mS || hS;
    if (anySolo)
    {
        p.lowActive  = lS ? 1.0f : 0.0f;
        p.midActive  = mS ? 1.0f : 0.0f;
        p.highActive = hS ? 1.0f : 0.0f;
    }
    else
    {
        p.lowActive  = raw (pid::lowMute)  > 0.5f ? 0.0f : 1.0f;
        p.midActive  = raw (pid::midMute)  > 0.5f ? 0.0f : 1.0f;
        p.highActive = raw (pid::highMute) > 0.5f ? 0.0f : 1.0f;
    }

    engine.setParams (p);

    // UI "energy" heuristic for the mascot: how hard are we pushing?
    const float driveNorm = juce::jlimit (0.0f, 1.0f, (p.midDrive - 1.0f) / 19.0f);
    const float wetNorm   = juce::jlimit (0.0f, 1.0f, p.dryMix / 0.5f);
    energy.store (juce::jlimit (0.0f, 1.0f, 0.6f * driveNorm + 0.4f * wetNorm));
}

void IaniboyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // If the oversampling choice changed, re-prepare (allocates; host thread
    // tolerant since it's rare and user-driven).
    const int osChoice = (int) apvts.getRawParameterValue (pid::oversamp)->load();
    if (osChoice != lastOsChoice)
    {
        lastOsChoice = osChoice;
        engine.prepare (lastSampleRate, lastBlockSize, lastChannels, osChoice);
        setLatencySamples (engine.getLatencySamples());
    }

    // input meter (before processing)
    float inMag = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        inMag = juce::jmax (inMag, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    inPeak.store (inMag);

    updateEngineParams();
    engine.process (buffer);

    // output meter + clipper gain reduction
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, buffer.getNumSamples()));
    outPeak.store (peak);
    clipGRdb.store (engine.getClipReductionDb());

    // feed the spectrum analyzer (UI reads asynchronously)
    if (buffer.getNumChannels() > 0)
        analyzer.pushMono (buffer.getArrayOfReadPointers(),
                           buffer.getNumChannels(), buffer.getNumSamples());
}

// ---- A/B compare ------------------------------------------------------------
void IaniboyAudioProcessor::switchAB (int slot)
{
    slot = juce::jlimit (0, 1, slot);
    if (slot == abCurrent) return;
    abState[abCurrent] = apvts.copyState().createCopy();   // stash current
    abCurrent = slot;
    if (abState[slot].isValid())
        apvts.replaceState (abState[slot].createCopy());
}

void IaniboyAudioProcessor::copyCurrentToOther()
{
    abState[1 - abCurrent] = apvts.copyState().createCopy();
}

// ---- programs ---------------------------------------------------------------
void IaniboyAudioProcessor::setCurrentProgram (int index)
{
    currentProgram = juce::jlimit (0, getNumPrograms() - 1, index);
    applyPreset (apvts, currentProgram);
}

const juce::String IaniboyAudioProcessor::getProgramName (int index)
{
    const auto& list = factoryPresets();
    if (index >= 0 && index < (int) list.size())
        return list[(size_t) index].name;
    return {};
}

// ---- state ------------------------------------------------------------------
void IaniboyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void IaniboyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* IaniboyAudioProcessor::createEditor()
{
    return new IaniboyAudioProcessorEditor (*this);
}

// ---- JUCE factory -----------------------------------------------------------
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IaniboyAudioProcessor();
}
