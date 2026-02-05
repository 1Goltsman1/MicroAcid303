#pragma once

#include "../core/DSPModule.h"
#include <atomic>
#include <cmath>

/**
 * Band-limited oscillator using PolyBLEP anti-aliasing
 * 303 style bass synthesis
 */
class Oscillator : public DSPModule {
public:
    enum class Waveform {
        Sawtooth,
        Square
    };

    Oscillator();
    ~Oscillator() override = default;

    // DSPModule interface
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    float processSample(float input) override;

    // Oscillator-specific methods
    void setFrequency(float frequencyHz);
    void setWaveform(Waveform waveform);
    void setFineTune(float cents); // ±50 cents

private:
    // PolyBLEP anti-aliasing
    float polyBLEP(float t, float dt) const;

    // State
    float m_sampleRate = 44100.0f;
    float m_phase = 0.0f;
    float m_phaseIncrement = 0.0f;
    float m_frequencySmoothing = 440.0f;

    // Parameters (atomic for thread safety)
    std::atomic<float> m_targetFrequency{440.0f};
    std::atomic<Waveform> m_waveform{Waveform::Sawtooth};
    std::atomic<float> m_fineTuneCents{0.0f};

    // Constants
    static constexpr float SMOOTHING_COEFF = 0.999f;
};
