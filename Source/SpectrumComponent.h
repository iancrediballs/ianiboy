#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include "dsp/AnalyzerFifo.h"
#include "Parameters.h"
#include "LookAndFeel.h"

// ============================================================================
//  Live spectrum analyzer with the Low/Mid/High crossover points drawn on it,
//  so you can literally see the 3-band split move as you turn the X knobs.
// ============================================================================

namespace ianiboy
{
    class SpectrumComponent : public juce::Component, private juce::Timer
    {
    public:
        SpectrumComponent (AnalyzerFifo& f, juce::AudioProcessorValueTreeState& s)
            : fifo (f), apvts (s)
        {
            scope.fill (-100.0f);
            startTimerHz (30);
        }

        void setSampleRate (double sr) { sampleRate = sr > 0 ? sr : 44100.0; }

        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat().reduced (1.0f);
            g.setColour (colours::bg);
            g.fillRoundedRectangle (r, 6.0f);

            drawGrid (g, r);
            drawCurve (g, r);
            drawCrossovers (g, r);

            g.setColour (colours::panelEdge);
            g.drawRoundedRectangle (r, 6.0f, 1.0f);
        }

    private:
        static constexpr int fftOrder = 11;
        static constexpr int fftSize  = 1 << fftOrder;      // 2048

        float freqToX (float freq, juce::Rectangle<float> r) const
        {
            const float lo = std::log10 (20.0f), hi = std::log10 (20000.0f);
            const float t = (std::log10 (juce::jlimit (20.0f, 20000.0f, freq)) - lo) / (hi - lo);
            return r.getX() + t * r.getWidth();
        }

        float dbToY (float db, juce::Rectangle<float> r) const
        {
            const float t = juce::jlimit (0.0f, 1.0f, (db + 90.0f) / 90.0f); // -90..0 dB
            return r.getBottom() - t * r.getHeight();
        }

        void drawGrid (juce::Graphics& g, juce::Rectangle<float> r)
        {
            g.setColour (colours::panelEdge.withAlpha (0.5f));
            for (float f : { 100.f, 1000.f, 10000.f })
            {
                const float x = freqToX (f, r);
                g.drawVerticalLine ((int) x, r.getY(), r.getBottom());
            }
            for (int d = -75; d <= -15; d += 15)
            {
                const float y = dbToY ((float) d, r);
                g.drawHorizontalLine ((int) y, r.getX(), r.getRight());
            }
        }

        void drawCurve (juce::Graphics& g, juce::Rectangle<float> r)
        {
            juce::Path p;
            bool started = false;
            const int w = (int) r.getWidth();
            for (int px = 0; px <= w; ++px)
            {
                const float x    = r.getX() + (float) px;
                const float lo   = std::log10 (20.0f), hi = std::log10 (20000.0f);
                const float freq = std::pow (10.0f, lo + (hi - lo) * (float) px / (float) w);
                int bin = (int) std::round (freq * (float) fftSize / (float) sampleRate);
                bin = juce::jlimit (1, fftSize / 2 - 1, bin);
                const float y = dbToY (scope[(size_t) bin], r);
                if (! started) { p.startNewSubPath (x, y); started = true; }
                else            p.lineTo (x, y);
            }

            auto fillP = p;
            fillP.lineTo (r.getRight(), r.getBottom());
            fillP.lineTo (r.getX(),     r.getBottom());
            fillP.closeSubPath();
            g.setColour (colours::orange.withAlpha (0.15f));
            g.fillPath (fillP);

            g.setColour (colours::orange);
            g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved));
        }

        void drawCrossovers (juce::Graphics& g, juce::Rectangle<float> r)
        {
            auto line = [&] (const char* id, const juce::String& text)
            {
                if (auto* v = apvts.getRawParameterValue (id))
                {
                    const float x = freqToX (v->load(), r);
                    g.setColour (colours::text.withAlpha (0.6f));
                    float dashes[] = { 3.0f, 3.0f };
                    g.drawDashedLine ({ { x, r.getY() }, { x, r.getBottom() } }, dashes, 2, 1.0f);
                    g.setColour (colours::textDim);
                    g.setFont (juce::Font (10.0f, juce::Font::plain));
                    g.drawText (text, (int) x - 24, (int) r.getY() + 2, 48, 12, juce::Justification::centred);
                }
            };
            line (pid::lowCross,  "LOW|MID");
            line (pid::highCross, "MID|HIGH");
        }

        void timerCallback() override
        {
            fifo.readLatest (fftData.data(), fftSize);
            window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
            fft.performFrequencyOnlyForwardTransform (fftData.data());

            for (int i = 0; i < fftSize / 2; ++i)
            {
                const float mag = fftData[(size_t) i] / (float) (fftSize / 2);
                float db = juce::Decibels::gainToDecibels (mag, -100.0f);
                // smooth: fast rise, slow fall for a readable display
                scope[(size_t) i] = (db > scope[(size_t) i])
                    ? db
                    : scope[(size_t) i] * 0.9f + db * 0.1f;
            }
            repaint();
        }

        AnalyzerFifo& fifo;
        juce::AudioProcessorValueTreeState& apvts;
        double sampleRate = 44100.0;

        juce::dsp::FFT fft { fftOrder };
        juce::dsp::WindowingFunction<float> window { (size_t) fftSize,
            juce::dsp::WindowingFunction<float>::hann };
        std::array<float, (size_t) fftSize * 2> fftData { {} };
        std::array<float, (size_t) fftSize / 2> scope   { {} };
    };
}
