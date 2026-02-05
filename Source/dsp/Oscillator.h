#pragma once

#include "../core/DSPModule.h"
#include <atomic>
#include <cmath>
#include <random>

/**
 * Band-limited oscillator with 12 waveforms
 * 303 style bass synthesis with extended capabilities
 */
class Oscillator : public DSPModule {
public:
    enum class Waveform {
        Sawtooth = 0,
        Square,
        Triangle,
        Sine,
        Pulse25,      // 25% pulse width
        Pulse12,      // 12.5% pulse width
        SuperSaw,     // Detuned saw stack
        Noise,        // White noise
        SawSquare,    // Saw + Square mix
        TriSaw,       // Triangle + Saw mix
        SyncSaw,      // Hard sync sawtooth
        FM            // FM synthesis
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
    void setWaveform(int index);
    void setFineTune(float cents);
    void setSlideTime(float seconds);

private:
    // Waveform generators
    float generateSawtooth();
    float generateSquare();
    float generateTriangle();
    float generateSine();
    float generatePulse(float width);
    float generateSuperSaw();
    float generateNoise();
    float generateSawSquare();
    float generateTriSaw();
    float generateSyncSaw();
    float generateFM();

    // PolyBLEP anti-aliasing
    float polyBLEP(float t, float dt) const;

    // State
    float m_sampleRate = 44100.0f;
    float m_phase = 0.0f;
    float m_phaseIncrement = 0.0f;
    float m_frequencySmoothing = 440.0f;
    float m_slideCoeff = 0.999f;

    // SuperSaw detuned phases
    float m_superSawPhases[7] = {0};
    float m_superSawDetune[7] = {-0.11f, -0.06f, -0.02f, 0.0f, 0.02f, 0.06f, 0.11f};

    // Sync oscillator
    float m_syncPhase = 0.0f;
    float m_syncRatio = 2.5f;

    // FM synthesis
    float m_modPhase = 0.0f;
    float m_fmIndex = 3.0f;
    float m_fmRatio = 2.0f;

    // Noise generator
    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_noiseDist{-1.0f, 1.0f};

    // Parameters (atomic for thread safety)
    std::atomic<float> m_targetFrequency{440.0f};
    std::atomic<Waveform> m_waveform{Waveform::Sawtooth};
    std::atomic<float> m_fineTuneCents{0.0f};
    std::atomic<float> m_slideTime{0.1f};

    // Constants
    static constexpr float TWO_PI = 6.283185307179586f;
};
