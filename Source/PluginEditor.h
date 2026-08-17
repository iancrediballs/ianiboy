#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "Mascot.h"
#include "Meter.h"
#include "SpectrumComponent.h"

// ============================================================================
//  The UI is laid out at a fixed DESIGN size inside an inner "canvas" that is
//  scaled to fit the actual window. That means nothing ever clips (looking at
//  you, Ceiling), and the plugin resizes crisply to any size the user drags.
// ============================================================================

namespace ianiboy
{
    class Canvas : public juce::Component, private juce::Timer
    {
    public:
        static constexpr int designW = 1060;
        static constexpr int designH = 700;

        explicit Canvas (IaniboyAudioProcessor&);

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        using APVTS     = juce::AudioProcessorValueTreeState;
        using SliderAtt = APVTS::SliderAttachment;
        using ComboAtt  = APVTS::ComboBoxAttachment;
        using ButtonAtt = APVTS::ButtonAttachment;

        struct Knob
        {
            juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag,
                                  juce::Slider::TextBoxBelow };
            juce::Label  caption;
            std::unique_ptr<SliderAtt> att;
        };

        Knob& addKnob (const juce::String& id, const juce::String& text);
        void  placeKnob (const juce::String& id, juce::Rectangle<int> cell);
        void  layoutKnobRow (juce::Rectangle<int> area, const juce::StringArray& ids, int perRow);

        void  initCombo (juce::ComboBox&, juce::Label&, const juce::String& text, const juce::StringArray&);
        void  makeToggle (juce::TextButton&, const juce::String& text);
        void  refreshPresetList();
        void  timerCallback() override;

        void  drawPanel (juce::Graphics&, juce::Rectangle<int>, const juce::String& title);

        IaniboyAudioProcessor& proc;

        std::map<juce::String, std::unique_ptr<Knob>> knobs;

        // header
        juce::ComboBox presetBox;
        juce::TextButton prevBtn { "<" }, nextBtn { ">" }, saveBtn { "SAVE" };
        juce::TextEditor nameField;
        juce::TextButton abA { "A" }, abB { "B" }, abCopy { "A>B" };
        juce::ComboBox osBox; juce::Label osLabel;

        // band combos + toggles
        juce::ComboBox lowModeBox, midTypeBox, highTypeBox;
        juce::Label    lowModeLabel, midTypeLabel, highTypeLabel;
        juce::TextButton lowSolo{"S"}, lowMuteB{"M"}, midSolo{"S"}, midMuteB{"M"}, highSolo{"S"}, highMuteB{"M"};
        juce::ToggleButton lowMonoBtn { "Mono" }, clipBtn { "Clip" }, autoBtn { "Auto Gain" };

        std::unique_ptr<ComboAtt>  lowModeAtt, midTypeAtt, highTypeAtt, osAtt;
        std::unique_ptr<ButtonAtt> lowSoloAtt, lowMuteAtt, midSoloAtt, midMuteAtt,
                                   highSoloAtt, highMuteAtt, lowMonoAtt, clipAtt, autoAtt;

        // visuals
        SpectrumComponent spectrum;
        Meter inMeter { "IN" }, outMeter { "OUT" };
        juce::Label grLabel;
        IaniMascot mascot;

        // section rects (design space) for paint()
        juce::Rectangle<int> headerR, specR, mascotR, lowR, midR, highR, globalR, outR, meterR;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Canvas)
    };

    // ------------------------------------------------------------------------
    class Editor : public juce::AudioProcessorEditor
    {
    public:
        explicit Editor (IaniboyAudioProcessor&);
        ~Editor() override;
        void resized() override;
        void paint (juce::Graphics&) override;

    private:
        IaniboyLNF lnf;
        Canvas canvas;
    };
}

// alias the processor expects
using IaniboyAudioProcessorEditor = ianiboy::Editor;
