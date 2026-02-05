#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
MicroAcid303AudioProcessor::MicroAcid303AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::mono(), true)
                     #endif
                       ),
#endif
      m_parameters (*this, nullptr, juce::Identifier("MicroAcid303"),
                    MicroAcidParameters::createParameterLayout())
{
    // Create DSP modules
    m_oscillator = std::make_unique<Oscillator>();
    m_envelope = std::make_unique<Envelope>();
    m_filter = std::make_unique<LadderFilter>();
}

MicroAcid303AudioProcessor::~MicroAcid303AudioProcessor()
{
}

//==============================================================================
const juce::String MicroAcid303AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MicroAcid303AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MicroAcid303AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MicroAcid303AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MicroAcid303AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MicroAcid303AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MicroAcid303AudioProcessor::getCurrentProgram()
{
    return 0;
}

void MicroAcid303AudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String MicroAcid303AudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void MicroAcid303AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void MicroAcid303AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_sampleRate = sampleRate;
    m_samplesPerBlock = samplesPerBlock;

    // Prepare DSP modules
    if (m_oscillator)
        m_oscillator->prepare(sampleRate, samplesPerBlock);

    if (m_envelope)
        m_envelope->prepare(sampleRate, samplesPerBlock);

    if (m_filter)
        m_filter->prepare(sampleRate, samplesPerBlock);
}

void MicroAcid303AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MicroAcid303AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This plugin outputs mono (303 style)
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono())
        return true;

    // Also support stereo output for effects
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo())
        return true;

    return false;
  #endif
}
#endif

void MicroAcid303AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                  juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear any existing audio data
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update ALL DSP parameters from plugin parameters
    updateOscillatorParameters();
    updateEnvelopeParameters();
    updateFilterParameters();

    // Get output gain parameter
    auto* outputGainParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::OUTPUT_GAIN));
    float outputGainDb = outputGainParam ? outputGainParam->get() : 0.0f;
    float outputGain = juce::Decibels::decibelsToGain(outputGainDb);

    // Process MIDI messages
    for (const auto metadata : midiMessages)
    {
        handleMidiMessage(metadata.getMessage());
    }

    // Generate audio
    auto* channelData = buffer.getWritePointer(0);
    const int numSamples = buffer.getNumSamples();

    // Check if all DSP modules are ready
    if (!m_oscillator || !m_envelope || !m_filter)
    {
        buffer.clear();
        return;
    }

    // Process audio sample by sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1. Generate oscillator output
        float signal = m_oscillator->processSample(0.0f);

        // 2. Get envelope value (always process envelope for smooth curves)
        float envValue = m_envelope->processSample(0.0f);

        // 3. Apply envelope to amplitude
        // Include accent: accent adds extra amplitude when active
        float amplitude = m_currentVelocity * (1.0f + m_accentAmount * 0.5f);
        signal *= envValue * amplitude;

        // 4. Update filter with current envelope value for modulation
        m_filter->setEnvelopeValue(envValue);

        // 5. Process through filter
        signal = m_filter->processSample(signal);

        // 6. Apply output gain
        signal *= outputGain;

        // 7. Soft clip to prevent clipping
        signal = std::tanh(signal * 0.8f);

        // 8. Write to output
        channelData[sample] = signal;
    }

    // If we have stereo output, copy mono to both channels
    if (totalNumOutputChannels > 1)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    }
}

//==============================================================================
bool MicroAcid303AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MicroAcid303AudioProcessor::createEditor()
{
    return new MicroAcid303AudioProcessorEditor (*this);
}

//==============================================================================
void MicroAcid303AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save parameter state
    auto state = m_parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MicroAcid303AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore parameter state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (m_parameters.state.getType()))
            m_parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Helper methods

void MicroAcid303AudioProcessor::handleMidiMessage(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        // Store current note
        m_currentNote = message.getNoteNumber();
        m_currentVelocity = message.getVelocity() / 127.0f;
        m_isNoteActive = true;

        // Set oscillator frequency
        if (m_oscillator)
        {
            float frequency = midiNoteToFrequency(m_currentNote);
            m_oscillator->setFrequency(frequency);
        }

        // Trigger envelope
        if (m_envelope)
        {
            m_envelope->noteOn();
        }
    }
    else if (message.isNoteOff())
    {
        // Only turn off if it's the current note (monophonic behavior)
        if (message.getNoteNumber() == m_currentNote)
        {
            m_isNoteActive = false;
            m_currentNote = -1;
            m_currentVelocity = 0.0f;

            // Trigger envelope release
            if (m_envelope)
            {
                m_envelope->noteOff();
            }
        }
    }
    else if (message.isAllNotesOff())
    {
        m_isNoteActive = false;
        m_currentNote = -1;
        m_currentVelocity = 0.0f;

        // Release envelope
        if (m_envelope)
        {
            m_envelope->noteOff();
        }
    }
}

void MicroAcid303AudioProcessor::updateOscillatorParameters()
{
    if (!m_oscillator)
        return;

    // Update waveform
    auto* waveformParam = dynamic_cast<juce::AudioParameterChoice*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::WAVEFORM));
    if (waveformParam)
    {
        int waveformIndex = waveformParam->getIndex();
        m_oscillator->setWaveform(waveformIndex == 0 ?
            Oscillator::Waveform::Sawtooth :
            Oscillator::Waveform::Square);
    }

    // Update fine tune
    auto* fineTuneParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::FINE_TUNE));
    if (fineTuneParam)
    {
        m_oscillator->setFineTune(fineTuneParam->get());
    }
}

void MicroAcid303AudioProcessor::updateEnvelopeParameters()
{
    if (!m_envelope)
        return;

    // For 303 style, we use a simplified envelope:
    // - Very fast attack (almost instant)
    // - Decay controls the overall envelope time
    // - No sustain (always decays to 0)
    // - Very fast release

    auto* decayParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::DECAY));
    if (decayParam)
    {
        float decayTime = decayParam->get();
        m_envelope->setAttack(0.001f);        // 1ms attack
        m_envelope->setDecay(decayTime);      // User-controlled decay
        m_envelope->setSustain(0.0f);         // No sustain (303 style)
        m_envelope->setRelease(0.01f);        // 10ms release
    }

    // Store accent amount for amplitude modulation
    auto* accentParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ACCENT));
    if (accentParam)
    {
        m_accentAmount = accentParam->get();
    }
}

void MicroAcid303AudioProcessor::updateFilterParameters()
{
    if (!m_filter)
        return;

    // Update cutoff frequency
    auto* cutoffParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::CUTOFF));
    if (cutoffParam)
    {
        m_filter->setCutoff(cutoffParam->get());
    }

    // Update resonance
    auto* resonanceParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::RESONANCE));
    if (resonanceParam)
    {
        m_filter->setResonance(resonanceParam->get());
    }

    // Update envelope modulation amount
    auto* envModParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ENV_MOD));
    if (envModParam)
    {
        m_filter->setEnvelopeAmount(envModParam->get());
    }
}

float MicroAcid303AudioProcessor::midiNoteToFrequency(int midiNote)
{
    // MIDI note to frequency: f = 440 * 2^((n - 69) / 12)
    // where n is the MIDI note number
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroAcid303AudioProcessor();
}
