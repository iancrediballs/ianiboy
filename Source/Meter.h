#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

// ============================================================================
//  Vertical level meter with smoothed fall and a peak-hold cap.
//  Feed it a linear peak each UI frame via setLevel().
// ============================================================================

namespace ianiboy
{
    class Meter : public juce::Component
    {
    public:
        explicit Meter (juce::String caption = {}) : label (std::move (caption)) {}

        void setLevel (float linearPeak)
        {
            const float db = juce::Decibels::gainToDecibels (linearPeak, -60.0f);
            const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
            level = juce::jmax (norm, level * 0.85f);       // fast attack, smooth decay
            if (norm >= peak) { peak = norm; peakHold = 30; }
            else if (--peakHold <= 0) peak = juce::jmax (0.0f, peak - 0.01f);
            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto labelArea = r.removeFromBottom (14.0f);

            g.setColour (colours::bg);
            g.fillRoundedRectangle (r, 3.0f);

            auto fill = r.reduced (2.0f);
            const float h = fill.getHeight() * level;
            auto bar = fill.removeFromBottom (h);

            juce::ColourGradient grad (juce::Colour (0xff35d07f), bar.getX(), bar.getBottom(),
                                       colours::orange, bar.getX(), r.getY(), false);
            grad.addColour (0.75, juce::Colour (0xffe0c000));
            g.setGradientFill (grad);
            g.fillRoundedRectangle (bar, 2.0f);

            // peak cap
            const float py = r.getBottom() - 2.0f - r.getHeight() * peak;
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.fillRect (r.getX() + 2.0f, py, r.getWidth() - 4.0f, 1.5f);

            g.setColour (colours::panelEdge);
            g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

            g.setColour (colours::textDim);
            g.setFont (juce::Font (10.0f, juce::Font::plain));
            g.drawText (label, labelArea, juce::Justification::centred);
        }

    private:
        juce::String label;
        float level = 0.0f, peak = 0.0f;
        int   peakHold = 0;
    };
}
