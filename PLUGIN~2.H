#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "Mascot.h"

class IaniboyAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit IaniboyAudioProcessorEditor (IaniboyAudioProcessor&);
    ~IaniboyAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS   = juce::AudioProcessorValueTreeState;
    using SliderAtt = APVTS::SliderAttachment;
    using ComboAtt  = APVTS::ComboBoxAttachment;
    using ButtonAtt = APVTS::ButtonAttachment;

    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag,
                              juce::Slider::TextBoxBelow };
        juce::Label  label;
        std::unique_ptr<SliderAtt> att;
    };

    Knob& addKnob (const juce::String& id, const juce::String& text);
    void  layoutGroup (juce::Rectangle<int> area, const juce::String& title,
                       const juce::StringArray& ids);
    void  timerCallback() override;

    IaniboyAudioProcessor& proc;
    ianiboy::IaniboyLNF lnf;
    ianiboy::IaniMascot mascot;

    std::map<juce::String, std::unique_ptr<Knob>> knobs;

    // combos / toggles
    juce::ComboBox presetBox, midTypeBox, lowModeBox, highTypeBox, osBox;
    juce::Label    presetLabel, midTypeLabel, lowModeLabel, highTypeLabel, osLabel;
    juce::ToggleButton lowMonoBtn { "Mono" }, clipBtn { "Clipper" };

    std::unique_ptr<ComboAtt>  midTypeAtt, lowModeAtt, highTypeAtt, osAtt;
    std::unique_ptr<ButtonAtt> lowMonoAtt, clipAtt;

    // section rectangles filled in resized(), used by paint()
    juce::Rectangle<int> globalRect, lowRect, midRect, highRect, outRect, mascotRect, headerRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IaniboyAudioProcessorEditor)
};
