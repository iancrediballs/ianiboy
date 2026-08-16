#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// ============================================================================
//  Ianiboy parameter definitions (single source of truth for IDs + ranges).
// ============================================================================

namespace ianiboy::pid
{
    // global
    static constexpr auto inGain    = "inGain";
    static constexpr auto outGain   = "outGain";
    static constexpr auto dryMix    = "dryMix";
    static constexpr auto oversamp  = "oversamp";

    // crossover
    static constexpr auto lowCross  = "lowCross";
    static constexpr auto highCross = "highCross";

    // low band
    static constexpr auto lowMode   = "lowMode";
    static constexpr auto lowDrive  = "lowDrive";
    static constexpr auto lowMono   = "lowMono";
    static constexpr auto lowGain   = "lowGain";

    // mid band
    static constexpr auto midType   = "midType";
    static constexpr auto midDrive  = "midDrive";
    static constexpr auto accentFreq= "accentFreq";
    static constexpr auto accentDb  = "accentDb";
    static constexpr auto midGain   = "midGain";
    static constexpr auto tame      = "tame";
    static constexpr auto tameFreq  = "tameFreq";
    static constexpr auto crushBits = "crushBits";
    static constexpr auto crushRate = "crushRate";

    // high band
    static constexpr auto highType  = "highType";
    static constexpr auto highDrive = "highDrive";
    static constexpr auto air       = "air";
    static constexpr auto highGain  = "highGain";

    // output
    static constexpr auto clipOn    = "clipOn";
    static constexpr auto clipCeil  = "clipCeil";

    // per-band solo / mute
    static constexpr auto lowMute   = "lowMute";
    static constexpr auto midMute   = "midMute";
    static constexpr auto highMute  = "highMute";
    static constexpr auto lowSolo   = "lowSolo";
    static constexpr auto midSolo   = "midSolo";
    static constexpr auto highSolo  = "highSolo";
}

namespace ianiboy
{
    inline juce::StringArray midTypeChoices()
    {
        return { "Tape", "Tube", "Hard Clip", "Foldback", "Diode", "Sine Fold", "Bitcrush" };
    }
    inline juce::StringArray lowModeChoices() { return { "Clean", "Soft", "Tape" }; }
    inline juce::StringArray highTypeChoices(){ return { "Tape Air", "Clean Clip" }; }
    inline juce::StringArray osChoices()      { return { "1x", "2x", "4x", "8x" }; }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto db  = [] (float lo, float hi, float /*def*/)
        { return NormalisableRange<float> (lo, hi, 0.01f); };

        // ---- global ----
        p.push_back (std::make_unique<AudioParameterFloat>(ParameterID{pid::inGain,1},  "Input",   db(-24.f,24.f,0.f), 0.0f));
        p.push_back (std::make_unique<AudioParameterFloat>(ParameterID{pid::outGain,1}, "Output",  db(-24.f,24.f,0.f), 0.0f));
        p.push_back (std::make_unique<AudioParameterFloat>(ParameterID{pid::dryMix,1},  "Dry Inject",
                        NormalisableRange<float>(0.f,0.5f,0.001f), 0.0f));
        p.push_back (std::make_unique<AudioParameterChoice>(ParameterID{pid::oversamp,1},"Oversampling", osChoices(), 2));

        // ---- crossover ----
        p.push_back (std::make_unique<AudioParameterFloat>(ParameterID{pid::lowCross,1}, "Low X",
                        NormalisableRange<float>(40.f,400.f,1.f,0.5f), 180.0f));
        p.push_back (std::make_unique<AudioParameterFloat>(ParameterID{pid::highCross,1},"High X",
                        NormalisableRange<float>(1200.f,9000.f,1.f,0.5f), 3000.0f));

        // ---- low band ----
        p.push_back (std::make_unique<AudioParameterChoice>(ParameterID{pid::lowMode,1}, "Low Mode", lowModeChoices(), 2));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::lowDrive,1},"Low Drive",
                        NormalisableRange<float>(1.f,8.f,0.01f,0.5f), 1.2f));
        p.push_back (std::make_unique<AudioParameterBool>  (ParameterID{pid::lowMono,1}, "Low Mono", true));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::lowGain,1}, "Low Gain", db(-24.f,12.f,0.f), 0.0f));

        // ---- mid band ----
        p.push_back (std::make_unique<AudioParameterChoice>(ParameterID{pid::midType,1}, "Mid Type", midTypeChoices(), 0));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::midDrive,1},"Mid Drive",
                        NormalisableRange<float>(1.f,20.f,0.01f,0.4f), 2.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::accentFreq,1},"Accent Freq",
                        NormalisableRange<float>(200.f,3000.f,1.f,0.5f), 800.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::accentDb,1},"Accent",
                        NormalisableRange<float>(0.f,12.f,0.1f), 3.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::midGain,1}, "Mid Gain", db(-24.f,12.f,0.f), 0.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::tame,1},    "Tame",
                        NormalisableRange<float>(0.f,1.f,0.001f), 0.25f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::tameFreq,1},"Tame Freq",
                        NormalisableRange<float>(1500.f,9000.f,1.f,0.5f), 4000.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::crushBits,1},"Crush Bits",
                        NormalisableRange<float>(2.f,16.f,0.1f), 12.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::crushRate,1},"Crush Rate",
                        NormalisableRange<float>(1.f,40.f,0.1f,0.5f), 1.0f));

        // ---- high band ----
        p.push_back (std::make_unique<AudioParameterChoice>(ParameterID{pid::highType,1},"High Type", highTypeChoices(), 0));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::highDrive,1},"High Drive",
                        NormalisableRange<float>(1.f,8.f,0.01f,0.5f), 1.3f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::air,1},     "Air",
                        NormalisableRange<float>(-6.f,12.f,0.1f), 2.0f));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID{pid::highGain,1},"High Gain", db(-24.f,12.f,0.f), 0.0f));

        // ---- output ----
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID{pid::clipOn,1},  "Clipper", true));
        p.push_back (std::make_unique<AudioParameterFloat>(ParameterID{pid::clipCeil,1},"Ceiling",
                        NormalisableRange<float>(-12.f,0.f,0.1f), -0.3f));

        // ---- per-band solo / mute ----
        p.push_back (std::make_unique<AudioParameterBool>(ParameterID{pid::lowMute,1},  "Low Mute",  false));
        p.push_back (std::make_unique<AudioParameterBool>(ParameterID{pid::midMute,1},  "Mid Mute",  false));
        p.push_back (std::make_unique<AudioParameterBool>(ParameterID{pid::highMute,1}, "High Mute", false));
        p.push_back (std::make_unique<AudioParameterBool>(ParameterID{pid::lowSolo,1},  "Low Solo",  false));
        p.push_back (std::make_unique<AudioParameterBool>(ParameterID{pid::midSolo,1},  "Mid Solo",  false));
        p.push_back (std::make_unique<AudioParameterBool>(ParameterID{pid::highSolo,1}, "High Solo", false));

        return { p.begin(), p.end() };
    }
}
