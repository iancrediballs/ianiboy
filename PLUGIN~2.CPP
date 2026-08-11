#include "PluginEditor.h"

using namespace ianiboy;

IaniboyAudioProcessorEditor::IaniboyAudioProcessorEditor (IaniboyAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    auto initCombo = [this] (juce::ComboBox& box, juce::Label& lab, const juce::String& text,
                             const juce::StringArray& items)
    {
        box.addItemList (items, 1);
        addAndMakeVisible (box);
        lab.setText (text, juce::dontSendNotification);
        lab.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (lab);
    };

    // --- presets (host programs) ---
    juce::StringArray presetNames;
    for (const auto& pr : factoryPresets()) presetNames.add (pr.name);
    initCombo (presetBox, presetLabel, "PRESET", presetNames);
    presetBox.setSelectedItemIndex (proc.getCurrentProgram(), juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        proc.setCurrentProgram (presetBox.getSelectedItemIndex());
    };

    // --- band type combos + toggles (attached to APVTS) ---
    initCombo (midTypeBox,  midTypeLabel,  "TYPE", midTypeChoices());
    initCombo (lowModeBox,  lowModeLabel,  "MODE", lowModeChoices());
    initCombo (highTypeBox, highTypeLabel, "TYPE", highTypeChoices());
    initCombo (osBox,       osLabel,       "OVERSAMPLING", osChoices());

    midTypeAtt  = std::make_unique<ComboAtt> (proc.apvts, pid::midType,  midTypeBox);
    lowModeAtt  = std::make_unique<ComboAtt> (proc.apvts, pid::lowMode,  lowModeBox);
    highTypeAtt = std::make_unique<ComboAtt> (proc.apvts, pid::highType, highTypeBox);
    osAtt       = std::make_unique<ComboAtt> (proc.apvts, pid::oversamp, osBox);

    addAndMakeVisible (lowMonoBtn);
    addAndMakeVisible (clipBtn);
    lowMonoAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::lowMono, lowMonoBtn);
    clipAtt    = std::make_unique<ButtonAtt> (proc.apvts, pid::clipOn,  clipBtn);

    // --- knobs ---
    addKnob (pid::inGain,     "INPUT");
    addKnob (pid::outGain,    "OUTPUT");
    addKnob (pid::dryMix,     "DRY INJECT");
    addKnob (pid::lowCross,   "LOW X");
    addKnob (pid::highCross,  "HIGH X");

    addKnob (pid::lowDrive,   "DRIVE");
    addKnob (pid::lowGain,    "GAIN");

    addKnob (pid::midDrive,   "DRIVE");
    addKnob (pid::accentFreq, "ACCENT Hz");
    addKnob (pid::accentDb,   "ACCENT");
    addKnob (pid::midGain,    "GAIN");
    addKnob (pid::tame,       "TAME");
    addKnob (pid::tameFreq,   "TAME Hz");
    addKnob (pid::crushBits,  "BITS");
    addKnob (pid::crushRate,  "RATE");

    addKnob (pid::highDrive,  "DRIVE");
    addKnob (pid::air,        "AIR");
    addKnob (pid::highGain,   "GAIN");

    addKnob (pid::clipCeil,   "CEILING");

    addAndMakeVisible (mascot);

    setResizable (true, true);
    setResizeLimits (820, 520, 1400, 900);
    setSize (940, 600);
    startTimerHz (45);
}

IaniboyAudioProcessorEditor::~IaniboyAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

IaniboyAudioProcessorEditor::Knob& IaniboyAudioProcessorEditor::addKnob (const juce::String& id,
                                                                        const juce::String& text)
{
    auto k = std::make_unique<Knob>();
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 66, 16);
    addAndMakeVisible (k->slider);
    k->label.setText (text, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (k->label);
    k->att = std::make_unique<SliderAtt> (proc.apvts, id, k->slider);
    auto& ref = *k;
    knobs[id] = std::move (k);
    return ref;
}

// -----------------------------------------------------------------------------
void IaniboyAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (colours::bg);

    // header
    g.setColour (colours::orange);
    g.setFont (juce::Font (34.0f, juce::Font::bold));
    g.drawText ("IANIBOY", headerRect.reduced (16, 0).withTrimmedRight (0),
                juce::Justification::centredLeft);
    g.setColour (colours::textDim);
    g.setFont (juce::Font (12.0f, juce::Font::plain));
    g.drawText ("SONIC ARCHITECTURE  //  MULTIBAND CHARACTER ENGINE",
                headerRect.reduced (18, 0).translated (0, 24), juce::Justification::centredLeft);

    auto panel = [&g] (juce::Rectangle<int> r, const juce::String& title)
    {
        g.setColour (colours::panel);
        g.fillRoundedRectangle (r.toFloat(), 8.0f);
        g.setColour (colours::panelEdge);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 8.0f, 1.0f);
        g.setColour (colours::orange);
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        g.drawText (title, r.reduced (10, 6).removeFromTop (18), juce::Justification::topLeft);
    };

    panel (globalRect, "GLOBAL");
    panel (lowRect,  "LOWS  //  MONO ANCHOR");
    panel (midRect,  "MIDS  //  FUZZ CORE");
    panel (highRect, "HIGHS  //  AIR & BITE");
    panel (outRect,  "OUTPUT");
}

void IaniboyAudioProcessorEditor::layoutGroup (juce::Rectangle<int> area, const juce::String&,
                                               const juce::StringArray& ids)
{
    // simple flow layout of knobs inside a panel body
    auto body = area.reduced (12, 28);
    const int kw = 84, kh = 92;
    int x = body.getX(), y = body.getY();
    for (const auto& id : ids)
    {
        auto it = knobs.find (id);
        if (it == knobs.end()) continue;
        if (x + kw > body.getRight()) { x = body.getX(); y += kh; }
        juce::Rectangle<int> cell (x, y, kw, kh);
        it->second->label.setBounds (cell.removeFromTop (16));
        it->second->slider.setBounds (cell);
        x += kw;
    }
}

void IaniboyAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();
    headerRect = r.removeFromTop (58);

    // top strip: preset selector (right of header)
    {
        auto strip = headerRect.reduced (12, 12);
        auto right = strip.removeFromRight (360);
        presetLabel.setBounds (right.removeFromLeft (60));
        presetBox.setBounds (right);
    }

    auto content = r.reduced (12);

    // mascot column on the right
    mascotRect = content.removeFromRight (250);
    mascot.setBounds (mascotRect.reduced (0, 0));
    content.removeFromRight (12);

    // global row across the top of content
    globalRect = content.removeFromTop (128);
    layoutGroup (globalRect, "GLOBAL",
                 { pid::inGain, pid::outGain, pid::dryMix, pid::lowCross, pid::highCross });
    {   // oversampling combo bottom-right of global panel
        auto g = globalRect.reduced (12, 28);
        auto row = g.removeFromBottom (24);
        osBox.setBounds (row.removeFromRight (90));
        osLabel.setBounds (row.removeFromRight (100));
    }

    content.removeFromTop (12);

    // three band panels side by side
    auto bands = content.removeFromTop (juce::jmax (150, content.getHeight() - 96));
    const int bw = (bands.getWidth() - 24) / 3;

    lowRect = bands.removeFromLeft (bw);
    bands.removeFromLeft (12);
    midRect = bands.removeFromLeft (bw);
    bands.removeFromLeft (12);
    highRect = bands;

    layoutGroup (lowRect,  "LOWS",  { pid::lowDrive, pid::lowGain });
    layoutGroup (midRect,  "MIDS",  { pid::midDrive, pid::accentFreq, pid::accentDb,
                                      pid::midGain, pid::tame, pid::tameFreq,
                                      pid::crushBits, pid::crushRate });
    layoutGroup (highRect, "HIGHS", { pid::highDrive, pid::air, pid::highGain });

    // combos/toggles inside band panels (bottom rows)
    {
        auto lo = lowRect.reduced (12); auto row = lo.removeFromBottom (24);
        lowModeLabel.setBounds (row.removeFromLeft (44));
        lowModeBox.setBounds (row.removeFromLeft (100));
        lowMonoBtn.setBounds (row.removeFromRight (70));
    }
    {
        auto mi = midRect.reduced (12); auto row = mi.removeFromBottom (24);
        midTypeLabel.setBounds (row.removeFromLeft (44));
        midTypeBox.setBounds (row.removeFromLeft (120));
    }
    {
        auto hi = highRect.reduced (12); auto row = hi.removeFromBottom (24);
        highTypeLabel.setBounds (row.removeFromLeft (44));
        highTypeBox.setBounds (row.removeFromLeft (110));
    }

    content.removeFromTop (12);

    // output strip
    outRect = content;
    layoutGroup (outRect, "OUTPUT", { pid::clipCeil });
    {
        auto o = outRect.reduced (12, 28);
        clipBtn.setBounds (o.removeFromRight (90).removeFromTop (24));
    }
}

void IaniboyAudioProcessorEditor::timerCallback()
{
    const float dryNorm = proc.apvts.getParameter (pid::dryMix)->getValue(); // 0..1
    mascot.setWetness (dryNorm);
    mascot.setEnergy  (proc.energy.load());
    mascot.punch      (proc.outPeak.load());

    // keep preset box in sync if the host changes program
    const int prog = proc.getCurrentProgram();
    if (presetBox.getSelectedItemIndex() != prog)
        presetBox.setSelectedItemIndex (prog, juce::dontSendNotification);
}
