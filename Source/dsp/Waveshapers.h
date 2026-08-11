#pragma once
#include <cmath>
#include <algorithm>

// ============================================================================
//  Ianiboy waveshaper library
//  Pure, stateless, per-sample transfer functions (except Bitcrusher which
//  holds sample-and-hold state). All functions expect input roughly in
//  [-1, 1] after a drive gain has been applied upstream, and stay bounded.
//
//  Distortion "character" per source material, from the Sonic Architecture
//  Blueprint + the Gemini signal-chain notes:
//    - Tube      : asymmetric soft knee  -> vocals, bass warmth
//    - Tape      : symmetric soft sat     -> lows, glue, gentle highs
//    - HardClip  : brickwall              -> drum transients, loudness
//    - Foldback  : wavefolder             -> synth screeches / stabs
//    - Diode     : asymmetric rectifier   -> industrial grit
//    - Sine      : sinusoidal fold        -> metallic FM-like edge
//    - Bitcrush  : bit + rate reduction   -> top-end digital sizzle
// ============================================================================

namespace ianiboy::shapers
{
    // ---- soft symmetric (tape / general glue) -----------------------------
    inline float tape (float x) noexcept
    {
        // tanh gives a smooth, symmetric soft-clip with musical odd harmonics.
        return std::tanh (x);
    }

    // ---- asymmetric tube --------------------------------------------------
    // Adds even harmonics via a DC-biased asymmetric curve. The positive and
    // negative halves saturate differently, like a triode.
    inline float tube (float x) noexcept
    {
        const float bias = 0.15f;
        float y = x + bias;
        y = y / (1.0f + std::abs (y));         // soft asymmetric divider
        return y - (bias / (1.0f + bias));     // remove the added DC offset
    }

    // ---- hard clip --------------------------------------------------------
    inline float hardClip (float x) noexcept
    {
        return std::clamp (x, -1.0f, 1.0f);
    }

    // ---- foldback wavefolder (synth screech) ------------------------------
    // Reflects the signal back on itself past +/-1 instead of clipping,
    // multiplying harmonics violently. Great for leads and stabs.
    inline float foldback (float x) noexcept
    {
        const float threshold = 1.0f;
        while (x > threshold || x < -threshold)
        {
            if (x > threshold)   x = 2.0f * threshold - x;
            if (x < -threshold)  x = -2.0f * threshold - x;
        }
        return x;
    }

    // ---- diode / asymmetric rectifier -------------------------------------
    inline float diode (float x) noexcept
    {
        // Positive half saturates soft, negative half is squashed harder.
        if (x >= 0.0f) return std::tanh (x);
        return 0.6f * std::tanh (1.6f * x);
    }

    // ---- sinusoidal fold --------------------------------------------------
    inline float sineFold (float x) noexcept
    {
        // Below unity behaves near-linear; above it, sin() folds the peaks.
        return std::sin (x * 1.5707963f); // sin(x * pi/2): unity slope at 0, folds past |x|>1
    }

    // ---- generic dispatch -------------------------------------------------
    enum class Type { Tape = 0, Tube, HardClip, Foldback, Diode, Sine, Bitcrush };

    inline float shape (Type t, float x) noexcept
    {
        switch (t)
        {
            case Type::Tube:     return tube (x);
            case Type::HardClip: return hardClip (x);
            case Type::Foldback: return foldback (x);
            case Type::Diode:    return diode (x);
            case Type::Sine:     return sineFold (x);
            case Type::Tape:
            default:             return tape (x);
        }
    }

    // ------------------------------------------------------------------------
    //  Bitcrusher: bit-depth quantisation + sample-rate reduction.
    //  Stateful (per channel) — used for the "digital sizzle" parallel blend.
    // ------------------------------------------------------------------------
    struct Bitcrusher
    {
        void reset() noexcept { phase = 0.0f; hold = 0.0f; }

        // bits in [1..16], downsample in [1..50] (1 = no rate reduction)
        float process (float x, float bits, float downsample) noexcept
        {
            const float step = std::pow (2.0f, bits) - 1.0f;

            phase += 1.0f;
            if (phase >= downsample)
            {
                phase -= downsample;
                // quantise to the reduced bit depth
                hold = std::round ((x * 0.5f + 0.5f) * step) / step * 2.0f - 1.0f;
            }
            return hold;
        }

        float phase { 0.0f };
        float hold  { 0.0f };
    };
}
