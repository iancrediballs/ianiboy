#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// ============================================================================
//  Ianiboy look & feel - "Safety Orange on black" from the blueprint deck.
// ============================================================================

namespace ianiboy
{
    namespace colours
    {
        const juce::Colour bg        { 0xff0d0d0f };
        const juce::Colour panel     { 0xff17171b };
        const juce::Colour panelEdge { 0xff2a2a30 };
        const juce::Colour orange     { 0xffff5a1f }; // SAFETY ORANGE
        const juce::Colour orangeDim  { 0xff7a2c10 };
        const juce::Colour text       { 0xffe8e8ec };
        const juce::Colour textDim    { 0xff8a8a92 };
        const juce::Colour skin       { 0xffffe0c4 };
        const juce::Colour hair       { 0xff23140a };
    }

    class IaniboyLNF : public juce::LookAndFeel_V4
    {
    public:
        IaniboyLNF()
        {
            setColour (juce::ResizableWindow::backgroundColourId, colours::bg);
            setColour (juce::Slider::textBoxTextColourId,        colours::text);
            setColour (juce::Slider::textBoxOutlineColourId,     colours::panelEdge);
            setColour (juce::ComboBox::backgroundColourId,       colours::panel);
            setColour (juce::ComboBox::textColourId,             colours::text);
            setColour (juce::ComboBox::outlineColourId,          colours::panelEdge);
            setColour (juce::PopupMenu::backgroundColourId,      colours::panel);
            setColour (juce::Label::textColourId,                colours::textDim);
            setColour (juce::ToggleButton::textColourId,         colours::text);
        }

        void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                               float pos, float startAngle, float endAngle,
                               juce::Slider&) override
        {
            auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
            const auto radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
            const auto cx = b.getCentreX(), cy = b.getCentreY();
            const auto angle = startAngle + pos * (endAngle - startAngle);
            const float track = radius * 0.72f;

            // track
            juce::Path bg;
            bg.addCentredArc (cx, cy, track, track, 0.0f, startAngle, endAngle, true);
            g.setColour (colours::panelEdge);
            g.strokePath (bg, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // value arc
            juce::Path arc;
            arc.addCentredArc (cx, cy, track, track, 0.0f, startAngle, angle, true);
            g.setColour (colours::orange);
            g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // knob body
            g.setColour (colours::panel);
            g.fillEllipse (juce::Rectangle<float> (radius, radius).withCentre ({ cx, cy }).reduced (radius * 0.35f));
            g.setColour (colours::panelEdge);
            g.drawEllipse (juce::Rectangle<float> (radius, radius).withCentre ({ cx, cy }).reduced (radius * 0.35f), 1.0f);

            // pointer
            juce::Point<float> tip (cx + std::cos (angle - juce::MathConstants<float>::halfPi) * track * 0.6f,
                                    cy + std::sin (angle - juce::MathConstants<float>::halfPi) * track * 0.6f);
            g.setColour (colours::orange);
            g.drawLine ({ { cx, cy }, tip }, 2.5f);
        }

        juce::Font getLabelFont (juce::Label&) override
        {
            return juce::Font (12.0f, juce::Font::plain);
        }
    };
}
