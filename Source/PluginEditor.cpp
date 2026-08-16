#include "PluginEditor.h"

namespace ianiboy
{

// ============================================================================
//  Canvas
// ============================================================================
Canvas::Canvas (IaniboyAudioProcessor& p)
    : proc (p), spectrum (p.analyzer, p.apvts)
{
    // ---- header: preset browser ----
    addAndMakeVisible (spectrum);
    addAndMakeVisible (mascot);
    addAndMakeVisible (inMeter);
    addAndMakeVisible (outMeter);

    refreshPresetList();
    presetBox.setSelectedItemIndex (proc.getCurrentProgram(), juce::dontSendNotification);
    addAndMakeVisible (presetBox);
    presetBox.onChange = [this]
    {
        const int idx = presetBox.getSelectedItemIndex();
        if (idx >= 0) proc.presets.loadPreset (idx);
    };

    auto browseBtn = [this] (juce::TextButton& b)
    {
        b.setColour (juce::TextButton::buttonColourId, colours::panel);
        addAndMakeVisible (b);
    };
    browseBtn (prevBtn); browseBtn (nextBtn); browseBtn (saveBtn);
    prevBtn.onClick = [this]
    {
        int i = juce::jmax (0, presetBox.getSelectedItemIndex() - 1);
        presetBox.setSelectedItemIndex (i);
    };
    nextBtn.onClick = [this]
    {
        int i = juce::jmin (presetBox.getNumItems() - 1, presetBox.getSelectedItemIndex() + 1);
        presetBox.setSelectedItemIndex (i);
    };
    nameField.setTextToShowWhenEmpty ("preset name", colours::textDim);
    nameField.setColour (juce::TextEditor::backgroundColourId, colours::panel);
    nameField.setColour (juce::TextEditor::outlineColourId, colours::panelEdge);
    addAndMakeVisible (nameField);
    saveBtn.onClick = [this]
    {
        const int ni = proc.presets.saveUserPreset (nameField.getText());
        refreshPresetList();
        if (ni >= 0) presetBox.setSelectedItemIndex (ni, juce::dontSendNotification);
        nameField.clear();
    };

    // ---- header: A/B + oversampling ----
    auto abStyle = [this] (juce::TextButton& b, juce::Colour on)
    {
        b.setColour (juce::TextButton::buttonColourId, colours::panel);
        b.setColour (juce::TextButton::buttonOnColourId, on);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
        addAndMakeVisible (b);
    };
    abStyle (abA, colours::orange); abStyle (abB, colours::orange);
    abCopy.setColour (juce::TextButton::buttonColourId, colours::panel);
    addAndMakeVisible (abCopy);
    abA.onClick    = [this] { proc.switchAB (0); };
    abB.onClick    = [this] { proc.switchAB (1); };
    abCopy.onClick = [this] { proc.copyCurrentToOther(); };

    initCombo (osBox, osLabel, "OS", osChoices());
    osAtt = std::make_unique<ComboAtt> (proc.apvts, pid::oversamp, osBox);

    // ---- band type combos ----
    initCombo (lowModeBox,  lowModeLabel,  "MODE", lowModeChoices());
    initCombo (midTypeBox,  midTypeLabel,  "TYPE", midTypeChoices());
    initCombo (highTypeBox, highTypeLabel, "TYPE", highTypeChoices());
    lowModeAtt  = std::make_unique<ComboAtt> (proc.apvts, pid::lowMode,  lowModeBox);
    midTypeAtt  = std::make_unique<ComboAtt> (proc.apvts, pid::midType,  midTypeBox);
    highTypeAtt = std::make_unique<ComboAtt> (proc.apvts, pid::highType, highTypeBox);

    // ---- solo / mute ----
    auto sm = [this] (juce::TextButton& b, juce::Colour on)
    {
        b.setClickingTogglesState (true);
        b.setColour (juce::TextButton::buttonColourId, colours::panel);
        b.setColour (juce::TextButton::buttonOnColourId, on);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
        addAndMakeVisible (b);
    };
    const juce::Colour soloCol (0xffe0c000), muteCol (0xffd0402a);
    sm (lowSolo, soloCol);  sm (lowMuteB, muteCol);
    sm (midSolo, soloCol);  sm (midMuteB, muteCol);
    sm (highSolo, soloCol); sm (highMuteB, muteCol);
    lowSoloAtt  = std::make_unique<ButtonAtt> (proc.apvts, pid::lowSolo,  lowSolo);
    lowMuteAtt  = std::make_unique<ButtonAtt> (proc.apvts, pid::lowMute,  lowMuteB);
    midSoloAtt  = std::make_unique<ButtonAtt> (proc.apvts, pid::midSolo,  midSolo);
    midMuteAtt  = std::make_unique<ButtonAtt> (proc.apvts, pid::midMute,  midMuteB);
    highSoloAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::highSolo, highSolo);
    highMuteAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::highMute, highMuteB);

    addAndMakeVisible (lowMonoBtn);
    addAndMakeVisible (clipBtn);
    lowMonoAtt = std::make_unique<ButtonAtt> (proc.apvts, pid::lowMono, lowMonoBtn);
    clipAtt    = std::make_unique<ButtonAtt> (proc.apvts, pid::clipOn,  clipBtn);

    // ---- knobs ----
    addKnob (pid::inGain,   "INPUT");
    addKnob (pid::outGain,  "OUTPUT");
    addKnob (pid::dryMix,   "DRY INJECT");
    addKnob (pid::lowCross, "LOW X");
    addKnob (pid::highCross,"HIGH X");
    addKnob (pid::lowDrive, "DRIVE");
    addKnob (pid::lowGain,  "GAIN");
    addKnob (pid::midDrive, "DRIVE");
    addKnob (pid::accentFreq,"ACCENT Hz");
    addKnob (pid::accentDb, "ACCENT");
    addKnob (pid::midGain,  "GAIN");
    addKnob (pid::tame,     "TAME");
    addKnob (pid::tameFreq, "TAME Hz");
    addKnob (pid::crushBits,"BITS");
    addKnob (pid::crushRate,"RATE");
    addKnob (pid::highDrive,"DRIVE");
    addKnob (pid::air,      "AIR");
    addKnob (pid::highGain, "GAIN");
    addKnob (pid::clipCeil, "CEILING");

    grLabel.setJustificationType (juce::Justification::centredLeft);
    grLabel.setColour (juce::Label::textColourId, colours::textDim);
    addAndMakeVisible (grLabel);

    startTimerHz (30);
}

void Canvas::refreshPresetList()
{
    presetBox.clear (juce::dontSendNotification);
    const auto names = proc.presets.allNames();
    const int nFactory = proc.presets.numFactory();
    for (int i = 0; i < names.size(); ++i)
    {
        if (i == nFactory && i < names.size())
            presetBox.addSeparator();
        presetBox.addItem (names[i], i + 1);   // item IDs are 1-based
    }
}

Canvas::Knob& Canvas::addKnob (const juce::String& id, const juce::String& text)
{
    auto k = std::make_unique<Knob>();
    k->slider.setColour (juce::Slider::textBoxTextColourId, colours::text);
    k->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (k->slider);
    k->caption.setText (text, juce::dontSendNotification);
    k->caption.setJustificationType (juce::Justification::centred);
    k->caption.setColour (juce::Label::textColourId, colours::textDim);
    addAndMakeVisible (k->caption);
    k->att = std::make_unique<SliderAtt> (proc.apvts, id, k->slider);
    auto& ref = *k;
    knobs[id] = std::move (k);
    return ref;
}

void Canvas::placeKnob (const juce::String& id, juce::Rectangle<int> cell)
{
    auto it = knobs.find (id);
    if (it == knobs.end()) return;
    auto c = cell.reduced (4);
    it->second->caption.setBounds (c.removeFromTop (14));
    const int tb = juce::jmin (76, c.getWidth() - 2);
    it->second->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, tb, 16);
    it->second->slider.setBounds (c);
}

void Canvas::layoutKnobRow (juce::Rectangle<int> area, const juce::StringArray& ids, int perRow)
{
    if (ids.isEmpty() || perRow <= 0) return;
    const int cw = area.getWidth() / perRow;
    for (int i = 0; i < ids.size(); ++i)
    {
        juce::Rectangle<int> cell (area.getX() + (i % perRow) * cw, area.getY(), cw, area.getHeight());
        placeKnob (ids[i], cell);
    }
}

void Canvas::initCombo (juce::ComboBox& box, juce::Label& lab, const juce::String& text,
                        const juce::StringArray& items)
{
    box.addItemList (items, 1);
    addAndMakeVisible (box);
    lab.setText (text, juce::dontSendNotification);
    lab.setColour (juce::Label::textColourId, colours::textDim);
    lab.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (lab);
}

// ---- layout ---------------------------------------------------------------
void Canvas::resized()
{
    auto full = juce::Rectangle<int> (0, 0, designW, designH).reduced (16);

    headerR = full.removeFromTop (60);
    full.removeFromTop (12);

    auto row1 = full.removeFromTop (176);
    mascotR = row1.removeFromRight (272);
    row1.removeFromRight (12);
    specR = row1;
    full.removeFromTop (16);

    auto rowB = full.removeFromTop (250);
    meterR = rowB.removeFromLeft (54);
    rowB.removeFromLeft (16);
    lowR = rowB.removeFromLeft (300);
    rowB.removeFromLeft (12);
    highR = rowB.removeFromRight (244);
    rowB.removeFromRight (12);
    midR = rowB;
    full.removeFromTop (16);

    auto rowC = full;
    outR = rowC.removeFromRight (392);
    rowC.removeFromRight (16);
    globalR = rowC;

    spectrum.setBounds (specR);
    mascot.setBounds (mascotR);

    // header controls
    {
        auto h = headerR;
        h.removeFromLeft (230);                       // logo space (painted)
        auto center = h.removeFromLeft (520).withSizeKeepingCentre (520, 28);
        prevBtn.setBounds (center.removeFromLeft (28));   center.removeFromLeft (4);
        presetBox.setBounds (center.removeFromLeft (220)); center.removeFromLeft (4);
        nextBtn.setBounds (center.removeFromLeft (28));    center.removeFromLeft (10);
        nameField.setBounds (center.removeFromLeft (150)); center.removeFromLeft (6);
        saveBtn.setBounds (center.removeFromLeft (64));

        auto right = h.withSizeKeepingCentre (h.getWidth(), 28);
        abA.setBounds (right.removeFromLeft (32));    right.removeFromLeft (4);
        abB.setBounds (right.removeFromLeft (32));    right.removeFromLeft (4);
        abCopy.setBounds (right.removeFromLeft (52));
        osBox.setBounds (right.removeFromRight (78)); right.removeFromRight (4);
        osLabel.setBounds (right.removeFromRight (28));
    }

    // meters
    {
        auto m = meterR.reduced (2);
        const int mw = (m.getWidth() - 6) / 2;
        inMeter.setBounds (m.removeFromLeft (mw));
        m.removeFromLeft (6);
        outMeter.setBounds (m.removeFromLeft (mw));
    }

    auto bandTop = [] (juce::Rectangle<int>& p, juce::Label& lab, juce::ComboBox& box,
                       int labW, int boxW, juce::TextButton& solo, juce::TextButton& mute)
    {
        auto top = p.removeFromTop (26);
        lab.setBounds (top.removeFromLeft (labW));
        box.setBounds (top.removeFromLeft (boxW));
        mute.setBounds (top.removeFromRight (28)); top.removeFromRight (4);
        solo.setBounds (top.removeFromRight (28));
    };

    // LOW
    {
        auto p = lowR.reduced (12); p.removeFromTop (22);
        bandTop (p, lowModeLabel, lowModeBox, 40, 130, lowSolo, lowMuteB);
        p.removeFromTop (6);
        layoutKnobRow (p.removeFromTop (110), { pid::lowDrive, pid::lowGain }, 2);
        p.removeFromTop (4);
        lowMonoBtn.setBounds (p.removeFromTop (24).removeFromLeft (100));
    }
    // MID
    {
        auto p = midR.reduced (12); p.removeFromTop (22);
        bandTop (p, midTypeLabel, midTypeBox, 40, 150, midSolo, midMuteB);
        p.removeFromTop (6);
        layoutKnobRow (p.removeFromTop (100), { pid::midDrive, pid::accentFreq, pid::accentDb, pid::midGain }, 4);
        p.removeFromTop (2);
        layoutKnobRow (p.removeFromTop (100), { pid::tame, pid::tameFreq, pid::crushBits, pid::crushRate }, 4);
    }
    // HIGH
    {
        auto p = highR.reduced (12); p.removeFromTop (22);
        bandTop (p, highTypeLabel, highTypeBox, 40, 96, highSolo, highMuteB);
        p.removeFromTop (6);
        layoutKnobRow (p.removeFromTop (100), { pid::highDrive, pid::air }, 2);
        p.removeFromTop (2);
        layoutKnobRow (p.removeFromTop (100), { pid::highGain }, 2);
    }
    // GLOBAL
    {
        auto p = globalR.reduced (12); p.removeFromTop (20);
        layoutKnobRow (p.removeFromTop (106),
                       { pid::inGain, pid::outGain, pid::dryMix, pid::lowCross, pid::highCross }, 5);
    }
    // OUTPUT
    {
        auto p = outR.reduced (12); p.removeFromTop (20);
        auto knobCell = p.removeFromLeft (130);
        placeKnob (pid::clipCeil, knobCell.removeFromTop (106));
        auto rightCol = p.reduced (8, 0);
        clipBtn.setBounds (rightCol.removeFromTop (30).removeFromLeft (110));
        rightCol.removeFromTop (10);
        grLabel.setBounds (rightCol.removeFromTop (24));
    }
}

// ---- paint ----------------------------------------------------------------
void Canvas::drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title)
{
    g.setColour (colours::panel);
    g.fillRoundedRectangle (r.toFloat(), 8.0f);
    g.setColour (colours::panelEdge);
    g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 8.0f, 1.0f);
    g.setColour (colours::orange);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (title, r.reduced (12, 8).removeFromTop (16), juce::Justification::topLeft);
}

void Canvas::paint (juce::Graphics& g)
{
    g.fillAll (colours::bg);

    // header brand
    g.setColour (colours::orange);
    g.setFont (juce::Font (30.0f, juce::Font::bold));
    g.drawText ("IANIBOY", headerR.withWidth (220).withTrimmedLeft (2),
                juce::Justification::centredLeft);
    g.setColour (colours::textDim);
    g.setFont (juce::Font (9.5f, juce::Font::plain));
    g.drawText ("SONIC ARCHITECTURE", headerR.withWidth (220).withTrimmedLeft (3).translated (1, 20),
                juce::Justification::centredLeft);

    drawPanel (g, lowR,    "LOWS  ·  MONO ANCHOR");
    drawPanel (g, midR,    "MIDS  ·  FUZZ CORE");
    drawPanel (g, highR,   "HIGHS  ·  AIR & BITE");
    drawPanel (g, globalR, "GLOBAL");
    drawPanel (g, outR,    "OUTPUT");
}

// ---- live updates ---------------------------------------------------------
void Canvas::timerCallback()
{
    spectrum.setSampleRate (proc.getSampleRate());

    inMeter.setLevel  (proc.inPeak.load());
    outMeter.setLevel (proc.outPeak.load());

    if (auto* dry = proc.apvts.getParameter (pid::dryMix))
        mascot.setWetness (dry->getValue());
    mascot.setEnergy (proc.energy.load());
    mascot.punch (proc.outPeak.load());

    const float gr = proc.clipGRdb.load();
    grLabel.setText ("GR  " + (gr < -0.05f ? juce::String (gr, 1) + " dB" : "0.0 dB"),
                     juce::dontSendNotification);

    abA.setToggleState (proc.getABSlot() == 0, juce::dontSendNotification);
    abB.setToggleState (proc.getABSlot() == 1, juce::dontSendNotification);
}

// ============================================================================
//  Editor (scales the Canvas to fit the window)
// ============================================================================
Editor::Editor (IaniboyAudioProcessor& p)
    : AudioProcessorEditor (&p), canvas (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (canvas);
    setResizable (true, true);
    setResizeLimits (795, 540, 1590, 1080);
    getConstrainer()->setFixedAspectRatio ((double) Canvas::designW / (double) Canvas::designH);
    setSize (1060, 720);
}

Editor::~Editor() { setLookAndFeel (nullptr); }

void Editor::paint (juce::Graphics& g) { g.fillAll (colours::bg); }

void Editor::resized()
{
    auto b = getLocalBounds().toFloat();
    const float s = juce::jmin (b.getWidth()  / (float) Canvas::designW,
                                b.getHeight() / (float) Canvas::designH);
    canvas.setBounds (0, 0, Canvas::designW, Canvas::designH);
    const float w = Canvas::designW * s, h = Canvas::designH * s;
    canvas.setTransform (juce::AffineTransform::scale (s)
                            .translated ((b.getWidth() - w) * 0.5f, (b.getHeight() - h) * 0.5f));
}

} // namespace ianiboy
