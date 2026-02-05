#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

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
    m_overdrive = std::make_unique<Overdrive>();
    m_effects = std::make_unique<Effects>();
    m_arpeggiator = std::make_unique<Arpeggiator>();
}

MicroAcid303AudioProcessor::~MicroAcid303AudioProcessor()
{
}

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
    return 2.0; // Effects tail
}

int MicroAcid303AudioProcessor::getNumPrograms()
{
    return 1;
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

void MicroAcid303AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    m_sampleRate = sampleRate;
    m_samplesPerBlock = samplesPerBlock;

    if (m_oscillator)
        m_oscillator->prepare(sampleRate, samplesPerBlock);

    if (m_envelope)
        m_envelope->prepare(sampleRate, samplesPerBlock);

    if (m_filter)
        m_filter->prepare(sampleRate, samplesPerBlock);

    if (m_overdrive)
        m_overdrive->prepare(sampleRate, samplesPerBlock);

    if (m_effects)
        m_effects->prepare(sampleRate, samplesPerBlock);

    if (m_arpeggiator)
        m_arpeggiator->prepare(sampleRate);
}

void MicroAcid303AudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MicroAcid303AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono())
        return true;
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

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get playhead info for arpeggiator
    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            if (posInfo->getBpm())
                m_bpm = *posInfo->getBpm();
            if (posInfo->getTimeInSamples())
                m_samplePosition = *posInfo->getTimeInSamples();
        }
    }

    // Update ALL parameters
    updateOscillatorParameters();
    updateEnvelopeParameters();
    updateFilterParameters();
    updateOverdriveParameters();
    updateEffectsParameters();
    updateArpeggiatorParameters();

    // Get output gain
    auto* outputGainParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::OUTPUT_GAIN));
    float outputGainDb = outputGainParam ? outputGainParam->get() : 0.0f;
    float outputGain = juce::Decibels::decibelsToGain(outputGainDb);

    // Check if arpeggiator is enabled
    auto* arpEnabledParam = dynamic_cast<juce::AudioParameterBool*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_ENABLED));
    bool arpEnabled = arpEnabledParam ? arpEnabledParam->get() : false;

    // Process MIDI (with or without arpeggiator)
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();

        if (arpEnabled && m_arpeggiator)
        {
            // Feed notes to arpeggiator
            if (msg.isNoteOn())
                m_arpeggiator->noteOn(msg.getNoteNumber(), msg.getVelocity() / 127.0f);
            else if (msg.isNoteOff())
                m_arpeggiator->noteOff(msg.getNoteNumber());
            else if (msg.isAllNotesOff())
                m_arpeggiator->allNotesOff();
        }
        else
        {
            // Direct MIDI handling
            handleMidiMessage(msg);
        }
    }

    // Generate audio
    auto* channelData = buffer.getWritePointer(0);
    const int numSamples = buffer.getNumSamples();

    if (!m_oscillator || !m_envelope || !m_filter || !m_overdrive || !m_effects)
    {
        buffer.clear();
        return;
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Process arpeggiator timing
        if (arpEnabled && m_arpeggiator)
        {
            if (m_arpeggiator->process(m_bpm, m_samplePosition + sample))
            {
                // Arpeggiator triggered a new note
                if (m_arpeggiator->isNoteActive())
                {
                    int note = m_arpeggiator->getCurrentNote();
                    float vel = m_arpeggiator->getCurrentVelocity();

                    m_currentNote = note;
                    m_currentVelocity = vel;
                    m_isNoteActive = true;

                    if (m_oscillator)
                        m_oscillator->setFrequency(midiNoteToFrequency(note));
                    if (m_envelope)
                        m_envelope->noteOn();
                }
            }

            // Check if gate closed
            if (!m_arpeggiator->isNoteActive() && m_isNoteActive)
            {
                m_isNoteActive = false;
                if (m_envelope)
                    m_envelope->noteOff();
            }
        }

        // 1. Generate oscillator
        float signal = m_oscillator->processSample(0.0f);

        // 2. Get envelope
        float envValue = m_envelope->processSample(0.0f);

        // 3. Apply envelope to amplitude with accent
        float amplitude = m_currentVelocity * (1.0f + m_accentAmount * 0.5f);
        signal *= envValue * amplitude;

        // 4. Apply filter with envelope modulation
        m_filter->setEnvelopeValue(envValue);
        signal = m_filter->processSample(signal);

        // 5. Apply overdrive
        signal = m_overdrive->processSample(signal);

        // 6. Apply effects
        signal = m_effects->processSample(signal);

        // 7. Apply output gain
        signal *= outputGain;

        // 8. Final soft clip
        signal = std::tanh(signal * 0.9f);

        channelData[sample] = signal;
    }

    // Copy mono to stereo if needed
    if (totalNumOutputChannels > 1)
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);

    m_samplePosition += numSamples;
}

bool MicroAcid303AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MicroAcid303AudioProcessor::createEditor()
{
    return new MicroAcid303AudioProcessorEditor (*this);
}

void MicroAcid303AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = m_parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MicroAcid303AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (m_parameters.state.getType()))
            m_parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

void MicroAcid303AudioProcessor::handleMidiMessage(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        m_currentNote = message.getNoteNumber();
        m_currentVelocity = message.getVelocity() / 127.0f;
        m_isNoteActive = true;

        if (m_oscillator)
            m_oscillator->setFrequency(midiNoteToFrequency(m_currentNote));

        if (m_envelope)
            m_envelope->noteOn();
    }
    else if (message.isNoteOff())
    {
        if (message.getNoteNumber() == m_currentNote)
        {
            m_isNoteActive = false;
            m_currentNote = -1;
            m_currentVelocity = 0.0f;

            if (m_envelope)
                m_envelope->noteOff();
        }
    }
    else if (message.isAllNotesOff())
    {
        m_isNoteActive = false;
        m_currentNote = -1;
        m_currentVelocity = 0.0f;

        if (m_envelope)
            m_envelope->noteOff();

        if (m_arpeggiator)
            m_arpeggiator->allNotesOff();
    }
}

void MicroAcid303AudioProcessor::updateOscillatorParameters()
{
    if (!m_oscillator) return;

    // Waveform
    auto* waveformParam = dynamic_cast<juce::AudioParameterChoice*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::WAVEFORM));
    if (waveformParam)
        m_oscillator->setWaveform(waveformParam->getIndex());

    // Fine tune
    auto* fineTuneParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::FINE_TUNE));
    if (fineTuneParam)
        m_oscillator->setFineTune(fineTuneParam->get());

    // Slide time
    auto* slideParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::SLIDE_TIME));
    if (slideParam)
        m_oscillator->setSlideTime(slideParam->get());
}

void MicroAcid303AudioProcessor::updateEnvelopeParameters()
{
    if (!m_envelope) return;

    auto* decayParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::DECAY));
    if (decayParam)
    {
        float decayTime = decayParam->get();
        m_envelope->setAttack(0.001f);
        m_envelope->setDecay(decayTime);
        m_envelope->setSustain(0.0f);
        m_envelope->setRelease(0.01f);
    }

    auto* accentParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ACCENT));
    if (accentParam)
        m_accentAmount = accentParam->get();
}

void MicroAcid303AudioProcessor::updateFilterParameters()
{
    if (!m_filter) return;

    auto* cutoffParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::CUTOFF));
    if (cutoffParam)
        m_filter->setCutoff(cutoffParam->get());

    auto* resonanceParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::RESONANCE));
    if (resonanceParam)
        m_filter->setResonance(resonanceParam->get());

    auto* envModParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ENV_MOD));
    if (envModParam)
        m_filter->setEnvelopeAmount(envModParam->get());
}

void MicroAcid303AudioProcessor::updateOverdriveParameters()
{
    if (!m_overdrive) return;

    auto* driveParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::DRIVE));
    if (driveParam)
        m_overdrive->setDrive(driveParam->get());

    auto* driveModeParam = dynamic_cast<juce::AudioParameterChoice*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::DRIVE_MODE));
    if (driveModeParam)
        m_overdrive->setMode(driveModeParam->getIndex());
}

void MicroAcid303AudioProcessor::updateEffectsParameters()
{
    if (!m_effects) return;

    auto* fxTypeParam = dynamic_cast<juce::AudioParameterChoice*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::FX_TYPE));
    if (fxTypeParam)
        m_effects->setType(fxTypeParam->getIndex());

    auto* fxTimeParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::FX_TIME));
    if (fxTimeParam)
        m_effects->setTime(fxTimeParam->get());

    auto* fxFeedbackParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::FX_FEEDBACK));
    if (fxFeedbackParam)
        m_effects->setFeedback(fxFeedbackParam->get());

    auto* fxMixParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::FX_MIX));
    if (fxMixParam)
        m_effects->setMix(fxMixParam->get());
}

void MicroAcid303AudioProcessor::updateArpeggiatorParameters()
{
    if (!m_arpeggiator) return;

    auto* arpEnabledParam = dynamic_cast<juce::AudioParameterBool*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_ENABLED));
    if (arpEnabledParam)
        m_arpeggiator->setEnabled(arpEnabledParam->get());

    auto* arpModeParam = dynamic_cast<juce::AudioParameterChoice*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_MODE));
    if (arpModeParam)
        m_arpeggiator->setMode(arpModeParam->getIndex());

    auto* arpDivParam = dynamic_cast<juce::AudioParameterChoice*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_DIVISION));
    if (arpDivParam)
        m_arpeggiator->setDivision(arpDivParam->getIndex());

    auto* arpGateParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_GATE));
    if (arpGateParam)
        m_arpeggiator->setGate(arpGateParam->get());

    auto* arpOctParam = dynamic_cast<juce::AudioParameterInt*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_OCTAVES));
    if (arpOctParam)
        m_arpeggiator->setOctaves(arpOctParam->get());

    auto* arpSwingParam = dynamic_cast<juce::AudioParameterFloat*>(
        m_parameters.getParameter(MicroAcidParameters::IDs::ARP_SWING));
    if (arpSwingParam)
        m_arpeggiator->setSwing(arpSwingParam->get());
}

float MicroAcid303AudioProcessor::midiNoteToFrequency(int midiNote)
{
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicroAcid303AudioProcessor();
}
