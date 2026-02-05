#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "core/Parameters.h"
#include "dsp/Oscillator.h"
#include "dsp/Envelope.h"
#include "dsp/LadderFilter.h"
#include "dsp/Overdrive.h"
#include "dsp/Effects.h"
#include "dsp/Arpeggiator.h"

/**
 * Main audio processor for the 303 Micro Acid plugin.
 */
class MicroAcid303AudioProcessor : public juce::AudioProcessor
{
public:
    MicroAcid303AudioProcessor();
    ~MicroAcid303AudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return m_parameters; }

private:
    void handleMidiMessage(const juce::MidiMessage& message);
    void updateOscillatorParameters();
    void updateEnvelopeParameters();
    void updateFilterParameters();
    void updateOverdriveParameters();
    void updateEffectsParameters();
    void updateArpeggiatorParameters();
    float midiNoteToFrequency(int midiNote);

    juce::AudioProcessorValueTreeState m_parameters;

    // DSP modules
    std::unique_ptr<Oscillator> m_oscillator;
    std::unique_ptr<Envelope> m_envelope;
    std::unique_ptr<LadderFilter> m_filter;
    std::unique_ptr<Overdrive> m_overdrive;
    std::unique_ptr<Effects> m_effects;
    std::unique_ptr<Arpeggiator> m_arpeggiator;

    // Voice state (monophonic)
    int m_currentNote = -1;
    float m_currentVelocity = 0.0f;
    bool m_isNoteActive = false;
    float m_accentAmount = 0.0f;

    // Playhead info for arpeggiator
    double m_bpm = 120.0;
    int64_t m_samplePosition = 0;

    // Sample rate storage
    double m_sampleRate = 44100.0;
    int m_samplesPerBlock = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroAcid303AudioProcessor)
};
