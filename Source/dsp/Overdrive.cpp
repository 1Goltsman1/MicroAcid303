#include "Overdrive.h"
#include <algorithm>

void Overdrive::prepare(double sampleRate, int samplesPerBlock)
{
    m_sampleRate = static_cast<float>(sampleRate);
    reset();
}

void Overdrive::reset()
{
    m_dcIn = 0.0f;
    m_dcOut = 0.0f;
}

float Overdrive::processSample(float input)
{
    float drive = m_drive.load(std::memory_order_relaxed);
    Mode mode = m_mode.load(std::memory_order_relaxed);
    float mix = m_mix.load(std::memory_order_relaxed);

    // Skip processing if drive is 1.0 (no effect)
    if (drive <= 1.01f)
        return input;

    float processed = 0.0f;

    switch (mode)
    {
        case Mode::Soft:      processed = processSoft(input, drive); break;
        case Mode::Classic:   processed = processClassic(input, drive); break;
        case Mode::Saturated: processed = processSaturated(input, drive); break;
        case Mode::Fuzz:      processed = processFuzz(input, drive); break;
        case Mode::Tape:      processed = processTape(input, drive); break;
        default:              processed = processClassic(input, drive); break;
    }

    // DC blocker to remove any DC offset from distortion
    float dcBlocked = processed - m_dcIn + DC_COEFF * m_dcOut;
    m_dcIn = processed;
    m_dcOut = dcBlocked;

    // Apply dry/wet mix
    return input * (1.0f - mix) + dcBlocked * mix;
}

float Overdrive::processSoft(float input, float drive)
{
    // Soft clipping using tanh - warm tube-like saturation
    float gained = input * drive;
    return std::tanh(gained) / std::tanh(drive);  // Normalize output
}

float Overdrive::processClassic(float input, float drive)
{
    // Classic 303 style - asymmetric soft clipping
    float gained = input * drive;

    // Asymmetric waveshaping
    if (gained > 0.0f)
        return std::tanh(gained * 1.2f) * 0.9f;
    else
        return std::tanh(gained * 0.8f) * 1.1f;
}

float Overdrive::processSaturated(float input, float drive)
{
    // Hard saturation with some harmonics
    float gained = input * drive;

    // Polynomial saturation: x - x^3/3
    float x = std::max(-1.5f, std::min(1.5f, gained));
    float out = x - (x * x * x) / 3.0f;

    return std::max(-1.0f, std::min(1.0f, out));
}

float Overdrive::processFuzz(float input, float drive)
{
    // Fuzz: heavy clipping with octave-up harmonics
    float gained = input * drive * 2.0f;

    // Full wave rectification adds octave
    float rectified = std::abs(gained);

    // Hard clip
    float clipped = std::max(-1.0f, std::min(1.0f, gained));

    // Mix original sign with rectified
    return (clipped * 0.7f + rectified * 0.3f * (input > 0 ? 1.0f : -1.0f));
}

float Overdrive::processTape(float input, float drive)
{
    // Tape saturation: gentle, warm compression
    float gained = input * drive * 0.7f;

    // Soft knee compression curve
    float absGained = std::abs(gained);
    float sign = gained >= 0.0f ? 1.0f : -1.0f;

    if (absGained < 0.5f)
        return gained;
    else if (absGained < 1.0f)
        return sign * (0.5f + (absGained - 0.5f) * 0.5f);
    else
        return sign * (0.75f + (absGained - 1.0f) * 0.1f);
}

void Overdrive::setDrive(float amount)
{
    m_drive.store(std::max(1.0f, std::min(10.0f, amount)), std::memory_order_relaxed);
}

void Overdrive::setMode(Mode mode)
{
    m_mode.store(mode, std::memory_order_relaxed);
}

void Overdrive::setMode(int index)
{
    if (index >= 0 && index <= 4)
        m_mode.store(static_cast<Mode>(index), std::memory_order_relaxed);
}

void Overdrive::setMix(float mix)
{
    m_mix.store(std::max(0.0f, std::min(1.0f, mix)), std::memory_order_relaxed);
}
