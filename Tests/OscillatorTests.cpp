#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>

// Include oscillator
#include "dsp/Oscillator.h"

using namespace Catch::Matchers;

constexpr float SAMPLE_RATE = 44100.0f;
constexpr int BUFFER_SIZE = 512;

TEST_CASE("Oscillator Basic Construction", "[oscillator][construction]") {
    SECTION("Can create oscillator instance") {
        REQUIRE_NOTHROW(Oscillator());
    }

    SECTION("Initializes and prepares") {
        Oscillator osc;
        REQUIRE_NOTHROW(osc.prepare(SAMPLE_RATE, BUFFER_SIZE));
    }
}

TEST_CASE("Oscillator DSPModule Interface", "[oscillator][interface]") {
    Oscillator osc;
    osc.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Can reset") {
        REQUIRE_NOTHROW(osc.reset());
    }

    SECTION("Can process samples") {
        float output = osc.processSample(0.0f);
        REQUIRE(std::isfinite(output));
    }

    SECTION("Output is in reasonable range") {
        osc.setFrequency(440.0f);
        osc.setWaveform(Oscillator::Waveform::Sawtooth);

        for (int i = 0; i < 1000; ++i) {
            float output = osc.processSample(0.0f);
            REQUIRE(std::isfinite(output));
            REQUIRE(std::abs(output) <= 1.5f); // Allow some headroom
        }
    }
}

TEST_CASE("Oscillator Frequency Setting", "[oscillator][frequency]") {
    Oscillator osc;
    osc.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Can set frequency") {
        REQUIRE_NOTHROW(osc.setFrequency(440.0f));
        REQUIRE_NOTHROW(osc.setFrequency(220.0f));
        REQUIRE_NOTHROW(osc.setFrequency(880.0f));
    }

    SECTION("Produces output at set frequency") {
        osc.setFrequency(100.0f); // Low frequency
        osc.setWaveform(Oscillator::Waveform::Sawtooth);

        std::vector<float> signal(SAMPLE_RATE);
        for (size_t i = 0; i < signal.size(); ++i) {
            signal[i] = osc.processSample(0.0f);
        }

        // Should have roughly 100 cycles per second
        // Count zero crossings as a rough frequency check
        int crossings = 0;
        for (size_t i = 1; i < signal.size(); ++i) {
            if ((signal[i-1] < 0 && signal[i] >= 0) ||
                (signal[i-1] >= 0 && signal[i] < 0)) {
                crossings++;
            }
        }

        // Should have roughly 200 crossings for 100Hz (2 per cycle)
        REQUIRE(crossings > 150);
        REQUIRE(crossings < 250);
    }
}

TEST_CASE("Oscillator Waveforms", "[oscillator][waveform]") {
    Oscillator osc;
    osc.prepare(SAMPLE_RATE, BUFFER_SIZE);
    osc.setFrequency(100.0f);

    SECTION("Sawtooth waveform") {
        osc.setWaveform(Oscillator::Waveform::Sawtooth);

        std::vector<float> signal(441); // One period at 100Hz
        for (size_t i = 0; i < signal.size(); ++i) {
            signal[i] = osc.processSample(0.0f);
        }

        // Check range
        float minVal = *std::min_element(signal.begin(), signal.end());
        float maxVal = *std::max_element(signal.begin(), signal.end());

        REQUIRE(minVal < -0.5f);
        REQUIRE(maxVal > 0.5f);
    }

    SECTION("Square waveform") {
        osc.setWaveform(Oscillator::Waveform::Square);

        std::vector<float> signal(441);
        for (size_t i = 0; i < signal.size(); ++i) {
            signal[i] = osc.processSample(0.0f);
        }

        // Square wave should have values near -1 and 1
        int highCount = 0;
        int lowCount = 0;
        for (float sample : signal) {
            if (sample > 0.3f) highCount++;
            else if (sample < -0.3f) lowCount++;
        }

        // Should spend significant time at both high and low
        REQUIRE(highCount > 100);
        REQUIRE(lowCount > 100);
    }
}

TEST_CASE("Oscillator Fine Tuning", "[oscillator][tuning]") {
    Oscillator osc;
    osc.prepare(SAMPLE_RATE, BUFFER_SIZE);
    osc.setFrequency(440.0f);

    SECTION("Can set fine tune") {
        REQUIRE_NOTHROW(osc.setFineTune(0.0f));
        REQUIRE_NOTHROW(osc.setFineTune(25.0f));
        REQUIRE_NOTHROW(osc.setFineTune(-25.0f));
        REQUIRE_NOTHROW(osc.setFineTune(50.0f));
        REQUIRE_NOTHROW(osc.setFineTune(-50.0f));
    }

    SECTION("Fine tune affects frequency") {
        osc.setWaveform(Oscillator::Waveform::Sawtooth);

        // Generate signal with no detuning
        osc.setFineTune(0.0f);
        std::vector<float> signal1(1000);
        for (size_t i = 0; i < signal1.size(); ++i) {
            signal1[i] = osc.processSample(0.0f);
        }

        // Reset and generate with detuning
        osc.reset();
        osc.setFineTune(50.0f);
        std::vector<float> signal2(1000);
        for (size_t i = 0; i < signal2.size(); ++i) {
            signal2[i] = osc.processSample(0.0f);
        }

        // Signals should be different
        bool different = false;
        for (size_t i = 100; i < 200; ++i) { // Skip transient
            if (std::abs(signal1[i] - signal2[i]) > 0.1f) {
                different = true;
                break;
            }
        }
        REQUIRE(different);
    }
}

TEST_CASE("Oscillator Real-Time Safety", "[oscillator][realtime]") {
    Oscillator osc;
    osc.prepare(SAMPLE_RATE, BUFFER_SIZE);
    osc.setFrequency(440.0f);

    SECTION("No NaN or Inf outputs") {
        osc.setWaveform(Oscillator::Waveform::Sawtooth);

        for (int i = 0; i < 10000; ++i) {
            float output = osc.processSample(0.0f);
            REQUIRE(std::isfinite(output));
            REQUIRE(!std::isnan(output));
            REQUIRE(!std::isinf(output));
        }
    }

    SECTION("Handles extreme frequencies") {
        // Very low frequency
        osc.setFrequency(10.0f);
        for (int i = 0; i < 100; ++i) {
            float output = osc.processSample(0.0f);
            REQUIRE(std::isfinite(output));
        }

        // Very high frequency (near Nyquist)
        osc.setFrequency(20000.0f);
        for (int i = 0; i < 100; ++i) {
            float output = osc.processSample(0.0f);
            REQUIRE(std::isfinite(output));
        }
    }
}

TEST_CASE("Oscillator Phase Continuity", "[oscillator][phase]") {
    Oscillator osc;
    osc.prepare(SAMPLE_RATE, BUFFER_SIZE);
    osc.setFrequency(440.0f);
    osc.setWaveform(Oscillator::Waveform::Sawtooth);

    SECTION("Reset returns to initial phase") {
        // Generate some samples
        for (int i = 0; i < 1000; ++i) {
            osc.processSample(0.0f);
        }

        // Reset
        osc.reset();
        float sample1 = osc.processSample(0.0f);

        // Reset again
        osc.reset();
        float sample2 = osc.processSample(0.0f);

        // Should get same first sample
        REQUIRE_THAT(sample1, WithinRel(sample2, 0.001f));
    }
}
