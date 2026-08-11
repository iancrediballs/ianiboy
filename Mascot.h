#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "LookAndFeel.h"

// ============================================================================
//  IaniMascot - the animated chibi of Ian.
//  Inspired by Dada Life "Eternal Happiness": as you push Dry -> Wet the
//  character crouches then leaps, arms fly up and the grin blows wide open.
//  Output-peak "punches" kick an extra bounce so he jumps on the transients.
//
//  Everything is vector-drawn, so it's featherweight and ships in the binary
//  with zero image assets. Feed it two live values each frame:
//    setWetness(0..1)  - Dry/Wet position (drives the leap + smile)
//    punch(peak)       - call with the output peak; loud hits add bounce
// ============================================================================

namespace ianiboy
{
    class IaniMascot : public juce::Component, private juce::Timer
    {
    public:
        IaniMascot() { startTimerHz (60); }

        void setWetness (float w) { target = juce::jlimit (0.0f, 1.0f, w); }
        void setEnergy  (float e) { energy = juce::jlimit (0.0f, 1.0f, e); }

        // Call with the current output magnitude; a rising edge adds a hop.
        void punch (float peak)
        {
            const float d = peak - lastPeak;
            if (d > 0.06f) vel -= d * 4.0f;   // upward kick (screen y is inverted)
            lastPeak = peak * 0.7f + lastPeak * 0.3f;
        }

        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();

            // glowing floor + backdrop that brightens with energy
            g.setColour (colours::panel);
            g.fillRoundedRectangle (r, 10.0f);
            {
                juce::ColourGradient grad (colours::orange.withAlpha (0.10f + 0.25f * energy),
                                           r.getCentreX(), r.getBottom(),
                                           colours::panel.withAlpha (0.0f),
                                           r.getCentreX(), r.getY(), false);
                g.setGradientFill (grad);
                g.fillRoundedRectangle (r, 10.0f);
            }
            g.setColour (colours::panelEdge);
            g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.0f);

            const float smile = juce::jlimit (0.0f, 1.0f, target);           // grin openness
            const float leap  = juce::jlimit (0.0f, 1.4f, pos);              // leap amount
            const float squash= juce::jlimit (-0.25f, 0.25f, -vel * 0.4f);   // anticipation crouch

            // layout box for the character
            auto box = r.reduced (16.0f);
            const float unit = juce::jmin (box.getWidth(), box.getHeight()) * 0.5f;
            const float groundY = box.getBottom() - unit * 0.15f;
            const float jumpPix  = leap * unit * 0.85f;
            const float cx = box.getCentreX();
            const float cy = groundY - unit * 0.75f - jumpPix;

            // little shadow that shrinks as he rises
            g.setColour (juce::Colours::black.withAlpha (0.35f * (1.0f - leap * 0.6f)));
            g.fillEllipse (juce::Rectangle<float> (unit * (0.9f - leap * 0.3f), unit * 0.18f)
                              .withCentre ({ cx, groundY + unit * 0.05f }));

            const float bodyH = unit * (0.95f + squash);
            const float bodyW = unit * (0.62f - squash * 0.6f);

            // --- body / hoodie ---
            juce::Path body;
            body.addRoundedRectangle (cx - bodyW * 0.5f, cy + unit * 0.15f, bodyW, bodyH, bodyW * 0.35f);
            g.setColour (colours::orange);
            g.fillPath (body);
            g.setColour (colours::orange.darker (0.4f));
            g.strokePath (body, juce::PathStrokeType (2.0f));

            // "IB" chest mark
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.setFont (juce::Font (unit * 0.22f, juce::Font::bold));
            g.drawText ("IB", juce::Rectangle<float> (cx - bodyW * 0.5f, cy + unit * 0.35f, bodyW, unit * 0.4f),
                        juce::Justification::centred);

            // --- arms (raise with the leap/wetness) ---
            const float armRaise = juce::jmax (leap, smile);
            const float shoulderY = cy + unit * 0.28f;
            const float handOutX  = bodyW * 0.5f + unit * 0.28f;
            const float handY     = shoulderY - armRaise * unit * 0.75f;
            g.setColour (colours::orange.brighter (0.1f));
            g.drawLine (cx - bodyW * 0.4f, shoulderY, cx - handOutX, handY, unit * 0.13f);
            g.drawLine (cx + bodyW * 0.4f, shoulderY, cx + handOutX, handY, unit * 0.13f);
            g.setColour (colours::skin);
            g.fillEllipse (juce::Rectangle<float> (unit * 0.16f, unit * 0.16f).withCentre ({ cx - handOutX, handY }));
            g.fillEllipse (juce::Rectangle<float> (unit * 0.16f, unit * 0.16f).withCentre ({ cx + handOutX, handY }));

            // --- head ---
            const float headR = unit * 0.52f;
            juce::Rectangle<float> head (headR * 2.0f, headR * 2.0f);
            head.setCentre ({ cx, cy - unit * 0.15f });
            g.setColour (colours::skin);
            g.fillEllipse (head);

            // hair (spiky)
            juce::Path hair;
            const float hx = head.getCentreX(), hy = head.getY();
            hair.startNewSubPath (hx - headR, head.getCentreY() - headR * 0.1f);
            hair.lineTo (hx - headR * 0.95f, hy - headR * 0.15f);
            for (int i = -3; i <= 3; ++i)
            {
                const float fx = hx + (float) i / 3.0f * headR * 0.95f;
                hair.lineTo (fx - headR * 0.14f, hy + headR * 0.08f);
                hair.lineTo (fx, hy - headR * 0.28f - std::abs ((float) i) * headR * 0.02f);
            }
            hair.lineTo (hx + headR * 0.95f, hy - headR * 0.15f);
            hair.lineTo (hx + headR, head.getCentreY() - headR * 0.1f);
            hair.closeSubPath();
            g.setColour (colours::hair);
            g.fillPath (hair);

            // --- eyes: open dots when dry, happy ^^ arcs when wet ---
            const float eyeY = head.getCentreY() - headR * 0.05f;
            const float eyeDX = headR * 0.42f;
            g.setColour (juce::Colours::black);
            if (smile < 0.5f)
            {
                const float er = headR * 0.12f * (1.0f - smile);
                g.fillEllipse (juce::Rectangle<float> (er * 2, er * 2).withCentre ({ cx - eyeDX, eyeY }));
                g.fillEllipse (juce::Rectangle<float> (er * 2, er * 2).withCentre ({ cx + eyeDX, eyeY }));
            }
            else
            {
                for (float s : { -1.0f, 1.0f })
                {
                    juce::Path e;
                    const float ex = cx + s * eyeDX;
                    e.startNewSubPath (ex - headR * 0.14f, eyeY);
                    e.lineTo (ex, eyeY - headR * 0.13f);
                    e.lineTo (ex + headR * 0.14f, eyeY);
                    g.strokePath (e, juce::PathStrokeType (headR * 0.06f, juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
                }
            }

            // --- mouth: grin that opens with wetness ---
            const float mouthY = head.getCentreY() + headR * 0.42f;
            const float mouthW = headR * (0.35f + 0.5f * smile);
            const float mouthH = headR * (0.08f + 0.45f * smile);
            juce::Path mouth;
            mouth.startNewSubPath (cx - mouthW, mouthY);
            mouth.quadraticTo (cx, mouthY + mouthH * 2.0f, cx + mouthW, mouthY);
            if (smile > 0.35f) // open, filled grin
            {
                mouth.quadraticTo (cx, mouthY + mouthH * 0.6f, cx - mouthW, mouthY);
                mouth.closeSubPath();
                g.setColour (juce::Colour (0xff5a1020));
                g.fillPath (mouth);
                // tongue
                g.setColour (juce::Colour (0xffff6f8a));
                g.fillEllipse (juce::Rectangle<float> (mouthW * 1.1f, mouthH * 0.9f)
                                  .withCentre ({ cx, mouthY + mouthH * 0.9f }));
            }
            g.setColour (juce::Colours::black);
            g.strokePath (mouth, juce::PathStrokeType (headR * 0.05f));

            // rosy cheeks when very happy
            if (smile > 0.6f)
            {
                g.setColour (juce::Colour (0xffff8a5a).withAlpha (0.5f));
                g.fillEllipse (juce::Rectangle<float> (headR * 0.28f, headR * 0.18f)
                                  .withCentre ({ cx - headR * 0.55f, mouthY - headR * 0.02f }));
                g.fillEllipse (juce::Rectangle<float> (headR * 0.28f, headR * 0.18f)
                                  .withCentre ({ cx + headR * 0.55f, mouthY - headR * 0.02f }));
            }

            // sparkles at full wet
            if (smile > 0.8f)
            {
                g.setColour (colours::orange.withAlpha (0.9f));
                for (int i = 0; i < 5; ++i)
                {
                    const float a = sparkPhase + i * juce::MathConstants<float>::twoPi / 5.0f;
                    const float sx = cx + std::cos (a) * unit * 1.2f;
                    const float sy = (cy - unit * 0.15f) + std::sin (a) * unit * 0.9f;
                    drawSparkle (g, sx, sy, unit * 0.09f);
                }
            }
        }

    private:
        static void drawSparkle (juce::Graphics& g, float x, float y, float s)
        {
            juce::Path p;
            p.startNewSubPath (x, y - s);
            p.lineTo (x + s * 0.25f, y - s * 0.25f);
            p.lineTo (x + s, y);
            p.lineTo (x + s * 0.25f, y + s * 0.25f);
            p.lineTo (x, y + s);
            p.lineTo (x - s * 0.25f, y + s * 0.25f);
            p.lineTo (x - s, y);
            p.lineTo (x - s * 0.25f, y - s * 0.25f);
            p.closeSubPath();
            g.fillPath (p);
        }

        void timerCallback() override
        {
            // spring toward the target leap height (critically-ish damped)
            const float k = 0.18f, damp = 0.78f;
            vel += (target - pos) * k;
            vel *= damp;
            pos += vel;
            pos = juce::jlimit (-0.2f, 1.6f, pos);
            sparkPhase += 0.08f;
            repaint();
        }

        float target { 0.0f }, pos { 0.0f }, vel { 0.0f };
        float energy { 0.0f }, lastPeak { 0.0f }, sparkPhase { 0.0f };
    };
}
