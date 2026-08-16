#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

// ============================================================================
//  Lock-free single-producer / single-consumer ring buffer.
//  The audio thread pushes mono output samples; the UI thread pulls the most
//  recent block to run an FFT for the spectrum display. No locks, no allocs.
// ============================================================================

namespace ianiboy
{
    class AnalyzerFifo
    {
    public:
        static constexpr int capacity = 1 << 12; // 4096

        void pushMono (const float* const* channels, int numCh, int numSamples)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float m = 0.0f;
                for (int ch = 0; ch < numCh; ++ch) m += channels[ch][i];
                if (numCh > 0) m /= (float) numCh;

                const int slot = writePos.load (std::memory_order_relaxed);
                buffer[(size_t) slot] = m;
                writePos.store ((slot + 1) & (capacity - 1), std::memory_order_release);
            }
        }

        // Copy the newest `n` samples (n <= capacity) into dest, oldest-first.
        void readLatest (float* dest, int n) const
        {
            const int w = writePos.load (std::memory_order_acquire);
            for (int i = 0; i < n; ++i)
            {
                const int idx = (w - n + i + capacity) & (capacity - 1);
                dest[i] = buffer[(size_t) idx];
            }
        }

    private:
        std::array<float, capacity> buffer { {} };
        std::atomic<int> writePos { 0 };
    };
}
