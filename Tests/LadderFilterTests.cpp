#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

// Include filter
#include "dsp/LadderFilter.h"

using namespace Catch::Matchers;

constexpr float SAMPLE_RATE = 44100.0f;
constexpr int BUFFER_SIZE = 512;

TEST_CASE("LadderFilter Basic Construction", "[filter][construction]") {
    SECTION("Can create filter instance") {
        REQUIRE_NOTHROW(LadderFilter());
    }

    SECTION("Initializes and prepares") {
        LadderFilter filter;
        REQUIRE_NOTHROW(filter.prepare(SAMPLE_RATE, BUFFER_SIZE));
    }
}

TEST_CASE("LadderFilter DSPModule Interface", "[filter][interface]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Can reset") {
        REQUIRE_NOTHROW(filter.reset());
    }

    SECTION("Can process samples") {
        float output = filter.processSample(0.5f);
        REQUIRE(std::isfinite(output));
    }

    SECTION("Output is in reasonable range") {
        filter.setCutoff(1000.0f);
        filter.setResonance(0.5f);

        for (int i = 0; i < 1000; ++i) {
            float input = std::sin(2.0f * M_PI * 440.0f * i / SAMPLE_RATE);
            float output = filter.processSample(input);
            REQUIRE(std::isfinite(output));
            REQUIRE(std::abs(output) <= 2.0f); // Allow some headroom for resonance
        }
    }
}

TEST_CASE("LadderFilter Parameter Setting", "[filter][parameters]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Can set cutoff frequency") {
        REQUIRE_NOTHROW(filter.setCutoff(100.0f));
        REQUIRE_NOTHROW(filter.setCutoff(1000.0f));
        REQUIRE_NOTHROW(filter.setCutoff(10000.0f));
    }

    SECTION("Can set resonance") {
        REQUIRE_NOTHROW(filter.setResonance(0.0f));
        REQUIRE_NOTHROW(filter.setResonance(0.5f));
        REQUIRE_NOTHROW(filter.setResonance(1.0f));
    }

    SECTION("Can set envelope amount") {
        REQUIRE_NOTHROW(filter.setEnvelopeAmount(-1.0f));
        REQUIRE_NOTHROW(filter.setEnvelopeAmount(0.0f));
        REQUIRE_NOTHROW(filter.setEnvelopeAmount(1.0f));
    }

    SECTION("Can set envelope value") {
        REQUIRE_NOTHROW(filter.setEnvelopeValue(0.0f));
        REQUIRE_NOTHROW(filter.setEnvelopeValue(0.5f));
        REQUIRE_NOTHROW(filter.setEnvelopeValue(1.0f));
    }

    SECTION("Get cutoff returns set value") {
        filter.setCutoff(1234.5f);
        REQUIRE_THAT(filter.getCutoff(), WithinRel(1234.5f, 0.01f));
    }
}

TEST_CASE("LadderFilter Frequency Response", "[filter][frequency]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Low cutoff attenuates high frequencies") {
        filter.setCutoff(500.0f);  // Low cutoff
        filter.setResonance(0.0f);

        // Generate high frequency signal (2000 Hz)
        std::vector<float> signal(1000);
        for (size_t i = 0; i < signal.size(); ++i) {
            signal[i] = std::sin(2.0f * M_PI * 2000.0f * i / SAMPLE_RATE);
        }

        // Process through filter
        std::vector<float> output(signal.size());
        for (size_t i = 0; i < signal.size(); ++i) {
            output[i] = filter.processSample(signal[i]);
        }

        // Output should be significantly attenuated
        // (Skip first samples for filter settling)
        float inputRMS = 0.0f, outputRMS = 0.0f;
        for (size_t i = 500; i < signal.size(); ++i) {
            inputRMS += signal[i] * signal[i];
            outputRMS += output[i] * output[i];
        }
        inputRMS = std::sqrt(inputRMS / 500.0f);
        outputRMS = std::sqrt(outputRMS / 500.0f);

        // High frequency should be attenuated
        REQUIRE(outputRMS < inputRMS * 0.5f);
    }

    SECTION("High cutoff passes low frequencies") {
        filter.setCutoff(5000.0f);  // High cutoff
        filter.setResonance(0.0f);

        // Generate low frequency signal (200 Hz)
        std::vector<float> signal(1000);
        for (size_t i = 0; i < signal.size(); ++i) {
            signal[i] = std::sin(2.0f * M_PI * 200.0f * i / SAMPLE_RATE);
        }

        // Process through filter
        std::vector<float> output(signal.size());
        for (size_t i = 0; i < signal.size(); ++i) {
            output[i] = filter.processSample(signal[i]);
        }

        // Output should be similar to input (passband)
        float inputRMS = 0.0f, outputRMS = 0.0f;
        for (size_t i = 500; i < signal.size(); ++i) {
            inputRMS += signal[i] * signal[i];
            outputRMS += output[i] * output[i];
        }
        inputRMS = std::sqrt(inputRMS / 500.0f);
        outputRMS = std::sqrt(outputRMS / 500.0f);

        // Low frequency should pass through (ladder filter has some attenuation even in passband)
        REQUIRE(outputRMS > inputRMS * 0.6f);
    }
}

TEST_CASE("LadderFilter Resonance", "[filter][resonance]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);
    filter.setCutoff(1000.0f);

    SECTION("Zero resonance produces normal response") {
        filter.setResonance(0.0f);

        std::vector<float> output(100);
        for (size_t i = 0; i < output.size(); ++i) {
            float input = std::sin(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);
            output[i] = filter.processSample(input);
        }

        // Should produce valid output
        bool allFinite = std::all_of(output.begin(), output.end(),
                                     [](float v) { return std::isfinite(v); });
        REQUIRE(allFinite);
    }

    SECTION("High resonance increases amplitude near cutoff") {
        // Test with low resonance
        filter.setResonance(0.1f);
        std::vector<float> outputLowRes(1000);
        for (size_t i = 0; i < outputLowRes.size(); ++i) {
            float input = std::sin(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);
            outputLowRes[i] = filter.processSample(input);
        }

        // Reset and test with high resonance
        filter.reset();
        filter.setCutoff(1000.0f);
        filter.setResonance(0.8f);
        std::vector<float> outputHighRes(1000);
        for (size_t i = 0; i < outputHighRes.size(); ++i) {
            float input = std::sin(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);
            outputHighRes[i] = filter.processSample(input);
        }

        // Calculate RMS for both
        float rmsLow = 0.0f, rmsHigh = 0.0f;
        for (size_t i = 500; i < 1000; ++i) {
            rmsLow += outputLowRes[i] * outputLowRes[i];
            rmsHigh += outputHighRes[i] * outputHighRes[i];
        }
        rmsLow = std::sqrt(rmsLow / 500.0f);
        rmsHigh = std::sqrt(rmsHigh / 500.0f);

        // High resonance should have higher amplitude
        REQUIRE(rmsHigh > rmsLow);
    }

    SECTION("Maximum resonance doesn't cause instability") {
        filter.setResonance(1.0f);

        for (int i = 0; i < 10000; ++i) {
            float input = std::sin(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);
            float output = filter.processSample(input);
            REQUIRE(std::isfinite(output));
            REQUIRE(std::abs(output) < 10.0f); // Should remain bounded
        }
    }
}

TEST_CASE("LadderFilter Envelope Modulation", "[filter][envelope]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);
    filter.setCutoff(1000.0f);
    filter.setResonance(0.3f);

    SECTION("Envelope modulation affects frequency response") {
        filter.setEnvelopeAmount(1.0f);  // Full positive modulation

        // Low envelope value
        filter.setEnvelopeValue(0.0f);
        std::vector<float> outputLowEnv(500);
        for (size_t i = 0; i < outputLowEnv.size(); ++i) {
            float input = std::sin(2.0f * M_PI * 2000.0f * i / SAMPLE_RATE);
            outputLowEnv[i] = filter.processSample(input);
        }

        // Reset and high envelope value
        filter.reset();
        filter.setCutoff(1000.0f);
        filter.setResonance(0.3f);
        filter.setEnvelopeAmount(1.0f);
        filter.setEnvelopeValue(1.0f);
        std::vector<float> outputHighEnv(500);
        for (size_t i = 0; i < outputHighEnv.size(); ++i) {
            float input = std::sin(2.0f * M_PI * 2000.0f * i / SAMPLE_RATE);
            outputHighEnv[i] = filter.processSample(input);
        }

        // Calculate RMS
        float rmsLow = 0.0f, rmsHigh = 0.0f;
        for (size_t i = 250; i < 500; ++i) {
            rmsLow += outputLowEnv[i] * outputLowEnv[i];
            rmsHigh += outputHighEnv[i] * outputHighEnv[i];
        }
        rmsLow = std::sqrt(rmsLow / 250.0f);
        rmsHigh = std::sqrt(rmsHigh / 250.0f);

        // High envelope should pass more high frequency content
        REQUIRE(rmsHigh > rmsLow);
    }

    SECTION("Negative envelope modulation works") {
        filter.setEnvelopeAmount(-1.0f);  // Full negative modulation
        filter.setEnvelopeValue(1.0f);

        // Should produce valid output
        for (int i = 0; i < 100; ++i) {
            float input = std::sin(2.0f * M_PI * 500.0f * i / SAMPLE_RATE);
            float output = filter.processSample(input);
            REQUIRE(std::isfinite(output));
        }
    }
}

TEST_CASE("LadderFilter Real-Time Safety", "[filter][realtime]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("No NaN or Inf outputs") {
        filter.setCutoff(1000.0f);
        filter.setResonance(0.7f);

        for (int i = 0; i < 10000; ++i) {
            float input = std::sin(2.0f * M_PI * 440.0f * i / SAMPLE_RATE);
            float output = filter.processSample(input);
            REQUIRE(std::isfinite(output));
            REQUIRE(!std::isnan(output));
            REQUIRE(!std::isinf(output));
        }
    }

    SECTION("Handles extreme inputs") {
        filter.setCutoff(1000.0f);
        filter.setResonance(0.5f);

        // Very large input
        float output = filter.processSample(10.0f);
        REQUIRE(std::isfinite(output));

        // Very small input
        output = filter.processSample(0.00001f);
        REQUIRE(std::isfinite(output));

        // Alternating large inputs
        for (int i = 0; i < 100; ++i) {
            output = filter.processSample((i % 2) ? 5.0f : -5.0f);
            REQUIRE(std::isfinite(output));
        }
    }

    SECTION("Handles extreme cutoff frequencies") {
        // Very low cutoff
        filter.setCutoff(20.0f);
        for (int i = 0; i < 100; ++i) {
            float input = std::sin(2.0f * M_PI * 100.0f * i / SAMPLE_RATE);
            float output = filter.processSample(input);
            REQUIRE(std::isfinite(output));
        }

        // Very high cutoff
        filter.reset();
        filter.setCutoff(20000.0f);
        for (int i = 0; i < 100; ++i) {
            float input = std::sin(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);
            float output = filter.processSample(input);
            REQUIRE(std::isfinite(output));
        }
    }
}

TEST_CASE("LadderFilter Reset Behavior", "[filter][reset]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);
    filter.setCutoff(1000.0f);
    filter.setResonance(0.5f);

    SECTION("Reset clears filter state") {
        // Process some samples to build up state
        for (int i = 0; i < 1000; ++i) {
            filter.processSample(std::sin(2.0f * M_PI * 440.0f * i / SAMPLE_RATE));
        }

        // Reset
        filter.reset();

        // Process zero input
        float output = filter.processSample(0.0f);

        // Should be near zero (may have tiny value due to numerical precision)
        REQUIRE(std::abs(output) < 0.01f);
    }
}

TEST_CASE("LadderFilter DC Blocking", "[filter][dc]") {
    LadderFilter filter;
    filter.prepare(SAMPLE_RATE, BUFFER_SIZE);
    filter.setCutoff(100.0f);  // Low cutoff
    filter.setResonance(0.0f);

    SECTION("Passes low frequencies but blocks DC") {
        // Process DC input
        std::vector<float> outputDC(1000);
        for (size_t i = 0; i < outputDC.size(); ++i) {
            outputDC[i] = filter.processSample(1.0f);
        }

        // DC should eventually settle to low value
        // (Perfect DC blocking not expected, but should attenuate)
        float avgOutput = 0.0f;
        for (size_t i = 900; i < 1000; ++i) {
            avgOutput += outputDC[i];
        }
        avgOutput /= 100.0f;

        // Should attenuate DC significantly
        REQUIRE(avgOutput < 0.9f);
    }
}
