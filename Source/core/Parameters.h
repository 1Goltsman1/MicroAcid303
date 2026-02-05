#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * Central parameter definitions for the 303 Micro Acid plugin.
 *
 * All parameter IDs, ranges, defaults, and metadata are defined here
 * to ensure consistency across the codebase.
 */
namespace MicroAcidParameters
{
    // Parameter IDs (used for identification in AudioProcessorValueTreeState)
    namespace IDs
    {
        // Oscillator
        const juce::String WAVEFORM            = "waveform";
        const juce::String FINE_TUNE           = "fineTune";

        // Filter
        const juce::String CUTOFF              = "cutoff";
        const juce::String RESONANCE           = "resonance";
        const juce::String ENV_MOD             = "envMod";

        // Envelope
        const juce::String DECAY               = "decay";
        const juce::String ACCENT              = "accent";

        // Slide
        const juce::String SLIDE_TIME          = "slideTime";

        // Overdrive
        const juce::String DRIVE               = "drive";
        const juce::String DRIVE_MODE          = "driveMode";

        // Effects
        const juce::String FX_TYPE             = "fxType";
        const juce::String FX_TIME             = "fxTime";
        const juce::String FX_FEEDBACK         = "fxFeedback";
        const juce::String FX_MIX              = "fxMix";

        // Output
        const juce::String OUTPUT_GAIN         = "outputGain";
    }

    // Create the parameter layout for AudioProcessorValueTreeState
    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // OSCILLATOR
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::WAVEFORM,
            "Waveform",
            juce::StringArray{"Saw", "Square"},
            0  // Default to Saw
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FINE_TUNE,
            "Fine Tune",
            juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
            0.0f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " cents"; }
        ));

        // FILTER
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::CUTOFF,
            "Cutoff",
            juce::NormalisableRange<float>(
                20.0f,     // min
                4000.0f,   // max
                0.1f,      // step
                0.3f       // skew factor (emphasize lower frequencies)
            ),
            1000.0f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " Hz"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::RESONANCE,
            "Resonance",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.5f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + " %"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::ENV_MOD,
            "Envelope Mod",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.5f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + " %"; }
        ));

        // ENVELOPE
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::DECAY,
            "Decay",
            juce::NormalisableRange<float>(
                0.01f,    // min
                2.0f,     // max
                0.01f,    // step
                0.5f      // skew factor
            ),
            0.5f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 2) + " s"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::ACCENT,
            "Accent",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + " %"; }
        ));

        // SLIDE
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::SLIDE_TIME,
            "Slide Time",
            juce::NormalisableRange<float>(0.01f, 0.5f, 0.01f),
            0.1f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 2) + " s"; }
        ));

        // OVERDRIVE
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::DRIVE,
            "Drive",
            juce::NormalisableRange<float>(
                1.0f,     // min
                10.0f,    // max
                0.1f,     // step
                0.4f      // skew factor
            ),
            1.0f,  // Default (no drive)
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1); }
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::DRIVE_MODE,
            "Drive Mode",
            juce::StringArray{"Soft", "Classic", "Saturated"},
            1  // Default to Classic
        ));

        // EFFECTS
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::FX_TYPE,
            "FX Type",
            juce::StringArray{"Tape Delay", "Digital Delay", "Reverb"},
            0  // Default to Tape Delay
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FX_TIME,
            "FX Time",
            juce::NormalisableRange<float>(
                10.0f,     // min
                2000.0f,   // max
                1.0f,      // step
                0.3f       // skew factor
            ),
            250.0f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " ms"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FX_FEEDBACK,
            "Feedback",
            juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
            0.5f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + " %"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FX_MIX,
            "FX Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.3f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + " %"; }
        ));

        // OUTPUT
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::OUTPUT_GAIN,
            "Output Gain",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
            0.0f,  // Default
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }
        ));

        return { params.begin(), params.end() };
    }
}
