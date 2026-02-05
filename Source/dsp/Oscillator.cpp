#include "Oscillator.h"
#include <algorithm>

Oscillator::Oscillator()
{
}

void Oscillator::prepare(double sampleRate, int samplesPerBlock)
{
    m_sampleRate = static_cast<float>(sampleRate);
    m_frequencySmoothing = m_targetFrequency.load();
    reset();
}

void Oscillator::reset()
{
    m_phase = 0.0f;
    m_phaseIncrement = m_targetFrequency.load() / m_sampleRate;
}

float Oscillator::processSample(float input)
{
    // Get current parameters
    float targetFreq = m_targetFrequency.load(std::memory_order_relaxed);
    Waveform waveform = m_waveform.load(std::memory_order_relaxed);
    float fineTune = m_fineTuneCents.load(std::memory_order_relaxed);

    // Apply fine tuning if needed
    if (fineTune != 0.0f)
    {
        targetFreq *= std::pow(2.0f, fineTune / 1200.0f);
    }

    // Smooth frequency changes
    m_frequencySmoothing = m_frequencySmoothing * SMOOTHING_COEFF +
                           targetFreq * (1.0f - SMOOTHING_COEFF);

    // Update phase increment
    m_phaseIncrement = m_frequencySmoothing / m_sampleRate;

    // Generate waveform
    float output = 0.0f;

    if (waveform == Waveform::Sawtooth)
    {
        // Naive sawtooth: 2 * phase - 1
        output = 2.0f * m_phase - 1.0f;

        // Apply PolyBLEP correction
        output -= polyBLEP(m_phase, m_phaseIncrement);
    }
    else // Square
    {
        // Naive square wave
        output = m_phase < 0.5f ? 1.0f : -1.0f;

        // Apply PolyBLEP at both edges
        output += polyBLEP(m_phase, m_phaseIncrement);

        // Second edge at phase = 0.5
        float phase2 = m_phase + 0.5f;
        if (phase2 >= 1.0f)
            phase2 -= 1.0f;
        output -= polyBLEP(phase2, m_phaseIncrement);
    }

    // Update phase
    m_phase += m_phaseIncrement;
    if (m_phase >= 1.0f)
    {
        m_phase -= 1.0f;
    }

    // Soft clip to prevent overload
    output = std::max(-1.0f, std::min(1.0f, output));

    return output;
}

void Oscillator::setFrequency(float frequencyHz)
{
    float nyquist = m_sampleRate * 0.49f;
    m_targetFrequency.store(std::max(0.0f, std::min(frequencyHz, nyquist)),
                            std::memory_order_relaxed);
}

void Oscillator::setWaveform(Waveform waveform)
{
    m_waveform.store(waveform, std::memory_order_relaxed);
}

void Oscillator::setFineTune(float cents)
{
    m_fineTuneCents.store(std::max(-50.0f, std::min(50.0f, cents)),
                          std::memory_order_relaxed);
}

float Oscillator::polyBLEP(float t, float dt) const
{
    // PolyBLEP: Polynomial Band-Limited Step
    // Corrects discontinuities in naive waveforms

    if (dt <= 0.0f)
        return 0.0f;

    // Handle discontinuity at phase = 0
    if (t < dt)
    {
        const float t_norm = t / dt;
        return t_norm * t_norm - 2.0f * t_norm + 1.0f;
    }
    // Handle discontinuity at phase = 1
    else if (t > 1.0f - dt)
    {
        const float t_norm = (t - 1.0f) / dt + 1.0f;
        return t_norm * t_norm + 2.0f * t_norm - 1.0f;
    }

    return 0.0f;
}
