#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/MultibandEngine.h"
#include "dsp/AnalyzerFifo.h"
#include "Parameters.h"
#include "Presets.h"
#include "PresetManager.h"

class IaniboyAudioProcessor : public juce::AudioProcessor
{
public:
    IaniboyAudioProcessor();
    ~IaniboyAudioProcessor() override = default;

    // -- AudioProcessor ------------------------------------------------------
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                        { return true;  }

    const juce::String getName() const override            { return "Ianiboy"; }
    bool acceptsMidi()  const override                     { return false; }
    bool producesMidi() const override                     { return false; }
    bool isMidiEffect() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0.0;   }

    // -- programs (factory presets exposed to the host) ----------------------
    int getNumPrograms() override                          { return (int) ianiboy::factoryPresets().size(); }
    int getCurrentProgram() override                       { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    // -- state ---------------------------------------------------------------
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // -- A/B compare ---------------------------------------------------------
    void switchAB (int slot);                 // 0 = A, 1 = B
    void copyCurrentToOther();                // duplicate active slot into the other
    int  getABSlot() const { return abCurrent; }

    juce::AudioProcessorValueTreeState apvts;
    ianiboy::PresetManager presets { apvts };
    ianiboy::AnalyzerFifo  analyzer;          // spectrum feed (UI reads)

    // Editor peeks these (atomic, UI-thread safe)
    std::atomic<float> inPeak   { 0.0f };
    std::atomic<float> outPeak  { 0.0f };
    std::atomic<float> clipGRdb { 0.0f };
    std::atomic<float> energy   { 0.0f };     // mascot "how wild" 0..1

private:
    void updateEngineParams();

    ianiboy::MultibandEngine engine;
    int currentProgram = 0;
    int lastOsChoice   = -1;
    double lastSampleRate = 44100.0;
    int    lastBlockSize  = 512;
    int    lastChannels   = 2;

    juce::ValueTree abState[2];
    int abCurrent = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IaniboyAudioProcessor)
};
