#pragma once

#include "../core/DSPModule.h"
#include <atomic>
#include <cmath>
#include <vector>
#include <random>

/**
 * Multi-effects processor with Delay, Reverb, Chorus, Flanger, Phaser
 */
class Effects : public DSPModule {
public:
    enum class Type {
        TapeDelay = 0,
        DigitalDelay,
        PingPong,
        Reverb,
        Chorus,
        Flanger,
        Phaser,
        Bitcrush
    };

    Effects();
    ~Effects() override = default;

    void prepare(double sampleRate, int samplesPerBlock) override;
    void reset() override;
    float processSample(float input) override;

    void setType(Type type);
    void setType(int index);
    void setTime(float ms);           // 10-2000ms
    void setFeedback(float amount);   // 0-0.95
    void setMix(float mix);           // 0-1.0
    void setModDepth(float depth);    // For chorus/flanger
    void setModRate(float hz);        // For chorus/flanger

private:
    // Effect processors
    float processTapeDelay(float input);
    float processDigitalDelay(float input);
    float processPingPong(float input);
    float processReverb(float input);
    float processChorus(float input);
    float processFlanger(float input);
    float processPhaser(float input);
    float processBitcrush(float input);

    // Utility
    float readDelay(float delaySamples);
    void writeDelay(float sample);
    float allpassFilter(float input, float* buffer, int& index, int length, float feedback);

    float m_sampleRate = 44100.0f;

    // Delay buffer
    std::vector<float> m_delayBuffer;
    int m_delayWritePos = 0;
    int m_maxDelaySamples = 0;

    // Ping pong
    std::vector<float> m_delayBufferR;
    int m_delayWritePosR = 0;
    bool m_pingPongSide = false;

    // Reverb (simple Schroeder)
    static constexpr int NUM_COMBS = 4;
    static constexpr int NUM_ALLPASS = 2;
    std::vector<float> m_combBuffers[NUM_COMBS];
    int m_combWritePos[NUM_COMBS] = {0};
    int m_combLengths[NUM_COMBS] = {1557, 1617, 1491, 1422};
    float m_combFeedback = 0.84f;

    std::vector<float> m_allpassBuffers[NUM_ALLPASS];
    int m_allpassWritePos[NUM_ALLPASS] = {0};
    int m_allpassLengths[NUM_ALLPASS] = {225, 341};

    // Chorus/Flanger LFO
    float m_lfoPhase = 0.0f;
    float m_lfoRate = 0.5f;

    // Phaser allpass stages
    static constexpr int NUM_PHASER_STAGES = 6;
    float m_phaserStages[NUM_PHASER_STAGES] = {0};

    // Wow/flutter for tape delay
    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_wowDist{-0.002f, 0.002f};
    float m_wowPhase = 0.0f;

    // Parameters
    std::atomic<Type> m_type{Type::TapeDelay};
    std::atomic<float> m_time{250.0f};
    std::atomic<float> m_feedback{0.5f};
    std::atomic<float> m_mix{0.3f};
    std::atomic<float> m_modDepth{0.5f};
    std::atomic<float> m_modRate{0.5f};

    static constexpr float TWO_PI = 6.283185307179586f;
};
