#pragma once

#include "../core/DSPModule.h"
#include <atomic>
#include <array>

/**
 * 303 style 4-pole (24dB/octave) resonant lowpass ladder filter
 *
 * Based on the classic Moog ladder filter design used in the 303.
 * Features:
 * - 4-pole (24dB/octave) lowpass response
 * - Self-oscillating resonance
 * - Non-linear saturation for character
 * - Oversampling for improved frequency response
 */
class LadderFilter : public DSPModule {
public:
    LadderFilter();
    ~LadderFilter() override = default;

    // DSPModule interface
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    float processSample(float input) override;

    // Filter parameters
    void setCutoff(float frequencyHz);          // Cutoff frequency in Hz
    void setResonance(float resonance);         // 0.0 to 1.0 (can self-oscillate near 1.0)
    void setEnvelopeAmount(float amount);       // -1.0 to 1.0 envelope modulation depth
    void setEnvelopeValue(float value);         // 0.0 to 1.0 current envelope value

    // Get current cutoff frequency
    float getCutoff() const { return m_targetCutoff.load(std::memory_order_relaxed); }

private:
    // Calculate filter coefficients
    void updateCoefficients();
    float saturate(float input) const;

    // State
    float m_sampleRate = 44100.0f;
    std::array<float, 4> m_stage = {0.0f, 0.0f, 0.0f, 0.0f};  // 4 filter stages
    float m_stageTanh[4] = {0.0f, 0.0f, 0.0f, 0.0f};          // Tanh outputs
    float m_feedback = 0.0f;                                   // Feedback for resonance
    float m_cutoffSmoothed = 1000.0f;                          // Smoothed cutoff

    // Parameters (atomic for thread safety)
    std::atomic<float> m_targetCutoff{1000.0f};
    std::atomic<float> m_resonance{0.0f};
    std::atomic<float> m_envelopeAmount{0.0f};
    std::atomic<float> m_envelopeValue{0.0f};

    // Coefficients
    float m_g = 0.0f;        // Cutoff coefficient
    float m_k = 0.0f;        // Resonance coefficient

    // Constants
    static constexpr float MIN_CUTOFF = 20.0f;     // 20 Hz
    static constexpr float MAX_CUTOFF = 20000.0f;  // 20 kHz
    static constexpr float CUTOFF_SMOOTHING = 0.9995f;
    static constexpr float SATURATION_AMOUNT = 1.5f;
};
