#pragma once

/**
 * Base interface for all DSP processing modules in the 303 Micro Acid plugin.
 *
 * This interface ensures all DSP components follow consistent lifecycle management
 * and maintain real-time safety in the audio processing thread.
 *
 * Thread Safety: All methods MUST be real-time safe (no allocations, no locks)
 */
class DSPModule
{
public:
    virtual ~DSPModule() = default;

    /**
     * Prepares the module for playback at the specified sample rate and block size.
     * This is where you should allocate any buffers and pre-calculate coefficients.
     *
     * Called on the audio thread but NOT during active processing.
     *
     * @param sampleRate The sample rate in Hz (e.g., 44100.0, 48000.0)
     * @param samplesPerBlock Maximum number of samples per process call
     */
    virtual void prepare(double sampleRate, int samplesPerBlock) = 0;

    /**
     * Resets the module's internal state to default values.
     * Clears any delay lines, phase accumulators, filter states, etc.
     *
     * Called when audio processing starts or stops.
     * Must be real-time safe.
     */
    virtual void reset() = 0;

    /**
     * Process a single audio sample.
     * This is the hot path - optimize heavily!
     *
     * @param input Input sample (use 0.0f if module is a generator)
     * @return Processed output sample
     */
    virtual float processSample(float input) = 0;
};
