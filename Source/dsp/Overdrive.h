#pragma once

#include "../core/DSPModule.h"
#include <atomic>
#include <cmath>

/**
 * Overdrive/Distortion module with multiple saturation modes
 */
class Overdrive : public DSPModule {
public:
    enum class Mode {
        Soft = 0,      // Soft clipping (tube-like)
        Classic,       // Classic 303 distortion
        Saturated,     // Hard saturation
        Fuzz,          // Fuzzy distortion
        Tape           // Tape saturation
    };

    Overdrive() = default;
    ~Overdrive() override = default;

    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    float processSample(float input) override;

    void setDrive(float amount);      // 1.0 - 10.0
    void setMode(Mode mode);
    void setMode(int index);
    void setMix(float mix);           // 0.0 - 1.0 dry/wet

private:
    float processSoft(float input, float drive);
    float processClassic(float input, float drive);
    float processSaturated(float input, float drive);
    float processFuzz(float input, float drive);
    float processTape(float input, float drive);

    float m_sampleRate = 44100.0f;

    // DC blocker state
    float m_dcIn = 0.0f;
    float m_dcOut = 0.0f;
    static constexpr float DC_COEFF = 0.995f;

    // Parameters
    std::atomic<float> m_drive{1.0f};
    std::atomic<Mode> m_mode{Mode::Classic};
    std::atomic<float> m_mix{1.0f};
};
