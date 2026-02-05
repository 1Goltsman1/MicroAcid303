#include "Envelope.h"
#include <algorithm>
#include <cmath>

Envelope::Envelope()
{
}

void Envelope::prepare(double sampleRate, int samplesPerBlock)
{
    (void)samplesPerBlock; // Unused for envelope
    m_sampleRate = static_cast<float>(sampleRate);

    // Recalculate coefficients based on sample rate
    m_attackCoeff = calculateCoefficient(m_attackTime.load());
    m_decayCoeff = calculateCoefficient(m_decayTime.load());
    m_releaseCoeff = calculateCoefficient(m_releaseTime.load());

    reset();
}

void Envelope::reset()
{
    m_level = 0.0f;
    m_stage.store(Stage::Idle, std::memory_order_relaxed);
}

float Envelope::processSample(float input)
{
    (void)input; // Envelope doesn't process input signal
    Stage currentStage = m_stage.load(std::memory_order_relaxed);

    switch (currentStage)
    {
        case Stage::Idle:
            m_level = 0.0f;
            break;

        case Stage::Attack:
        {
            // Exponential approach to 1.0
            m_level += m_attackCoeff * (1.0f - m_level);

            // Check if attack is complete
            if (m_level >= 1.0f - EPSILON)
            {
                m_level = 1.0f;
                m_stage.store(Stage::Decay, std::memory_order_relaxed);
            }
            break;
        }

        case Stage::Decay:
        {
            float sustainLevel = m_sustainLevel.load(std::memory_order_relaxed);

            // Exponential approach to sustain level
            m_level += m_decayCoeff * (sustainLevel - m_level);

            // Check if decay is complete
            if (std::abs(m_level - sustainLevel) < EPSILON)
            {
                m_level = sustainLevel;
                m_stage.store(Stage::Sustain, std::memory_order_relaxed);
            }
            break;
        }

        case Stage::Sustain:
        {
            // Hold at sustain level
            m_level = m_sustainLevel.load(std::memory_order_relaxed);
            break;
        }

        case Stage::Release:
        {
            // Exponential approach to 0
            m_level += m_releaseCoeff * (0.0f - m_level);

            // Check if release is complete
            if (m_level < EPSILON)
            {
                m_level = 0.0f;
                m_stage.store(Stage::Idle, std::memory_order_relaxed);
            }
            break;
        }
    }

    return m_level;
}

void Envelope::noteOn()
{
    // Recalculate coefficients in case parameters changed
    m_attackCoeff = calculateCoefficient(m_attackTime.load());
    m_decayCoeff = calculateCoefficient(m_decayTime.load());
    m_releaseCoeff = calculateCoefficient(m_releaseTime.load());

    // Start attack stage (even if already running - retrigger)
    m_stage.store(Stage::Attack, std::memory_order_relaxed);
}

void Envelope::noteOff()
{
    // Move to release stage
    m_stage.store(Stage::Release, std::memory_order_relaxed);
}

void Envelope::setAttack(float timeSeconds)
{
    float clampedTime = std::max(MIN_TIME, std::min(timeSeconds, MAX_TIME));
    m_attackTime.store(clampedTime, std::memory_order_relaxed);
}

void Envelope::setDecay(float timeSeconds)
{
    float clampedTime = std::max(MIN_TIME, std::min(timeSeconds, MAX_TIME));
    m_decayTime.store(clampedTime, std::memory_order_relaxed);
}

void Envelope::setSustain(float level)
{
    float clampedLevel = std::max(0.0f, std::min(1.0f, level));
    m_sustainLevel.store(clampedLevel, std::memory_order_relaxed);
}

void Envelope::setRelease(float timeSeconds)
{
    float clampedTime = std::max(MIN_TIME, std::min(timeSeconds, MAX_TIME));
    m_releaseTime.store(clampedTime, std::memory_order_relaxed);
}

float Envelope::calculateCoefficient(float timeSeconds) const
{
    if (timeSeconds <= 0.0f || m_sampleRate <= 0.0f)
        return 1.0f;

    // Calculate coefficient for exponential curve
    // Using: coeff = 1 - exp(-5 / (time * sampleRate))
    // This gives approximately 99% completion in the specified time
    // (5 time constants = 99.3% completion)
    float samples = timeSeconds * m_sampleRate;
    return 1.0f - std::exp(-5.0f / samples);
}
