#include "Effects.h"
#include <algorithm>

Effects::Effects() : m_rng(std::random_device{}())
{
}

void Effects::prepare(double sampleRate, int samplesPerBlock)
{
    m_sampleRate = static_cast<float>(sampleRate);

    // Allocate delay buffer (max 2 seconds)
    m_maxDelaySamples = static_cast<int>(m_sampleRate * 2.0f);
    m_delayBuffer.resize(m_maxDelaySamples, 0.0f);
    m_delayBufferR.resize(m_maxDelaySamples, 0.0f);

    // Allocate reverb comb filters
    for (int i = 0; i < NUM_COMBS; ++i)
    {
        int scaledLength = static_cast<int>(m_combLengths[i] * m_sampleRate / 44100.0f);
        m_combBuffers[i].resize(scaledLength, 0.0f);
    }

    // Allocate reverb allpass filters
    for (int i = 0; i < NUM_ALLPASS; ++i)
    {
        int scaledLength = static_cast<int>(m_allpassLengths[i] * m_sampleRate / 44100.0f);
        m_allpassBuffers[i].resize(scaledLength, 0.0f);
    }

    reset();
}

void Effects::reset()
{
    std::fill(m_delayBuffer.begin(), m_delayBuffer.end(), 0.0f);
    std::fill(m_delayBufferR.begin(), m_delayBufferR.end(), 0.0f);
    m_delayWritePos = 0;
    m_delayWritePosR = 0;
    m_lfoPhase = 0.0f;
    m_wowPhase = 0.0f;

    for (int i = 0; i < NUM_COMBS; ++i)
    {
        std::fill(m_combBuffers[i].begin(), m_combBuffers[i].end(), 0.0f);
        m_combWritePos[i] = 0;
    }

    for (int i = 0; i < NUM_ALLPASS; ++i)
    {
        std::fill(m_allpassBuffers[i].begin(), m_allpassBuffers[i].end(), 0.0f);
        m_allpassWritePos[i] = 0;
    }

    for (int i = 0; i < NUM_PHASER_STAGES; ++i)
        m_phaserStages[i] = 0.0f;
}

float Effects::processSample(float input)
{
    Type type = m_type.load(std::memory_order_relaxed);
    float mix = m_mix.load(std::memory_order_relaxed);

    float wet = 0.0f;

    switch (type)
    {
        case Type::TapeDelay:    wet = processTapeDelay(input); break;
        case Type::DigitalDelay: wet = processDigitalDelay(input); break;
        case Type::PingPong:     wet = processPingPong(input); break;
        case Type::Reverb:       wet = processReverb(input); break;
        case Type::Chorus:       wet = processChorus(input); break;
        case Type::Flanger:      wet = processFlanger(input); break;
        case Type::Phaser:       wet = processPhaser(input); break;
        case Type::Bitcrush:     wet = processBitcrush(input); break;
        default:                 wet = processDigitalDelay(input); break;
    }

    return input * (1.0f - mix) + wet * mix;
}

float Effects::processTapeDelay(float input)
{
    float time = m_time.load(std::memory_order_relaxed);
    float feedback = m_feedback.load(std::memory_order_relaxed);

    // Add wow and flutter
    m_wowPhase += 0.3f / m_sampleRate;
    if (m_wowPhase >= 1.0f) m_wowPhase -= 1.0f;

    float wow = std::sin(m_wowPhase * TWO_PI) * 0.002f;
    float flutter = m_wowDist(m_rng);
    float timeModulation = 1.0f + wow + flutter;

    float delaySamples = (time / 1000.0f) * m_sampleRate * timeModulation;
    delaySamples = std::max(1.0f, std::min(delaySamples, static_cast<float>(m_maxDelaySamples - 1)));

    float delayed = readDelay(delaySamples);

    // Soft saturation on feedback (tape character)
    float feedbackSignal = std::tanh(delayed * 1.5f) * 0.9f;

    writeDelay(input + feedbackSignal * feedback);

    return delayed;
}

float Effects::processDigitalDelay(float input)
{
    float time = m_time.load(std::memory_order_relaxed);
    float feedback = m_feedback.load(std::memory_order_relaxed);

    float delaySamples = (time / 1000.0f) * m_sampleRate;
    delaySamples = std::max(1.0f, std::min(delaySamples, static_cast<float>(m_maxDelaySamples - 1)));

    float delayed = readDelay(delaySamples);
    writeDelay(input + delayed * feedback);

    return delayed;
}

float Effects::processPingPong(float input)
{
    float time = m_time.load(std::memory_order_relaxed);
    float feedback = m_feedback.load(std::memory_order_relaxed);

    float delaySamples = (time / 1000.0f) * m_sampleRate;
    delaySamples = std::max(1.0f, std::min(delaySamples, static_cast<float>(m_maxDelaySamples - 1)));

    // Read from both channels
    int readPos = m_delayWritePos - static_cast<int>(delaySamples);
    if (readPos < 0) readPos += m_maxDelaySamples;

    int readPosR = m_delayWritePosR - static_cast<int>(delaySamples);
    if (readPosR < 0) readPosR += m_maxDelaySamples;

    float delayedL = m_delayBuffer[readPos];
    float delayedR = m_delayBufferR[readPosR];

    // Cross-feed (ping pong)
    m_delayBuffer[m_delayWritePos] = input + delayedR * feedback;
    m_delayBufferR[m_delayWritePosR] = delayedL * feedback;

    m_delayWritePos = (m_delayWritePos + 1) % m_maxDelaySamples;
    m_delayWritePosR = (m_delayWritePosR + 1) % m_maxDelaySamples;

    return (delayedL + delayedR) * 0.5f;
}

float Effects::processReverb(float input)
{
    float feedback = m_feedback.load(std::memory_order_relaxed);
    m_combFeedback = 0.7f + feedback * 0.25f;

    // Parallel comb filters
    float combSum = 0.0f;
    for (int i = 0; i < NUM_COMBS; ++i)
    {
        int bufferSize = static_cast<int>(m_combBuffers[i].size());
        if (bufferSize == 0) continue;

        float delayed = m_combBuffers[i][m_combWritePos[i]];
        float newSample = input + delayed * m_combFeedback;
        m_combBuffers[i][m_combWritePos[i]] = newSample;
        m_combWritePos[i] = (m_combWritePos[i] + 1) % bufferSize;
        combSum += delayed;
    }
    combSum *= 0.25f;

    // Series allpass filters
    float allpassOut = combSum;
    for (int i = 0; i < NUM_ALLPASS; ++i)
    {
        int bufferSize = static_cast<int>(m_allpassBuffers[i].size());
        if (bufferSize == 0) continue;

        float delayed = m_allpassBuffers[i][m_allpassWritePos[i]];
        float temp = allpassOut + delayed * 0.5f;
        m_allpassBuffers[i][m_allpassWritePos[i]] = temp;
        m_allpassWritePos[i] = (m_allpassWritePos[i] + 1) % bufferSize;
        allpassOut = delayed - allpassOut * 0.5f;
    }

    return allpassOut;
}

float Effects::processChorus(float input)
{
    float depth = m_modDepth.load(std::memory_order_relaxed);
    float rate = m_modRate.load(std::memory_order_relaxed);

    // LFO
    m_lfoPhase += rate / m_sampleRate;
    if (m_lfoPhase >= 1.0f) m_lfoPhase -= 1.0f;

    float lfo = std::sin(m_lfoPhase * TWO_PI);

    // Modulated delay time (10-30ms range)
    float baseDelay = 20.0f;
    float modDelay = depth * 10.0f;
    float delaySamples = ((baseDelay + lfo * modDelay) / 1000.0f) * m_sampleRate;
    delaySamples = std::max(1.0f, std::min(delaySamples, static_cast<float>(m_maxDelaySamples - 1)));

    float delayed = readDelay(delaySamples);
    writeDelay(input);

    return (input + delayed) * 0.7f;
}

float Effects::processFlanger(float input)
{
    float depth = m_modDepth.load(std::memory_order_relaxed);
    float rate = m_modRate.load(std::memory_order_relaxed);
    float feedback = m_feedback.load(std::memory_order_relaxed);

    // LFO
    m_lfoPhase += rate / m_sampleRate;
    if (m_lfoPhase >= 1.0f) m_lfoPhase -= 1.0f;

    float lfo = std::sin(m_lfoPhase * TWO_PI);

    // Very short modulated delay (0.1-10ms)
    float baseDelay = 2.0f;
    float modDelay = depth * 5.0f;
    float delaySamples = ((baseDelay + lfo * modDelay) / 1000.0f) * m_sampleRate;
    delaySamples = std::max(1.0f, std::min(delaySamples, static_cast<float>(m_maxDelaySamples - 1)));

    float delayed = readDelay(delaySamples);
    writeDelay(input + delayed * feedback * 0.7f);

    return (input + delayed) * 0.7f;
}

float Effects::processPhaser(float input)
{
    float depth = m_modDepth.load(std::memory_order_relaxed);
    float rate = m_modRate.load(std::memory_order_relaxed);
    float feedback = m_feedback.load(std::memory_order_relaxed);

    // LFO
    m_lfoPhase += rate / m_sampleRate;
    if (m_lfoPhase >= 1.0f) m_lfoPhase -= 1.0f;

    float lfo = (std::sin(m_lfoPhase * TWO_PI) + 1.0f) * 0.5f; // 0 to 1

    // Calculate allpass coefficient from LFO
    float minFreq = 200.0f;
    float maxFreq = 1600.0f;
    float freq = minFreq + lfo * (maxFreq - minFreq) * depth;
    float coeff = (1.0f - std::tan(3.14159f * freq / m_sampleRate)) /
                  (1.0f + std::tan(3.14159f * freq / m_sampleRate));

    // Cascade of allpass filters
    float signal = input + m_phaserStages[NUM_PHASER_STAGES - 1] * feedback * 0.5f;

    for (int i = 0; i < NUM_PHASER_STAGES; ++i)
    {
        float allpassOut = coeff * (signal - m_phaserStages[i]) + signal;
        m_phaserStages[i] = signal;
        signal = allpassOut;
    }

    return (input + signal) * 0.5f;
}

float Effects::processBitcrush(float input)
{
    float depth = m_modDepth.load(std::memory_order_relaxed);

    // Bit depth reduction (16 to 2 bits based on depth)
    int bits = static_cast<int>(16 - depth * 14);
    bits = std::max(2, std::min(16, bits));

    float levels = std::pow(2.0f, static_cast<float>(bits));
    float crushed = std::round(input * levels) / levels;

    // Sample rate reduction
    float rate = m_modRate.load(std::memory_order_relaxed);
    static float held = 0.0f;
    static int counter = 0;

    int holdSamples = static_cast<int>(1.0f + rate * 20.0f);
    if (++counter >= holdSamples)
    {
        held = crushed;
        counter = 0;
    }

    return held;
}

// === UTILITY FUNCTIONS ===

float Effects::readDelay(float delaySamples)
{
    // Linear interpolation for smooth delay
    int indexA = m_delayWritePos - static_cast<int>(delaySamples);
    int indexB = indexA - 1;

    if (indexA < 0) indexA += m_maxDelaySamples;
    if (indexB < 0) indexB += m_maxDelaySamples;

    float frac = delaySamples - std::floor(delaySamples);
    return m_delayBuffer[indexA] * (1.0f - frac) + m_delayBuffer[indexB] * frac;
}

void Effects::writeDelay(float sample)
{
    m_delayBuffer[m_delayWritePos] = sample;
    m_delayWritePos = (m_delayWritePos + 1) % m_maxDelaySamples;
}

// === PARAMETER SETTERS ===

void Effects::setType(Type type)
{
    m_type.store(type, std::memory_order_relaxed);
}

void Effects::setType(int index)
{
    if (index >= 0 && index <= 7)
        m_type.store(static_cast<Type>(index), std::memory_order_relaxed);
}

void Effects::setTime(float ms)
{
    m_time.store(std::max(10.0f, std::min(2000.0f, ms)), std::memory_order_relaxed);
}

void Effects::setFeedback(float amount)
{
    m_feedback.store(std::max(0.0f, std::min(0.95f, amount)), std::memory_order_relaxed);
}

void Effects::setMix(float mix)
{
    m_mix.store(std::max(0.0f, std::min(1.0f, mix)), std::memory_order_relaxed);
}

void Effects::setModDepth(float depth)
{
    m_modDepth.store(std::max(0.0f, std::min(1.0f, depth)), std::memory_order_relaxed);
}

void Effects::setModRate(float hz)
{
    m_modRate.store(std::max(0.1f, std::min(10.0f, hz)), std::memory_order_relaxed);
}
