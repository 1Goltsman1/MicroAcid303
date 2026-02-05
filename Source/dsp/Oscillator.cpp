#include "Oscillator.h"
#include <algorithm>

Oscillator::Oscillator() : m_rng(std::random_device{}())
{
    // Initialize supersaw phases with slight offsets
    for (int i = 0; i < 7; ++i)
        m_superSawPhases[i] = static_cast<float>(i) / 7.0f;
}

void Oscillator::prepare(double sampleRate, int samplesPerBlock)
{
    m_sampleRate = static_cast<float>(sampleRate);
    m_frequencySmoothing = m_targetFrequency.load();

    // Calculate slide coefficient based on slide time
    float slideTime = m_slideTime.load();
    m_slideCoeff = std::exp(-1.0f / (slideTime * m_sampleRate));

    reset();
}

void Oscillator::reset()
{
    m_phase = 0.0f;
    m_syncPhase = 0.0f;
    m_modPhase = 0.0f;
    m_phaseIncrement = m_targetFrequency.load() / m_sampleRate;

    for (int i = 0; i < 7; ++i)
        m_superSawPhases[i] = static_cast<float>(i) / 7.0f;
}

float Oscillator::processSample(float input)
{
    // Get current parameters
    float targetFreq = m_targetFrequency.load(std::memory_order_relaxed);
    Waveform waveform = m_waveform.load(std::memory_order_relaxed);
    float fineTune = m_fineTuneCents.load(std::memory_order_relaxed);
    float slideTime = m_slideTime.load(std::memory_order_relaxed);

    // Apply fine tuning
    if (fineTune != 0.0f)
        targetFreq *= std::pow(2.0f, fineTune / 1200.0f);

    // Calculate slide coefficient
    m_slideCoeff = std::exp(-1.0f / (slideTime * m_sampleRate + 0.001f));

    // Smooth frequency changes (portamento/slide)
    m_frequencySmoothing = m_frequencySmoothing * m_slideCoeff + targetFreq * (1.0f - m_slideCoeff);

    // Update phase increment
    m_phaseIncrement = m_frequencySmoothing / m_sampleRate;

    // Generate waveform based on selection
    float output = 0.0f;

    switch (waveform)
    {
        case Waveform::Sawtooth:  output = generateSawtooth(); break;
        case Waveform::Square:    output = generateSquare(); break;
        case Waveform::Triangle:  output = generateTriangle(); break;
        case Waveform::Sine:      output = generateSine(); break;
        case Waveform::Pulse25:   output = generatePulse(0.25f); break;
        case Waveform::Pulse12:   output = generatePulse(0.125f); break;
        case Waveform::SuperSaw:  output = generateSuperSaw(); break;
        case Waveform::Noise:     output = generateNoise(); break;
        case Waveform::SawSquare: output = generateSawSquare(); break;
        case Waveform::TriSaw:    output = generateTriSaw(); break;
        case Waveform::SyncSaw:   output = generateSyncSaw(); break;
        case Waveform::FM:        output = generateFM(); break;
        default:                  output = generateSawtooth(); break;
    }

    // Update main phase
    m_phase += m_phaseIncrement;
    if (m_phase >= 1.0f)
        m_phase -= 1.0f;

    // Soft clip output
    return std::max(-1.0f, std::min(1.0f, output));
}

// === WAVEFORM GENERATORS ===

float Oscillator::generateSawtooth()
{
    float output = 2.0f * m_phase - 1.0f;
    output -= polyBLEP(m_phase, m_phaseIncrement);
    return output;
}

float Oscillator::generateSquare()
{
    float output = m_phase < 0.5f ? 1.0f : -1.0f;
    output += polyBLEP(m_phase, m_phaseIncrement);

    float phase2 = m_phase + 0.5f;
    if (phase2 >= 1.0f) phase2 -= 1.0f;
    output -= polyBLEP(phase2, m_phaseIncrement);

    return output;
}

float Oscillator::generateTriangle()
{
    // Triangle from integrated square
    float output;
    if (m_phase < 0.5f)
        output = 4.0f * m_phase - 1.0f;
    else
        output = 3.0f - 4.0f * m_phase;
    return output;
}

float Oscillator::generateSine()
{
    return std::sin(m_phase * TWO_PI);
}

float Oscillator::generatePulse(float width)
{
    float output = m_phase < width ? 1.0f : -1.0f;
    output += polyBLEP(m_phase, m_phaseIncrement);

    float phase2 = m_phase + (1.0f - width);
    if (phase2 >= 1.0f) phase2 -= 1.0f;
    output -= polyBLEP(phase2, m_phaseIncrement);

    return output;
}

float Oscillator::generateSuperSaw()
{
    float output = 0.0f;

    // 7 detuned sawtooth oscillators
    for (int i = 0; i < 7; ++i)
    {
        float detunedInc = m_phaseIncrement * (1.0f + m_superSawDetune[i]);
        m_superSawPhases[i] += detunedInc;
        if (m_superSawPhases[i] >= 1.0f)
            m_superSawPhases[i] -= 1.0f;

        float saw = 2.0f * m_superSawPhases[i] - 1.0f;
        saw -= polyBLEP(m_superSawPhases[i], detunedInc);

        // Center oscillator louder
        float gain = (i == 3) ? 0.3f : 0.15f;
        output += saw * gain;
    }

    return output;
}

float Oscillator::generateNoise()
{
    return m_noiseDist(m_rng);
}

float Oscillator::generateSawSquare()
{
    float saw = generateSawtooth();

    // Generate square without updating phase again
    float sq = m_phase < 0.5f ? 1.0f : -1.0f;
    sq += polyBLEP(m_phase, m_phaseIncrement);
    float phase2 = m_phase + 0.5f;
    if (phase2 >= 1.0f) phase2 -= 1.0f;
    sq -= polyBLEP(phase2, m_phaseIncrement);

    return (saw + sq) * 0.5f;
}

float Oscillator::generateTriSaw()
{
    float saw = 2.0f * m_phase - 1.0f;
    saw -= polyBLEP(m_phase, m_phaseIncrement);

    float tri;
    if (m_phase < 0.5f)
        tri = 4.0f * m_phase - 1.0f;
    else
        tri = 3.0f - 4.0f * m_phase;

    return (saw + tri) * 0.5f;
}

float Oscillator::generateSyncSaw()
{
    // Hard sync: slave oscillator resets when master completes cycle
    m_syncPhase += m_phaseIncrement * m_syncRatio;

    // Reset sync oscillator when master resets
    if (m_phase + m_phaseIncrement >= 1.0f)
        m_syncPhase = 0.0f;

    if (m_syncPhase >= 1.0f)
        m_syncPhase -= 1.0f;

    float output = 2.0f * m_syncPhase - 1.0f;
    output -= polyBLEP(m_syncPhase, m_phaseIncrement * m_syncRatio);

    return output;
}

float Oscillator::generateFM()
{
    // FM synthesis: carrier modulated by modulator
    m_modPhase += m_phaseIncrement * m_fmRatio;
    if (m_modPhase >= 1.0f)
        m_modPhase -= 1.0f;

    float modulator = std::sin(m_modPhase * TWO_PI);
    float carrier = std::sin((m_phase + modulator * m_fmIndex * 0.1f) * TWO_PI);

    return carrier;
}

// === PARAMETER SETTERS ===

void Oscillator::setFrequency(float frequencyHz)
{
    float nyquist = m_sampleRate * 0.49f;
    m_targetFrequency.store(std::max(20.0f, std::min(frequencyHz, nyquist)),
                            std::memory_order_relaxed);
}

void Oscillator::setWaveform(Waveform waveform)
{
    m_waveform.store(waveform, std::memory_order_relaxed);
}

void Oscillator::setWaveform(int index)
{
    if (index >= 0 && index <= 11)
        m_waveform.store(static_cast<Waveform>(index), std::memory_order_relaxed);
}

void Oscillator::setFineTune(float cents)
{
    m_fineTuneCents.store(std::max(-50.0f, std::min(50.0f, cents)),
                          std::memory_order_relaxed);
}

void Oscillator::setSlideTime(float seconds)
{
    m_slideTime.store(std::max(0.001f, std::min(0.5f, seconds)),
                      std::memory_order_relaxed);
}

float Oscillator::polyBLEP(float t, float dt) const
{
    if (dt <= 0.0f) return 0.0f;

    if (t < dt)
    {
        const float t_norm = t / dt;
        return t_norm * t_norm - 2.0f * t_norm + 1.0f;
    }
    else if (t > 1.0f - dt)
    {
        const float t_norm = (t - 1.0f) / dt + 1.0f;
        return t_norm * t_norm + 2.0f * t_norm - 1.0f;
    }

    return 0.0f;
}
