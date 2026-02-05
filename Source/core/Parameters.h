#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

/**
 * Central parameter definitions for the 303 Micro Acid plugin.
 */
namespace MicroAcidParameters
{
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

        // Arpeggiator
        const juce::String ARP_ENABLED         = "arpEnabled";
        const juce::String ARP_MODE            = "arpMode";
        const juce::String ARP_DIVISION        = "arpDivision";
        const juce::String ARP_GATE            = "arpGate";
        const juce::String ARP_OCTAVES         = "arpOctaves";
        const juce::String ARP_SWING           = "arpSwing";

        // Output
        const juce::String OUTPUT_GAIN         = "outputGain";
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // OSCILLATOR - 12 waveforms
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::WAVEFORM,
            "Waveform",
            juce::StringArray{
                "Saw", "Square", "Triangle", "Sine",
                "Pulse 25%", "Pulse 12%", "SuperSaw", "Noise",
                "Saw+Sqr", "Tri+Saw", "Sync", "FM"
            },
            0
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FINE_TUNE,
            "Fine Tune",
            juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f),
            0.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " ct"; }
        ));

        // FILTER
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::CUTOFF,
            "Cutoff",
            juce::NormalisableRange<float>(20.0f, 4000.0f, 0.1f, 0.3f),
            1000.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " Hz"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::RESONANCE,
            "Resonance",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.5f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::ENV_MOD,
            "Env Mod",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.5f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        // ENVELOPE
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::DECAY,
            "Decay",
            juce::NormalisableRange<float>(0.01f, 2.0f, 0.01f, 0.5f),
            0.5f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 2) + " s"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::ACCENT,
            "Accent",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        // SLIDE
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::SLIDE_TIME,
            "Slide",
            juce::NormalisableRange<float>(0.001f, 0.5f, 0.001f, 0.4f),
            0.05f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 1000)) + " ms"; }
        ));

        // OVERDRIVE - 5 modes
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::DRIVE,
            "Drive",
            juce::NormalisableRange<float>(1.0f, 10.0f, 0.1f, 0.4f),
            1.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + "x"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::DRIVE_MODE,
            "Drive Mode",
            juce::StringArray{"Soft", "Classic", "Saturated", "Fuzz", "Tape"},
            1
        ));

        // EFFECTS - 8 types
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::FX_TYPE,
            "FX Type",
            juce::StringArray{
                "Tape Dly", "Digi Dly", "PingPong",
                "Reverb", "Chorus", "Flanger", "Phaser", "Bitcrush"
            },
            0
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FX_TIME,
            "FX Time",
            juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.3f),
            250.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 0) + " ms"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FX_FEEDBACK,
            "Feedback",
            juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
            0.5f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::FX_MIX,
            "FX Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.3f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        // ARPEGGIATOR
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            IDs::ARP_ENABLED,
            "Arp On",
            false
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::ARP_MODE,
            "Arp Mode",
            juce::StringArray{"Up", "Down", "Up/Down", "Down/Up", "Random", "Order", "Chord"},
            0
        ));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            IDs::ARP_DIVISION,
            "Arp Rate",
            juce::StringArray{
                "1/1", "1/2", "1/4", "1/8", "1/16", "1/32",
                "1/4D", "1/8D", "1/4T", "1/8T"
            },
            3  // Default to 1/8
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::ARP_GATE,
            "Arp Gate",
            juce::NormalisableRange<float>(0.1f, 1.0f, 0.01f),
            0.5f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        params.push_back(std::make_unique<juce::AudioParameterInt>(
            IDs::ARP_OCTAVES,
            "Arp Oct",
            1, 4, 1
        ));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::ARP_SWING,
            "Swing",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            0.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(int(value * 100)) + "%"; }
        ));

        // OUTPUT
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            IDs::OUTPUT_GAIN,
            "Output",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
            0.0f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String(value, 1) + " dB"; }
        ));

        return { params.begin(), params.end() };
    }
}
