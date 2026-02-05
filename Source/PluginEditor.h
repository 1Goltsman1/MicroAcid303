#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

/**
 * Custom LookAndFeel for 303 inspired aesthetic
 */
class MicroAcidLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MicroAcidLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonWidth, int buttonHeight,
                     juce::ComboBox& box) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    juce::Font getLabelFont(juce::Label& label) override;

private:
    // Authentic vintage 303 color palette
    juce::Colour metalGrey = juce::Colour(0xff8a8a8a);
    juce::Colour darkGrey = juce::Colour(0xff3a3a3a);
    juce::Colour lightGrey = juce::Colour(0xffc0c0c0);
    juce::Colour orangeAccent = juce::Colour(0xffff6600);  // Classic 303 orange
    juce::Colour blueAccent = juce::Colour(0xff00aaff);    // LED blue for displays
};

/**
 * Plugin Editor (GUI) for the TB-Style Bassline.
 *
 * This class creates and manages the user interface, including:
 * - Knobs and sliders for all parameters
 * - Visual feedback (waveform, envelope display)
 * - 303 inspired retro aesthetic
 */
class MicroAcid303AudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    MicroAcid303AudioProcessorEditor (MicroAcid303AudioProcessor&);
    ~MicroAcid303AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    //==============================================================================
    // Helper methods
    void setupRotarySlider(juce::Slider& slider, const juce::String& suffix = "");
    void setupLinearSlider(juce::Slider& slider, const juce::String& suffix = "");
    void setupLabel(juce::Label& label, const juce::String& text);
    void drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);

    //==============================================================================
    // Reference to the processor
    MicroAcid303AudioProcessor& m_audioProcessor;

    // Custom LookAndFeel
    MicroAcidLookAndFeel m_microAcidLookAndFeel;

    //==============================================================================
    // OSCILLATOR SECTION
    juce::ComboBox m_waveformSelector;
    juce::Label m_waveformLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_waveformAttachment;

    juce::Slider m_fineTuneSlider;
    juce::Label m_fineTuneLabel;
    juce::Label m_fineTuneValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_fineTuneAttachment;

    //==============================================================================
    // FILTER SECTION
    juce::Slider m_cutoffSlider;
    juce::Label m_cutoffLabel;
    juce::Label m_cutoffValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_cutoffAttachment;

    juce::Slider m_resonanceSlider;
    juce::Label m_resonanceLabel;
    juce::Label m_resonanceValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_resonanceAttachment;

    juce::Slider m_envModSlider;
    juce::Label m_envModLabel;
    juce::Label m_envModValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_envModAttachment;

    //==============================================================================
    // ENVELOPE SECTION
    juce::Slider m_decaySlider;
    juce::Label m_decayLabel;
    juce::Label m_decayValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_decayAttachment;

    juce::Slider m_accentSlider;
    juce::Label m_accentLabel;
    juce::Label m_accentValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_accentAttachment;

    //==============================================================================
    // SLIDE SECTION
    juce::Slider m_slideTimeSlider;
    juce::Label m_slideTimeLabel;
    juce::Label m_slideTimeValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_slideTimeAttachment;

    //==============================================================================
    // OVERDRIVE SECTION
    juce::Slider m_driveSlider;
    juce::Label m_driveLabel;
    juce::Label m_driveValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_driveAttachment;

    juce::ComboBox m_driveModeSelector;
    juce::Label m_driveModeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_driveModeAttachment;

    //==============================================================================
    // EFFECTS SECTION
    juce::ComboBox m_fxTypeSelector;
    juce::Label m_fxTypeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_fxTypeAttachment;

    juce::Slider m_fxTimeSlider;
    juce::Label m_fxTimeLabel;
    juce::Label m_fxTimeValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_fxTimeAttachment;

    juce::Slider m_fxFeedbackSlider;
    juce::Label m_fxFeedbackLabel;
    juce::Label m_fxFeedbackValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_fxFeedbackAttachment;

    juce::Slider m_fxMixSlider;
    juce::Label m_fxMixLabel;
    juce::Label m_fxMixValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_fxMixAttachment;

    //==============================================================================
    // ARPEGGIATOR SECTION
    juce::ToggleButton m_arpEnabledButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> m_arpEnabledAttachment;

    juce::ComboBox m_arpModeSelector;
    juce::Label m_arpModeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_arpModeAttachment;

    juce::ComboBox m_arpDivisionSelector;
    juce::Label m_arpDivisionLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> m_arpDivisionAttachment;

    juce::Slider m_arpGateSlider;
    juce::Label m_arpGateLabel;
    juce::Label m_arpGateValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_arpGateAttachment;

    juce::Slider m_arpOctavesSlider;
    juce::Label m_arpOctavesLabel;
    juce::Label m_arpOctavesValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_arpOctavesAttachment;

    juce::Slider m_arpSwingSlider;
    juce::Label m_arpSwingLabel;
    juce::Label m_arpSwingValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_arpSwingAttachment;

    //==============================================================================
    // OUTPUT SECTION
    juce::Slider m_outputGainSlider;
    juce::Label m_outputGainLabel;
    juce::Label m_outputGainValueLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> m_outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicroAcid303AudioProcessorEditor)
};
