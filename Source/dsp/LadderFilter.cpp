#include "LadderFilter.h"
#include <algorithm>
#include <cmath>

LadderFilter::LadderFilter()
{
}

void LadderFilter::prepare(double sampleRate, int samplesPerBlock)
{
    (void)samplesPerBlock; // Unused
    m_sampleRate = static_cast<float>(sampleRate);
    m_cutoffSmoothed = m_targetCutoff.load();
    updateCoefficients();
    reset();
}

void LadderFilter::reset()
{
    m_stage.fill(0.0f);
    m_stageTanh[0] = m_stageTanh[1] = m_stageTanh[2] = m_stageTanh[3] = 0.0f;
    m_feedback = 0.0f;
}

float LadderFilter::processSample(float input)
{
    // Get current parameters
    float targetCutoff = m_targetCutoff.load(std::memory_order_relaxed);
    float resonance = m_resonance.load(std::memory_order_relaxed);
    float envAmount = m_envelopeAmount.load(std::memory_order_relaxed);
    float envValue = m_envelopeValue.load(std::memory_order_relaxed);

    // Apply envelope modulation to cutoff
    if (envAmount != 0.0f)
    {
        // Envelope modulates in exponential fashion (like analog filters)
        float modulation = envAmount * envValue;
        // Convert to frequency multiplier (±4 octaves)
        float multiplier = std::pow(2.0f, modulation * 4.0f);
        targetCutoff *= multiplier;
        targetCutoff = std::max(MIN_CUTOFF, std::min(targetCutoff, MAX_CUTOFF));
    }

    // Smooth cutoff changes
    m_cutoffSmoothed = m_cutoffSmoothed * CUTOFF_SMOOTHING +
                       targetCutoff * (1.0f - CUTOFF_SMOOTHING);

    // Update filter coefficients
    updateCoefficients();

    // Apply input saturation
    input = saturate(input);

    // Feedback for resonance (from stage 4 to input)
    input -= m_k * m_feedback;

    // Process through 4 filter stages (ladder topology)
    for (int stage = 0; stage < 4; ++stage)
    {
        // One-pole lowpass filter per stage
        float stageInput = (stage == 0) ? input : m_stageTanh[stage - 1];
        m_stage[stage] += m_g * (stageInput - m_stageTanh[stage]);

        // Non-linear saturation (tanh approximation)
        m_stageTanh[stage] = saturate(m_stage[stage]);
    }

    // Update feedback for next sample
    m_feedback = m_stageTanh[3];

    // Output from final stage
    return m_feedback;
}

void LadderFilter::setCutoff(float frequencyHz)
{
    float clampedFreq = std::max(MIN_CUTOFF, std::min(frequencyHz, MAX_CUTOFF));
    m_targetCutoff.store(clampedFreq, std::memory_order_relaxed);
}

void LadderFilter::setResonance(float resonance)
{
    float clampedRes = std::max(0.0f, std::min(resonance, 1.0f));
    m_resonance.store(clampedRes, std::memory_order_relaxed);
}

void LadderFilter::setEnvelopeAmount(float amount)
{
    float clampedAmount = std::max(-1.0f, std::min(amount, 1.0f));
    m_envelopeAmount.store(clampedAmount, std::memory_order_relaxed);
}

void LadderFilter::setEnvelopeValue(float value)
{
    float clampedValue = std::max(0.0f, std::min(value, 1.0f));
    m_envelopeValue.store(clampedValue, std::memory_order_relaxed);
}

void LadderFilter::updateCoefficients()
{
    float resonance = m_resonance.load(std::memory_order_relaxed);

    // Calculate cutoff coefficient (g)
    // Using bilinear transform approximation
    float wd = 2.0f * M_PI * m_cutoffSmoothed;
    float T = 1.0f / m_sampleRate;
    float wa = (2.0f / T) * std::tan(wd * T / 2.0f);
    m_g = wa * T / 2.0f;

    // Clamp g for stability
    m_g = std::min(m_g, 0.99f);

    // Calculate resonance coefficient (k)
    // Scale resonance to achieve self-oscillation at high values
    m_k = 4.0f * resonance * (1.0f + 0.5f * resonance);
}

float LadderFilter::saturate(float input) const
{
    // Fast tanh approximation for soft clipping
    // Using tanh(x) ≈ x / (1 + |x|) for fast computation
    // Scaled for gentle saturation
    float x = input * SATURATION_AMOUNT;
    return x / (1.0f + std::abs(x));
}
