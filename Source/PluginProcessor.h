#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "dsp/MultibandEngine.h"
#include "Parameters.h"
#include "Presets.h"

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

    juce::AudioProcessorValueTreeState apvts;

    // Editor peeks these for the mascot / meters (atomic, UI-thread safe)
    std::atomic<float> outPeak { 0.0f };
    std::atomic<float> energy  { 0.0f };   // 0..1 "how wild is the processing"

private:
    void updateEngineParams();

    ianiboy::MultibandEngine engine;
    int currentProgram = 0;
    int lastOsChoice   = -1;
    double lastSampleRate = 44100.0;
    int    lastBlockSize  = 512;
    int    lastChannels   = 2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IaniboyAudioProcessor)
};
