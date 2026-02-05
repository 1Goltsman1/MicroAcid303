#pragma once

#include "../core/DSPModule.h"
#include <atomic>

/**
 * ADSR Envelope Generator for 303 style synthesis
 *
 * Implements:
 * - Attack, Decay, Sustain, Release (ADSR)
 * - Exponential curves for natural sound
 * - Retrigger support for fast note sequences
 */
class Envelope : public DSPModule {
public:
    enum class Stage {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    Envelope();
    ~Envelope() override = default;

    // DSPModule interface
    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    float processSample(float input) override;

    // Envelope control
    void noteOn();
    void noteOff();

    // Parameter setters (in seconds)
    void setAttack(float timeSeconds);
    void setDecay(float timeSeconds);
    void setSustain(float level);  // 0.0 to 1.0
    void setRelease(float timeSeconds);

    // Query current state
    Stage getCurrentStage() const { return m_stage.load(std::memory_order_relaxed); }
    float getCurrentLevel() const { return m_level; }
    bool isActive() const { return m_stage.load(std::memory_order_relaxed) != Stage::Idle; }

private:
    float calculateCoefficient(float timeSeconds) const;

    // State
    float m_sampleRate = 44100.0f;
    float m_level = 0.0f;
    std::atomic<Stage> m_stage{Stage::Idle};

    // Parameters (atomic for thread safety)
    std::atomic<float> m_attackTime{0.001f};   // 1ms default
    std::atomic<float> m_decayTime{0.3f};      // 300ms default
    std::atomic<float> m_sustainLevel{0.7f};   // 70% default
    std::atomic<float> m_releaseTime{0.3f};    // 300ms default

    // Coefficients for exponential curves
    float m_attackCoeff = 0.0f;
    float m_decayCoeff = 0.0f;
    float m_releaseCoeff = 0.0f;

    // Constants
    static constexpr float MIN_TIME = 0.001f;  // 1ms minimum
    static constexpr float MAX_TIME = 10.0f;   // 10s maximum
    static constexpr float EPSILON = 0.001f;   // Threshold for stage transitions
};
