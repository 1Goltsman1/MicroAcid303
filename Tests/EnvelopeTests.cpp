#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <vector>

// Include envelope
#include "dsp/Envelope.h"

using namespace Catch::Matchers;

constexpr float SAMPLE_RATE = 44100.0f;
constexpr int BUFFER_SIZE = 512;

TEST_CASE("Envelope Basic Construction", "[envelope][construction]") {
    SECTION("Can create envelope instance") {
        REQUIRE_NOTHROW(Envelope());
    }

    SECTION("Initializes and prepares") {
        Envelope env;
        REQUIRE_NOTHROW(env.prepare(SAMPLE_RATE, BUFFER_SIZE));
    }
}

TEST_CASE("Envelope DSPModule Interface", "[envelope][interface]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Can reset") {
        REQUIRE_NOTHROW(env.reset());
    }

    SECTION("Can process samples") {
        float output = env.processSample(0.0f);
        REQUIRE(std::isfinite(output));
    }

    SECTION("Initial state is Idle") {
        REQUIRE(env.getCurrentStage() == Envelope::Stage::Idle);
        REQUIRE(env.getCurrentLevel() == 0.0f);
        REQUIRE_FALSE(env.isActive());
    }
}

TEST_CASE("Envelope Note On/Off", "[envelope][trigger]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Note on triggers attack") {
        env.noteOn();
        REQUIRE(env.getCurrentStage() == Envelope::Stage::Attack);
        REQUIRE(env.isActive());
    }

    SECTION("Note off triggers release") {
        env.noteOn();
        env.processSample(0.0f); // Process one sample
        env.noteOff();
        // Should eventually be in release, but might take a few samples
        for (int i = 0; i < 100; ++i) {
            env.processSample(0.0f);
        }
        // After note off and processing, should be releasing or idle
        auto stage = env.getCurrentStage();
        REQUIRE((stage == Envelope::Stage::Release || stage == Envelope::Stage::Idle));
    }

    SECTION("Reset returns to idle") {
        env.noteOn();
        env.processSample(0.0f);
        env.reset();
        REQUIRE(env.getCurrentStage() == Envelope::Stage::Idle);
        REQUIRE(env.getCurrentLevel() == 0.0f);
    }
}

TEST_CASE("Envelope ADSR Stages", "[envelope][stages]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);

    // Set short times for faster testing
    env.setAttack(0.01f);   // 10ms
    env.setDecay(0.01f);    // 10ms
    env.setSustain(0.5f);   // 50%
    env.setRelease(0.01f);  // 10ms

    SECTION("Attack stage increases level") {
        env.noteOn();

        float prevLevel = 0.0f;
        bool increased = false;

        for (int i = 0; i < 100; ++i) {
            float level = env.processSample(0.0f);
            if (level > prevLevel) {
                increased = true;
            }
            prevLevel = level;
        }

        REQUIRE(increased);
        REQUIRE(prevLevel > 0.0f);
    }

    SECTION("Attack reaches peak then decays") {
        env.noteOn();

        // Process through attack
        for (int i = 0; i < 1000; ++i) {
            env.processSample(0.0f);
        }

        // Should have reached decay or sustain
        auto stage = env.getCurrentStage();
        REQUIRE((stage == Envelope::Stage::Decay || stage == Envelope::Stage::Sustain));
    }

    SECTION("Sustain holds level") {
        env.noteOn();

        // Process until sustain
        for (int i = 0; i < 2000; ++i) {
            env.processSample(0.0f);
        }

        // Should be in sustain
        REQUIRE(env.getCurrentStage() == Envelope::Stage::Sustain);

        // Level should remain stable
        float level1 = env.processSample(0.0f);
        float level2 = env.processSample(0.0f);
        float level3 = env.processSample(0.0f);

        REQUIRE_THAT(level1, WithinRel(level2, 0.01f));
        REQUIRE_THAT(level2, WithinRel(level3, 0.01f));
        REQUIRE_THAT(level1, WithinRel(0.5f, 0.1f)); // Near sustain level
    }

    SECTION("Release decreases level") {
        env.noteOn();

        // Get to sustain
        for (int i = 0; i < 2000; ++i) {
            env.processSample(0.0f);
        }

        float sustainLevel = env.getCurrentLevel();

        // Trigger release
        env.noteOff();

        // Process release
        for (int i = 0; i < 100; ++i) {
            env.processSample(0.0f);
        }

        float releaseLevel = env.getCurrentLevel();

        REQUIRE(releaseLevel < sustainLevel);
    }

    SECTION("Release eventually reaches idle") {
        env.noteOn();

        // Quick attack/decay
        for (int i = 0; i < 1000; ++i) {
            env.processSample(0.0f);
        }

        env.noteOff();

        // Process until idle
        for (int i = 0; i < 5000; ++i) {
            env.processSample(0.0f);
        }

        REQUIRE(env.getCurrentStage() == Envelope::Stage::Idle);
        REQUIRE(env.getCurrentLevel() < 0.01f);
    }
}

TEST_CASE("Envelope Parameter Setting", "[envelope][parameters]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("Can set attack time") {
        REQUIRE_NOTHROW(env.setAttack(0.1f));
        REQUIRE_NOTHROW(env.setAttack(1.0f));
        REQUIRE_NOTHROW(env.setAttack(0.001f));
    }

    SECTION("Can set decay time") {
        REQUIRE_NOTHROW(env.setDecay(0.1f));
        REQUIRE_NOTHROW(env.setDecay(1.0f));
        REQUIRE_NOTHROW(env.setDecay(0.001f));
    }

    SECTION("Can set sustain level") {
        REQUIRE_NOTHROW(env.setSustain(0.0f));
        REQUIRE_NOTHROW(env.setSustain(0.5f));
        REQUIRE_NOTHROW(env.setSustain(1.0f));
    }

    SECTION("Can set release time") {
        REQUIRE_NOTHROW(env.setRelease(0.1f));
        REQUIRE_NOTHROW(env.setRelease(1.0f));
        REQUIRE_NOTHROW(env.setRelease(0.001f));
    }

    SECTION("Parameters affect envelope shape") {
        // Fast attack
        env.setAttack(0.001f);
        env.noteOn();

        int samplesTo90Percent = 0;
        for (int i = 0; i < 1000; ++i) {
            float level = env.processSample(0.0f);
            if (level >= 0.9f) {
                samplesTo90Percent = i;
                break;
            }
        }

        // Should reach 90% quickly with fast attack
        REQUIRE(samplesTo90Percent < 100);

        // Reset for slow attack test
        env.reset();
        env.setAttack(0.1f); // 100ms
        env.noteOn();

        samplesTo90Percent = 0;
        for (int i = 0; i < 10000; ++i) {
            float level = env.processSample(0.0f);
            if (level >= 0.9f) {
                samplesTo90Percent = i;
                break;
            }
        }

        // Should take longer with slow attack
        REQUIRE(samplesTo90Percent > 100);
    }
}

TEST_CASE("Envelope Real-Time Safety", "[envelope][realtime]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);

    SECTION("No NaN or Inf outputs") {
        env.setAttack(0.01f);
        env.setDecay(0.1f);
        env.setSustain(0.7f);
        env.setRelease(0.2f);
        env.noteOn();

        for (int i = 0; i < 10000; ++i) {
            float output = env.processSample(0.0f);
            REQUIRE(std::isfinite(output));
            REQUIRE(!std::isnan(output));
            REQUIRE(!std::isinf(output));
        }
    }

    SECTION("Output always in valid range") {
        env.setAttack(0.01f);
        env.setDecay(0.1f);
        env.setSustain(0.7f);
        env.setRelease(0.2f);
        env.noteOn();

        for (int i = 0; i < 10000; ++i) {
            float output = env.processSample(0.0f);
            REQUIRE(output >= 0.0f);
            REQUIRE(output <= 1.0f);
        }
    }

    SECTION("Handles rapid note on/off") {
        for (int cycle = 0; cycle < 100; ++cycle) {
            env.noteOn();
            for (int i = 0; i < 10; ++i) {
                float output = env.processSample(0.0f);
                REQUIRE(std::isfinite(output));
            }
            env.noteOff();
            for (int i = 0; i < 10; ++i) {
                float output = env.processSample(0.0f);
                REQUIRE(std::isfinite(output));
            }
        }
    }
}

TEST_CASE("Envelope Retrigger Behavior", "[envelope][retrigger]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);
    env.setAttack(0.05f);
    env.setDecay(0.1f);
    env.setSustain(0.6f);
    env.setRelease(0.1f);

    SECTION("Can retrigger during attack") {
        env.noteOn();

        // Process partway through attack
        for (int i = 0; i < 100; ++i) {
            env.processSample(0.0f);
        }

        // Retrigger
        env.noteOn();

        REQUIRE(env.getCurrentStage() == Envelope::Stage::Attack);
    }

    SECTION("Can retrigger during decay") {
        env.noteOn();

        // Process through attack and into decay
        for (int i = 0; i < 3000; ++i) {
            env.processSample(0.0f);
        }

        // Retrigger
        env.noteOn();

        REQUIRE(env.getCurrentStage() == Envelope::Stage::Attack);
    }

    SECTION("Can retrigger during sustain") {
        env.noteOn();

        // Process to sustain
        for (int i = 0; i < 10000; ++i) {
            env.processSample(0.0f);
        }

        REQUIRE(env.getCurrentStage() == Envelope::Stage::Sustain);

        // Retrigger
        env.noteOn();

        REQUIRE(env.getCurrentStage() == Envelope::Stage::Attack);
    }

    SECTION("Can retrigger during release") {
        env.noteOn();

        // Get to sustain
        for (int i = 0; i < 5000; ++i) {
            env.processSample(0.0f);
        }

        env.noteOff();

        // Partway through release
        for (int i = 0; i < 100; ++i) {
            env.processSample(0.0f);
        }

        // Retrigger
        env.noteOn();

        REQUIRE(env.getCurrentStage() == Envelope::Stage::Attack);
    }
}

TEST_CASE("Envelope Exponential Curves", "[envelope][curve]") {
    Envelope env;
    env.prepare(SAMPLE_RATE, BUFFER_SIZE);
    env.setAttack(0.1f);
    env.setDecay(0.1f);
    env.setSustain(0.5f);
    env.setRelease(0.1f);

    SECTION("Attack has exponential characteristic") {
        env.noteOn();

        std::vector<float> levels;
        for (int i = 0; i < 500; ++i) {
            levels.push_back(env.processSample(0.0f));
        }

        // Early samples should increase faster than later samples
        float earlyRate = levels[50] - levels[10];
        float lateRate = levels[400] - levels[360];

        REQUIRE(earlyRate > lateRate);
    }

    SECTION("Release has exponential characteristic") {
        env.noteOn();

        // Get to sustain
        for (int i = 0; i < 10000; ++i) {
            env.processSample(0.0f);
        }

        env.noteOff();

        std::vector<float> levels;
        for (int i = 0; i < 500; ++i) {
            levels.push_back(env.processSample(0.0f));
        }

        // Early samples should decrease faster than later samples
        float earlyRate = std::abs(levels[10] - levels[50]);
        float lateRate = std::abs(levels[360] - levels[400]);

        REQUIRE(earlyRate > lateRate);
    }
}
