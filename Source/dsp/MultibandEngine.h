#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <memory>
#include "Waveshapers.h"

// ============================================================================
//  Ianiboy MultibandEngine
//  The heart of the plugin. Realises the Sonic Architecture Blueprint:
//
//    input trim
//      -> UPSAMPLE (oversampled nonlinear domain, anti-aliased)
//           -> 3-band Linkwitz-Riley split (phase-compensated low band)
//                LOW  : subsonic brickwall, strict-mono, clean/soft/tape
//                MID  : accent pre-EQ -> waveshaper -> dynamic "tame" de-harsh
//                HIGH : tape/clip air + bite, high shelf
//           -> recombine
//      -> DOWNSAMPLE
//    + parallel DRY INJECTION (delay-compensated to the OS latency)
//    -> output clipper (loudness density) -> output trim
//
//  Everything nonlinear happens oversampled so aliasing never reaches the
//  mixdown. The dry-inject delay line is matched to the reported latency so
//  the raw transient locks in phase with the saturated body (no comb filter).
// ============================================================================

namespace ianiboy
{
    struct EngineParams
    {
        float inGain      = 1.0f;   // linear
        float outGain     = 1.0f;   // linear

        float lowCross    = 180.0f; // Hz
        float highCross   = 3000.0f;// Hz

        int   lowMode     = 2;      // 0 clean, 1 soft, 2 tape
        float lowDrive    = 1.0f;
        bool  lowMono     = true;
        float lowGain     = 1.0f;

        int   midType     = 0;      // shapers::Type
        float midDrive    = 1.0f;
        float accentFreq  = 800.0f;
        float accentDb    = 0.0f;
        float midGain     = 1.0f;
        float tameAmount  = 0.0f;   // 0..1 dynamic de-harsh depth
        float tameFreq    = 4000.0f;
        float crushBits   = 12.0f;
        float crushRate   = 1.0f;

        int   highType    = 0;      // 0 tape, 1 clean clip
        float highDrive   = 1.0f;
        float airDb       = 0.0f;
        float highGain    = 1.0f;

        float dryMix      = 0.0f;   // 0..1 parallel dry injection
        bool  clipOn      = true;
        float clipCeil    = 0.98f;  // linear ceiling

        // per-band enable (1 = on, 0 = off) resolved from solo/mute upstream
        float lowActive   = 1.0f;
        float midActive   = 1.0f;
        float highActive  = 1.0f;

        bool  autoGain    = false;  // match output loudness back to input
    };

    class MultibandEngine
    {
    public:
        MultibandEngine() = default;

        // factorLog2: 0->1x, 1->2x, 2->4x, 3->8x
        void prepare (double sampleRate, int samplesPerBlock, int numChannels, int factorLog2)
        {
            baseSampleRate = sampleRate;
            channels       = juce::jlimit (1, 2, numChannels);
            osLog2         = juce::jlimit (0, 3, factorLog2);

            oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
                (size_t) channels, (size_t) osLog2,
                juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
            oversampler->initProcessing ((size_t) samplesPerBlock);
            oversampler->reset();

            osSampleRate = baseSampleRate * (double) (1 << osLog2);

            // Prepare the crossover + band filters at the OVERSAMPLED rate.
            juce::dsp::ProcessSpec osSpec { osSampleRate,
                                            (juce::uint32) (samplesPerBlock * (1 << osLog2)),
                                            (juce::uint32) channels };

            lrLow.prepare  (osSpec);
            lrHigh.prepare (osSpec);
            lowAP.prepare  (osSpec);
            lrLow.setType  (juce::dsp::LinkwitzRileyFilterType::lowpass);
            lrHigh.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
            lowAP.setType  (juce::dsp::LinkwitzRileyFilterType::allpass);

            for (int ch = 0; ch < 2; ++ch)
            {
                subsonic[ch].prepare (osSpec);
                accent[ch].prepare   (osSpec);
                tameBP[ch].prepare   (osSpec);
                air[ch].prepare      (osSpec);
                crusher[ch].reset();
                tameEnv[ch] = 0.0f;
            }

            // Dry-injection delay: match the oversampler's reported latency
            // (referred to the BASE sample rate) so wet/dry stay phase-locked.
            const int latency = (int) std::ceil (oversampler->getLatencyInSamples());
            latencySamples = latency;

            dryDelay.prepare ({ baseSampleRate,
                                (juce::uint32) samplesPerBlock,
                                (juce::uint32) channels });
            dryDelay.setMaximumDelayInSamples (juce::jmax (1, latency + 4));
            dryDelay.setDelay ((float) latency);
            dryDelay.reset();

            updateFilters();
            reset();
        }

        void reset()
        {
            lrLow.reset(); lrHigh.reset(); lowAP.reset();
            for (int ch = 0; ch < 2; ++ch)
            {
                subsonic[ch].reset();
                accent[ch].reset();
                tameBP[ch].reset();
                air[ch].reset();
                crusher[ch].reset();
                tameEnv[ch] = 0.0f;
            }
            dryDelay.reset();
        }

        int getLatencySamples() const noexcept { return latencySamples; }

        void setParams (const EngineParams& p) { params = p; updateFilters(); }

        // --- main block process ------------------------------------------------
        void process (juce::AudioBuffer<float>& buffer)
        {
            const int numCh = juce::jmin (channels, buffer.getNumChannels());
            const int n     = buffer.getNumSamples();

            // measure true input loudness (pre-gain) for the Auto Gain matcher
            float inRMS = 0.0f;
            for (int ch = 0; ch < numCh; ++ch) inRMS += buffer.getRMSLevel (ch, 0, n);
            if (numCh > 0) inRMS /= (float) numCh;

            // 1) input trim
            buffer.applyGain (params.inGain);

            // 2) capture dry, run it through the compensating delay line
            juce::AudioBuffer<float> dry;
            dry.makeCopyOf (buffer);
            {
                juce::dsp::AudioBlock<float> db (dry);
                juce::dsp::ProcessContextReplacing<float> ctx (db);
                dryDelay.process (ctx);
            }

            // 3) oversample the wet path and run the nonlinear multiband engine
            juce::dsp::AudioBlock<float> block (buffer);
            auto osBlock = oversampler->processSamplesUp (block);

            const int osN = (int) osBlock.getNumSamples();
            processOversampled (osBlock, numCh, osN);

            oversampler->processSamplesDown (block);

            // 4) parallel dry injection (blueprint: 10-15% raw transient energy)
            //    The clean, delay-aligned transient is ADDED on top of the
            //    fully saturated body so the punch returns without dulling fuzz.
            if (params.dryMix > 0.0f)
                for (int ch = 0; ch < numCh; ++ch)
                    buffer.addFrom (ch, 0, dry, ch, 0, n, params.dryMix);

            // 5) output clipper -> output trim (track peak gain reduction)
            float maxOver = 0.0f;
            for (int ch = 0; ch < numCh; ++ch)
            {
                auto* d = buffer.getWritePointer (ch);
                for (int i = 0; i < n; ++i)
                {
                    float x = d[i] * params.outGain;
                    if (params.clipOn)
                    {
                        const float a = std::abs (x);
                        if (a > params.clipCeil)
                            maxOver = juce::jmax (maxOver, a - params.clipCeil);
                        x = juce::jlimit (-params.clipCeil, params.clipCeil, x);
                    }
                    d[i] = x;
                }
            }
            // reduction in dB: how far the loudest peak was shaved
            clipReductionDb = (maxOver > 0.0f && params.clipCeil > 0.0f)
                ? juce::Decibels::gainToDecibels (params.clipCeil / (params.clipCeil + maxOver))
                : 0.0f;

            // 6) Auto Gain: glide output loudness back to the input loudness
            if (params.autoGain)
            {
                float outRMS = 0.0f;
                for (int ch = 0; ch < numCh; ++ch) outRMS += buffer.getRMSLevel (ch, 0, n);
                if (numCh > 0) outRMS /= (float) numCh;

                if (outRMS > 1.0e-6f && inRMS > 1.0e-6f)
                {
                    const float target = juce::jlimit (0.0625f, 16.0f, inRMS / outRMS); // +/-24 dB
                    smoothedAuto += 0.10f * (target - smoothedAuto);
                }
                buffer.applyGain (smoothedAuto);
                autoGainDb = juce::Decibels::gainToDecibels (smoothedAuto);
            }
            else
            {
                smoothedAuto = 1.0f;
                autoGainDb   = 0.0f;
            }
        }

        float getClipReductionDb() const noexcept { return clipReductionDb; }
        float getAutoGainDb()     const noexcept { return autoGainDb; }

    private:
        // ---- per-block coefficient refresh ------------------------------------
        void updateFilters()
        {
            if (osSampleRate <= 0.0) return;

            lrLow.setCutoffFrequency  (juce::jlimit (20.0f,  (float) osSampleRate * 0.45f, params.lowCross));
            lrHigh.setCutoffFrequency (juce::jlimit (params.lowCross + 20.0f, (float) osSampleRate * 0.45f, params.highCross));
            lowAP.setCutoffFrequency  (juce::jlimit (params.lowCross + 20.0f, (float) osSampleRate * 0.45f, params.highCross));

            for (int ch = 0; ch < 2; ++ch)
            {
                subsonic[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass (osSampleRate, 20.0);
                accent[ch].coefficients   = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                                                osSampleRate,
                                                juce::jlimit (100.0, osSampleRate * 0.45, (double) params.accentFreq),
                                                1.0, juce::Decibels::decibelsToGain (params.accentDb));
                tameBP[ch].coefficients   = juce::dsp::IIR::Coefficients<float>::makeBandPass (
                                                osSampleRate,
                                                juce::jlimit (500.0, osSampleRate * 0.45, (double) params.tameFreq), 2.0);
                air[ch].coefficients      = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                                                osSampleRate,
                                                juce::jlimit (3000.0, osSampleRate * 0.45, 8000.0),
                                                0.7, juce::Decibels::decibelsToGain (params.airDb));
            }
        }

        // ---- oversampled inner loop -------------------------------------------
        void processOversampled (juce::dsp::AudioBlock<float>& osBlock, int numCh, int osN)
        {
            const auto midType  = (shapers::Type) params.midType;
            const bool doCrush  = (midType == shapers::Type::Bitcrush);

            // sample-outer / channel-inner so the low band can be summed to mono
            for (int i = 0; i < osN; ++i)
            {
                float lowCh[2] = { 0.0f, 0.0f };
                float midCh[2] = { 0.0f, 0.0f };
                float hiCh[2]  = { 0.0f, 0.0f };

                for (int ch = 0; ch < numCh; ++ch)
                {
                    const float in = osBlock.getSample (ch, i);

                    float low = 0.0f, rest = 0.0f, mid = 0.0f, high = 0.0f;
                    lrLow.processSample  (ch, in,   low, rest);
                    lrHigh.processSample (ch, rest, mid, high);
                    low = lowAP.processSample (ch, low); // phase-align low band

                    // ---- LOW: subsonic brickwall + gentle character ----------
                    low = subsonic[ch].processSample (low);
                    if (params.lowMode == 1)      low = shapers::tape (low * params.lowDrive) ;
                    else if (params.lowMode == 2)  low = shapers::tube (low * params.lowDrive);
                    else                           low = low * params.lowDrive;
                    low *= params.lowGain;

                    // ---- MID: accent -> shaper -> dynamic tame ---------------
                    mid = accent[ch].processSample (mid);
                    float driven = mid * params.midDrive;
                    float shaped = doCrush
                                     ? crusher[ch].process (driven, params.crushBits, params.crushRate)
                                     : shapers::shape (midType, driven);

                    // dynamic de-harsh: subtract a level-tracked band of fuzz
                    if (params.tameAmount > 0.0f)
                    {
                        const float bp   = tameBP[ch].processSample (shaped);
                        const float rect = std::abs (bp);
                        // one-pole envelope (fast attack, medium release)
                        const float a = (rect > tameEnv[ch]) ? 0.5f : 0.02f;
                        tameEnv[ch] += a * (rect - tameEnv[ch]);
                        const float duck = params.tameAmount * juce::jmin (1.0f, tameEnv[ch] * 3.0f);
                        shaped -= duck * bp;
                    }
                    mid = shaped * params.midGain;

                    // ---- HIGH: air + bite ------------------------------------
                    float h = high * params.highDrive;
                    h = (params.highType == 1) ? shapers::hardClip (h) : shapers::tape (h);
                    h = air[ch].processSample (h);
                    high = h * params.highGain;

                    lowCh[ch] = low; midCh[ch] = mid; hiCh[ch] = high;
                }

                // strict-mono low band (blueprint: lows collapsed for weight)
                if (params.lowMono && numCh == 2)
                {
                    const float m = 0.5f * (lowCh[0] + lowCh[1]);
                    lowCh[0] = lowCh[1] = m;
                }

                for (int ch = 0; ch < numCh; ++ch)
                    osBlock.setSample (ch, i,   lowCh[ch] * params.lowActive
                                              + midCh[ch] * params.midActive
                                              + hiCh[ch]  * params.highActive);
            }
        }

        // ---- state ------------------------------------------------------------
        EngineParams params;
        double baseSampleRate = 44100.0, osSampleRate = 176400.0;
        int channels = 2, osLog2 = 2, latencySamples = 0;
        float clipReductionDb = 0.0f;
        float smoothedAuto = 1.0f, autoGainDb = 0.0f;

        std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

        juce::dsp::LinkwitzRileyFilter<float> lrLow, lrHigh, lowAP;
        std::array<juce::dsp::IIR::Filter<float>, 2> subsonic, accent, tameBP, air;
        std::array<shapers::Bitcrusher, 2> crusher;
        std::array<float, 2> tameEnv { { 0.0f, 0.0f } };

        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> dryDelay { 64 };
    };
}
