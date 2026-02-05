#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// MicroAcidLookAndFeel Implementation
//==============================================================================

MicroAcidLookAndFeel::MicroAcidLookAndFeel()
{
    // Set authentic vintage hardware color scheme
    setColour(juce::Slider::thumbColourId, orangeAccent);
    setColour(juce::Slider::rotarySliderFillColourId, blueAccent);
    setColour(juce::Slider::rotarySliderOutlineColourId, darkGrey);
    setColour(juce::Slider::trackColourId, blueAccent);
    setColour(juce::Slider::backgroundColourId, darkGrey);
    setColour(juce::Label::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::backgroundColourId, darkGrey);
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, lightGrey);
    setColour(juce::ComboBox::arrowColourId, orangeAccent);
}

void MicroAcidLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                     float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                     juce::Slider&)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 6.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // DROP SHADOW for depth (vintage hardware lifted off panel)
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillEllipse(rx + 3.0f, ry + 3.0f, rw, rw);

    // OUTER METALLIC BODY with radial brushed metal effect
    juce::ColourGradient outerGradient(
        juce::Colour(0xffdddddd), centreX - radius * 0.7f, centreY - radius * 0.7f,
        juce::Colour(0xff999999), centreX + radius * 0.7f, centreY + radius * 0.7f, true);
    g.setGradientFill(outerGradient);
    g.fillEllipse(rx, ry, rw, rw);

    // Metallic rim highlight (top-left)
    g.setColour(juce::Colour(0xffffffff).withAlpha(0.5f));
    juce::Path topHighlight;
    topHighlight.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                               juce::MathConstants<float>::pi * 1.0f,
                               juce::MathConstants<float>::pi * 1.5f, true);
    g.strokePath(topHighlight, juce::PathStrokeType(2.0f));

    // Metallic rim shadow (bottom-right)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.3f));
    juce::Path bottomShadow;
    bottomShadow.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                               juce::MathConstants<float>::pi * 0.0f,
                               juce::MathConstants<float>::pi * 0.5f, true);
    g.strokePath(bottomShadow, juce::PathStrokeType(2.0f));

    // INNER DARK BACKGROUND (recessed area)
    auto innerRadius = radius - 10.0f;

    // Inner shadow for depth
    g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
    g.fillEllipse(centreX - innerRadius + 1.0f, centreY - innerRadius + 1.0f,
                  innerRadius * 2.0f, innerRadius * 2.0f);

    // Dark inner area
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius,
                  innerRadius * 2.0f, innerRadius * 2.0f);

    // RANGE ARC (subtle gray showing full range)
    juce::Path trackArc;
    trackArc.addCentredArc(centreX, centreY, innerRadius - 5.0f, innerRadius - 5.0f,
                          0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff3a3a3a));
    g.strokePath(trackArc, juce::PathStrokeType(2.5f));

    // VALUE ARC (bright blue showing current value)
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, innerRadius - 5.0f, innerRadius - 5.0f,
                          0.0f, rotaryStartAngle, angle, true);
    g.setColour(blueAccent.withAlpha(0.9f));
    g.strokePath(valueArc, juce::PathStrokeType(3.0f));

    // VALUE ARC GLOW (LED-like effect)
    g.setColour(blueAccent.withAlpha(0.3f));
    g.strokePath(valueArc, juce::PathStrokeType(5.0f));

    // POINTER INDICATOR (thick orange line - very 303 MICRO ACID!)
    juce::Path pointer;
    auto pointerLength = innerRadius - 10.0f;
    auto pointerThickness = 4.0f;
    pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength,
                        pointerThickness, pointerLength - 8.0f);

    // Pointer shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle)
                                           .translated(centreX + 0.5f, centreY + 0.5f));

    // Main pointer (bright orange)
    g.setColour(orangeAccent);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    // CENTER CAP (chunky metallic button)
    auto capRadius = 8.0f;

    // Cap shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillEllipse(centreX - capRadius + 1.0f, centreY - capRadius + 1.0f,
                  capRadius * 2.0f, capRadius * 2.0f);

    // Cap body gradient
    juce::ColourGradient capGradient(
        juce::Colour(0xff222222), centreX - capRadius, centreY - capRadius,
        juce::Colour(0xff000000), centreX + capRadius, centreY + capRadius, true);
    g.setGradientFill(capGradient);
    g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    // Cap highlight
    g.setColour(juce::Colour(0xff444444));
    g.fillEllipse(centreX - capRadius * 0.6f, centreY - capRadius * 0.6f,
                  capRadius * 0.8f, capRadius * 0.8f);
}

void MicroAcidLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                     float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                     juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto trackWidth = juce::jmin(8.0f, (float)width * 0.3f);

    if (slider.isHorizontal())
    {
        auto trackY = (float)y + (float)height * 0.5f - trackWidth * 0.5f;

        // Track shadow
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle((float)x + 1.0f, trackY + 1.0f, (float)width, trackWidth, trackWidth * 0.5f);

        // Background track (chrome/silver)
        juce::ColourGradient trackGradient(
            juce::Colour(0xff888888), (float)x, trackY,
            juce::Colour(0xffcccccc), (float)x, trackY + trackWidth, false);
        g.setGradientFill(trackGradient);
        g.fillRoundedRectangle((float)x, trackY, (float)width, trackWidth, trackWidth * 0.5f);

        // Filled portion (blue with glow)
        g.setColour(blueAccent.withAlpha(0.5f));
        g.fillRoundedRectangle((float)x, trackY, sliderPos - (float)x, trackWidth, trackWidth * 0.5f);

        // Thumb (chunky metallic fader cap)
        auto thumbWidth = 18.0f;
        auto thumbHeight = trackWidth + 12.0f;

        // Thumb shadow
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(sliderPos - thumbWidth * 0.5f + 1.0f, trackY - 6.0f + 1.0f,
                              thumbWidth, thumbHeight, 3.0f);

        // Thumb body
        juce::ColourGradient thumbGradient(
            juce::Colour(0xffd0d0d0), sliderPos, trackY - 6.0f,
            juce::Colour(0xff909090), sliderPos, trackY - 6.0f + thumbHeight, false);
        g.setGradientFill(thumbGradient);
        g.fillRoundedRectangle(sliderPos - thumbWidth * 0.5f, trackY - 6.0f,
                              thumbWidth, thumbHeight, 3.0f);

        // Thumb highlight
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillRoundedRectangle(sliderPos - thumbWidth * 0.5f + 2.0f, trackY - 4.0f,
                              thumbWidth - 4.0f, 4.0f, 2.0f);

        // Thumb outline
        g.setColour(orangeAccent);
        g.drawRoundedRectangle(sliderPos - thumbWidth * 0.5f, trackY - 6.0f,
                              thumbWidth, thumbHeight, 3.0f, 1.5f);
    }
    else
    {
        auto trackX = (float)x + (float)width * 0.5f - trackWidth * 0.5f;

        // Track shadow (inner)
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(trackX + 1.0f, (float)y + 1.0f, trackWidth, (float)height, trackWidth * 0.5f);

        // Background track (recessed chrome channel)
        juce::ColourGradient trackGradient(
            juce::Colour(0xff888888), trackX, (float)y,
            juce::Colour(0xffcccccc), trackX + trackWidth, (float)y, false);
        g.setGradientFill(trackGradient);
        g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, trackWidth * 0.5f);

        // Filled portion (from bottom, blue with LED glow)
        g.setColour(blueAccent.withAlpha(0.5f));
        g.fillRoundedRectangle(trackX, sliderPos, trackWidth,
                              (float)(y + height) - sliderPos, trackWidth * 0.5f);

        // Thumb (chunky vertical fader cap)
        auto thumbHeight = 20.0f;
        auto thumbWidth2 = trackWidth + 12.0f;

        // Thumb shadow
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(trackX - 6.0f + 1.0f, sliderPos - thumbHeight * 0.5f + 1.0f,
                              thumbWidth2, thumbHeight, 3.0f);

        // Thumb body gradient
        juce::ColourGradient thumbGradient(
            juce::Colour(0xffd0d0d0), trackX - 6.0f, sliderPos,
            juce::Colour(0xff909090), trackX - 6.0f + thumbWidth2, sliderPos, false);
        g.setGradientFill(thumbGradient);
        g.fillRoundedRectangle(trackX - 6.0f, sliderPos - thumbHeight * 0.5f,
                              thumbWidth2, thumbHeight, 3.0f);

        // Thumb highlight (top edge)
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.fillRoundedRectangle(trackX - 4.0f, sliderPos - thumbHeight * 0.5f + 2.0f,
                              thumbWidth2 - 4.0f, 4.0f, 2.0f);

        // Thumb outline (orange accent)
        g.setColour(orangeAccent);
        g.drawRoundedRectangle(trackX - 6.0f, sliderPos - thumbHeight * 0.5f,
                              thumbWidth2, thumbHeight, 3.0f, 1.5f);
    }
}

void MicroAcidLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                 int buttonX, int buttonY, int buttonWidth, int buttonHeight,
                                 juce::ComboBox& box)
{
    auto cornerSize = 4.0f;
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    // Shadow (recessed look)
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRoundedRectangle(boxBounds.toFloat().translated(1.0f, 1.0f), cornerSize);

    // Main body with metallic gradient
    juce::ColourGradient gradient(
        juce::Colour(0xff2a2a2a), 0, 0,
        juce::Colour(0xff3a3a3a), 0, (float)height, false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

    // Top highlight edge
    g.setColour(juce::Colour(0xff555555));
    g.drawLine(4.0f, 1.0f, (float)width - 4.0f, 1.0f, 1.0f);

    // Border with metallic edge
    g.setColour(lightGrey.withAlpha(0.6f));
    g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.5f);

    // Bottom edge shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawLine(4.0f, (float)height - 1.0f, (float)width - 4.0f, (float)height - 1.0f, 1.0f);

    // LED indicator on the left (shows active state)
    auto ledX = 8.0f;
    auto ledY = (float)height * 0.5f;
    auto ledRadius = 3.0f;

    // LED glow
    g.setColour(orangeAccent.withAlpha(0.3f));
    g.fillEllipse(ledX - ledRadius - 1.0f, ledY - ledRadius - 1.0f,
                  (ledRadius + 1.0f) * 2.0f, (ledRadius + 1.0f) * 2.0f);

    // LED body
    g.setColour(isButtonDown ? orangeAccent : orangeAccent.darker(0.6f));
    g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);

    // LED highlight
    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.fillEllipse(ledX - ledRadius * 0.5f, ledY - ledRadius * 0.5f,
                  ledRadius, ledRadius * 0.7f);

    // Arrow (vintage style chevron)
    juce::Path path;
    auto arrowX = (float)buttonX + (float)buttonWidth * 0.5f;
    auto arrowY = (float)buttonY + (float)buttonHeight * 0.5f;
    auto arrowSize = 5.0f;

    path.startNewSubPath(arrowX - arrowSize, arrowY - arrowSize * 0.4f);
    path.lineTo(arrowX, arrowY + arrowSize * 0.4f);
    path.lineTo(arrowX + arrowSize, arrowY - arrowSize * 0.4f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 1.0f : 0.3f));
    g.strokePath(path, juce::PathStrokeType(2.0f, juce::PathStrokeType::mitered));
}

void MicroAcidLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font(getLabelFont(label));

        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        auto textArea = label.getBorderSize().subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                        juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
                        label.getMinimumHorizontalScale());
    }
}

juce::Font MicroAcidLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::FontOptions("Helvetica", 13.0f, juce::Font::plain));
}

//==============================================================================
// MicroAcid303AudioProcessorEditor Implementation
//==============================================================================

MicroAcid303AudioProcessorEditor::MicroAcid303AudioProcessorEditor (MicroAcid303AudioProcessor& p)
    : AudioProcessorEditor (&p),
      m_audioProcessor (p),
      m_midiKeyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Set the custom look and feel
    setLookAndFeel(&m_microAcidLookAndFeel);

    // Set default editor size - taller to accommodate keyboard and visualizations
    setSize(920, 750);
    setResizable(true, true);
    setResizeLimits(800, 650, 1400, 1100);

    // Enable keyboard focus for QWERTY input
    setWantsKeyboardFocus(true);
    addKeyListener(this);

    //==============================================================================
    // OSCILLATOR SECTION

    // Add all 12 waveform items
    m_waveformSelector.addItem("Saw", 1);
    m_waveformSelector.addItem("Square", 2);
    m_waveformSelector.addItem("Triangle", 3);
    m_waveformSelector.addItem("Sine", 4);
    m_waveformSelector.addItem("Pulse 25%", 5);
    m_waveformSelector.addItem("Pulse 12%", 6);
    m_waveformSelector.addItem("SuperSaw", 7);
    m_waveformSelector.addItem("Noise", 8);
    m_waveformSelector.addItem("Saw+Sqr", 9);
    m_waveformSelector.addItem("Tri+Saw", 10);
    m_waveformSelector.addItem("Sync", 11);
    m_waveformSelector.addItem("FM", 12);
    addAndMakeVisible(m_waveformSelector);
    m_waveformSelector.setTooltip("Select waveform: 12 different waveforms including classic 303 Saw and Square");
    m_waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::WAVEFORM, m_waveformSelector);

    setupLabel(m_waveformLabel, "WAVEFORM");
    addAndMakeVisible(m_waveformLabel);

    setupRotarySlider(m_fineTuneSlider);
    m_fineTuneSlider.setTooltip("Fine tune oscillator pitch in cents");
    m_fineTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::FINE_TUNE, m_fineTuneSlider);

    setupLabel(m_fineTuneLabel, "TUNE");
    addAndMakeVisible(m_fineTuneLabel);

    m_fineTuneValueLabel.setJustificationType(juce::Justification::centred);
    m_fineTuneValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_fineTuneValueLabel);

    //==============================================================================
    // FILTER SECTION (Main controls - large knobs)

    setupRotarySlider(m_cutoffSlider);
    m_cutoffSlider.setTooltip("Filter cutoff frequency");
    m_cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::CUTOFF, m_cutoffSlider);

    setupLabel(m_cutoffLabel, "CUTOFF");
    addAndMakeVisible(m_cutoffLabel);

    m_cutoffValueLabel.setJustificationType(juce::Justification::centred);
    m_cutoffValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_cutoffValueLabel);

    setupRotarySlider(m_resonanceSlider);
    m_resonanceSlider.setTooltip("Filter resonance (self-oscillation)");
    m_resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::RESONANCE, m_resonanceSlider);

    setupLabel(m_resonanceLabel, "RESONANCE");
    addAndMakeVisible(m_resonanceLabel);

    m_resonanceValueLabel.setJustificationType(juce::Justification::centred);
    m_resonanceValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_resonanceValueLabel);

    setupRotarySlider(m_envModSlider);
    m_envModSlider.setTooltip("Envelope modulation amount to filter cutoff");
    m_envModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ENV_MOD, m_envModSlider);

    setupLabel(m_envModLabel, "ENV MOD");
    addAndMakeVisible(m_envModLabel);

    m_envModValueLabel.setJustificationType(juce::Justification::centred);
    m_envModValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_envModValueLabel);

    //==============================================================================
    // ENVELOPE SECTION

    setupRotarySlider(m_decaySlider);
    m_decaySlider.setTooltip("Envelope decay time");
    m_decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::DECAY, m_decaySlider);

    setupLabel(m_decayLabel, "DECAY");
    addAndMakeVisible(m_decayLabel);

    m_decayValueLabel.setJustificationType(juce::Justification::centred);
    m_decayValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_decayValueLabel);

    setupRotarySlider(m_accentSlider);
    m_accentSlider.setTooltip("Accent amount (increases filter cutoff and volume)");
    m_accentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ACCENT, m_accentSlider);

    setupLabel(m_accentLabel, "ACCENT");
    addAndMakeVisible(m_accentLabel);

    m_accentValueLabel.setJustificationType(juce::Justification::centred);
    m_accentValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_accentValueLabel);

    //==============================================================================
    // SLIDE SECTION

    setupRotarySlider(m_slideTimeSlider);
    m_slideTimeSlider.setTooltip("Portamento/slide time between notes");
    m_slideTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::SLIDE_TIME, m_slideTimeSlider);

    setupLabel(m_slideTimeLabel, "SLIDE");
    addAndMakeVisible(m_slideTimeLabel);

    m_slideTimeValueLabel.setJustificationType(juce::Justification::centred);
    m_slideTimeValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_slideTimeValueLabel);

    //==============================================================================
    // OVERDRIVE SECTION

    setupRotarySlider(m_driveSlider);
    m_driveSlider.setTooltip("Drive/distortion amount");
    m_driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::DRIVE, m_driveSlider);

    setupLabel(m_driveLabel, "DRIVE");
    addAndMakeVisible(m_driveLabel);

    m_driveValueLabel.setJustificationType(juce::Justification::centred);
    m_driveValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_driveValueLabel);

    // Add all 5 overdrive modes
    m_driveModeSelector.addItem("Soft", 1);
    m_driveModeSelector.addItem("Classic", 2);
    m_driveModeSelector.addItem("Saturated", 3);
    m_driveModeSelector.addItem("Fuzz", 4);
    m_driveModeSelector.addItem("Tape", 5);
    addAndMakeVisible(m_driveModeSelector);
    m_driveModeSelector.setTooltip("Drive mode: Soft (tube), Classic (303), Saturated, Fuzz, Tape");
    m_driveModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::DRIVE_MODE, m_driveModeSelector);

    setupLabel(m_driveModeLabel, "MODE");
    addAndMakeVisible(m_driveModeLabel);

    //==============================================================================
    // EFFECTS SECTION

    // Add all 8 effect types
    m_fxTypeSelector.addItem("Tape Dly", 1);
    m_fxTypeSelector.addItem("Digi Dly", 2);
    m_fxTypeSelector.addItem("PingPong", 3);
    m_fxTypeSelector.addItem("Reverb", 4);
    m_fxTypeSelector.addItem("Chorus", 5);
    m_fxTypeSelector.addItem("Flanger", 6);
    m_fxTypeSelector.addItem("Phaser", 7);
    m_fxTypeSelector.addItem("Bitcrush", 8);
    addAndMakeVisible(m_fxTypeSelector);
    m_fxTypeSelector.setTooltip("Effects: Tape Delay, Digital Delay, Ping Pong, Reverb, Chorus, Flanger, Phaser, Bitcrush");
    m_fxTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::FX_TYPE, m_fxTypeSelector);

    setupLabel(m_fxTypeLabel, "FX TYPE");
    addAndMakeVisible(m_fxTypeLabel);

    setupRotarySlider(m_fxTimeSlider);
    m_fxTimeSlider.setTooltip("Delay time or reverb size");
    m_fxTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::FX_TIME, m_fxTimeSlider);

    setupLabel(m_fxTimeLabel, "TIME");
    addAndMakeVisible(m_fxTimeLabel);

    m_fxTimeValueLabel.setJustificationType(juce::Justification::centred);
    m_fxTimeValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_fxTimeValueLabel);

    setupRotarySlider(m_fxFeedbackSlider);
    m_fxFeedbackSlider.setTooltip("Effect feedback amount");
    m_fxFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::FX_FEEDBACK, m_fxFeedbackSlider);

    setupLabel(m_fxFeedbackLabel, "FEEDBACK");
    addAndMakeVisible(m_fxFeedbackLabel);

    m_fxFeedbackValueLabel.setJustificationType(juce::Justification::centred);
    m_fxFeedbackValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_fxFeedbackValueLabel);

    setupRotarySlider(m_fxMixSlider);
    m_fxMixSlider.setTooltip("Dry/wet mix of effect");
    m_fxMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::FX_MIX, m_fxMixSlider);

    setupLabel(m_fxMixLabel, "MIX");
    addAndMakeVisible(m_fxMixLabel);

    m_fxMixValueLabel.setJustificationType(juce::Justification::centred);
    m_fxMixValueLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(m_fxMixValueLabel);

    //==============================================================================
    // ARPEGGIATOR SECTION

    m_arpEnabledButton.setButtonText("ARP ON");
    m_arpEnabledButton.setClickingTogglesState(true);
    addAndMakeVisible(m_arpEnabledButton);
    m_arpEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ARP_ENABLED, m_arpEnabledButton);

    // Add all 7 arpeggiator modes
    m_arpModeSelector.addItem("Up", 1);
    m_arpModeSelector.addItem("Down", 2);
    m_arpModeSelector.addItem("Up/Down", 3);
    m_arpModeSelector.addItem("Down/Up", 4);
    m_arpModeSelector.addItem("Random", 5);
    m_arpModeSelector.addItem("Order", 6);
    m_arpModeSelector.addItem("Chord", 7);
    addAndMakeVisible(m_arpModeSelector);
    m_arpModeSelector.setTooltip("Arpeggiator mode: Up, Down, Up/Down, Down/Up, Random, Order, Chord");
    m_arpModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ARP_MODE, m_arpModeSelector);

    setupLabel(m_arpModeLabel, "MODE");
    addAndMakeVisible(m_arpModeLabel);

    // Add all 10 note divisions
    m_arpDivisionSelector.addItem("1/1", 1);
    m_arpDivisionSelector.addItem("1/2", 2);
    m_arpDivisionSelector.addItem("1/4", 3);
    m_arpDivisionSelector.addItem("1/8", 4);
    m_arpDivisionSelector.addItem("1/16", 5);
    m_arpDivisionSelector.addItem("1/32", 6);
    m_arpDivisionSelector.addItem("1/4D", 7);
    m_arpDivisionSelector.addItem("1/8D", 8);
    m_arpDivisionSelector.addItem("1/4T", 9);
    m_arpDivisionSelector.addItem("1/8T", 10);
    addAndMakeVisible(m_arpDivisionSelector);
    m_arpDivisionSelector.setTooltip("Note division: whole, half, quarter, eighth, sixteenth, dotted, triplet");
    m_arpDivisionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ARP_DIVISION, m_arpDivisionSelector);

    setupLabel(m_arpDivisionLabel, "RATE");
    addAndMakeVisible(m_arpDivisionLabel);

    setupRotarySlider(m_arpGateSlider);
    m_arpGateSlider.setTooltip("Gate length (note duration)");
    m_arpGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ARP_GATE, m_arpGateSlider);

    setupLabel(m_arpGateLabel, "GATE");
    addAndMakeVisible(m_arpGateLabel);

    m_arpGateValueLabel.setJustificationType(juce::Justification::centred);
    m_arpGateValueLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(m_arpGateValueLabel);

    setupRotarySlider(m_arpOctavesSlider);
    m_arpOctavesSlider.setTooltip("Octave range (1-4)");
    m_arpOctavesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ARP_OCTAVES, m_arpOctavesSlider);

    setupLabel(m_arpOctavesLabel, "OCT");
    addAndMakeVisible(m_arpOctavesLabel);

    m_arpOctavesValueLabel.setJustificationType(juce::Justification::centred);
    m_arpOctavesValueLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(m_arpOctavesValueLabel);

    setupRotarySlider(m_arpSwingSlider);
    m_arpSwingSlider.setTooltip("Swing amount");
    m_arpSwingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::ARP_SWING, m_arpSwingSlider);

    setupLabel(m_arpSwingLabel, "SWING");
    addAndMakeVisible(m_arpSwingLabel);

    m_arpSwingValueLabel.setJustificationType(juce::Justification::centred);
    m_arpSwingValueLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(m_arpSwingValueLabel);

    //==============================================================================
    // OUTPUT SECTION

    setupLinearSlider(m_outputGainSlider);
    m_outputGainSlider.setTooltip("Output gain control");
    m_outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        m_audioProcessor.getValueTreeState(), MicroAcidParameters::IDs::OUTPUT_GAIN, m_outputGainSlider);

    setupLabel(m_outputGainLabel, "OUTPUT");
    addAndMakeVisible(m_outputGainLabel);

    m_outputGainValueLabel.setJustificationType(juce::Justification::centred);
    m_outputGainValueLabel.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(m_outputGainValueLabel);

    //==============================================================================
    // MIDI KEYBOARD SECTION (v1.1)
    m_midiKeyboard.setName("Keyboard");
    m_midiKeyboard.setAvailableRange(36, 84);  // C2 to C6 (4 octaves)
    m_midiKeyboard.setOctaveForMiddleC(4);
    m_midiKeyboard.setKeyWidth(20.0f);
    m_midiKeyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour(0xffeeeedd));
    m_midiKeyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xff2a2a2a));
    m_midiKeyboard.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour(0xff3a3a3a));
    m_midiKeyboard.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour(0x40ff6600));
    m_midiKeyboard.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour(0x80ff6600));
    addAndMakeVisible(m_midiKeyboard);

    setupLabel(m_keyboardLabel, "KEYBOARD (Use QWERTY: Z-M = C3-B3, Q-P = C4-B4)");
    addAndMakeVisible(m_keyboardLabel);

    // Start timer for updating value labels and visualizations
    startTimerHz(30);
}

MicroAcid303AudioProcessorEditor::~MicroAcid303AudioProcessorEditor()
{
    removeKeyListener(this);
    setLookAndFeel(nullptr);
}

//==============================================================================
void MicroAcid303AudioProcessorEditor::paint (juce::Graphics& g)
{
    // ========== BRUSHED METAL BACKGROUND ==========
    // Base metal color
    g.setColour(juce::Colour(0xffc0c0c0));
    g.fillAll();

    // Horizontal brushed metal texture (authentic 80s hardware)
    for (int i = 0; i < getHeight(); i += 1)
    {
        float brightness = 0.95f + (std::sin((float)i * 0.3f) * 0.05f);
        g.setColour(juce::Colour(0xffc0c0c0).withBrightness(brightness));
        g.drawLine(0.0f, (float)i, (float)getWidth(), (float)i, 1.0f);
    }

    // Subtle darker edge vignette
    juce::ColourGradient edgeGradient(
        juce::Colours::black.withAlpha(0.15f), (float)getWidth() * 0.5f, (float)getHeight() * 0.5f,
        juce::Colours::transparentBlack, 0, 0, true);
    edgeGradient.addColour(0.7, juce::Colours::transparentBlack);
    g.setGradientFill(edgeGradient);
    g.fillAll();

    // ========== VINTAGE HEADER BAR ==========
    auto titleBounds = getLocalBounds().removeFromTop(60);

    // Header background (darker gunmetal panel)
    juce::ColourGradient headerGradient(
        juce::Colour(0xff3a3a3a), 0, (float)titleBounds.getY(),
        juce::Colour(0xff2a2a2a), 0, (float)titleBounds.getBottom(),
        false
    );
    g.setGradientFill(headerGradient);
    g.fillRect(titleBounds);

    // Header top edge highlight
    g.setColour(juce::Colour(0xff555555));
    g.drawLine(0, (float)titleBounds.getY(), (float)getWidth(), (float)titleBounds.getY(), 1.0f);

    // Header bottom edge (metallic separator)
    g.setColour(juce::Colour(0xff909090));
    g.drawLine(0, (float)titleBounds.getBottom() - 1.0f, (float)getWidth(), (float)titleBounds.getBottom() - 1.0f, 2.0f);

    g.setColour(juce::Colour(0xff1a1a1a));
    g.drawLine(0, (float)titleBounds.getBottom(), (float)getWidth(), (float)titleBounds.getBottom(), 1.0f);

    // POWER LED INDICATOR (authentic vintage LED)
    auto ledX = 25.0f;
    auto ledY = (float)titleBounds.getCentreY();
    auto ledRadius = 5.0f;

    // LED bezel (recessed)
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillEllipse(ledX - ledRadius - 2.0f, ledY - ledRadius - 2.0f, (ledRadius + 2.0f) * 2.0f, (ledRadius + 2.0f) * 2.0f);

    // LED glow
    g.setColour(juce::Colour(0xffffaa00).withAlpha(0.5f));
    g.fillEllipse(ledX - ledRadius - 3.0f, ledY - ledRadius - 3.0f, (ledRadius + 3.0f) * 2.0f, (ledRadius + 3.0f) * 2.0f);

    // LED body
    g.setColour(juce::Colour(0xffffaa00));
    g.fillEllipse(ledX - ledRadius, ledY - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);

    // LED highlight (makes it look 3D)
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.fillEllipse(ledX - ledRadius * 0.5f, ledY - ledRadius * 0.7f, ledRadius, ledRadius * 0.8f);

    // POWER label
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(juce::FontOptions("Helvetica", 9.0f, juce::Font::bold)));
    g.drawText("POWER", (int)ledX - 10, (int)ledY + 10, 30, 12, juce::Justification::centred);

    // Main title text with depth
    auto titleTextBounds = titleBounds.reduced(60, 0);

    // Title shadow
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.setFont(juce::Font(juce::FontOptions("Helvetica", 36.0f, juce::Font::bold)));
    g.drawText("303 MICRO ACID", titleTextBounds.translated(2, 2), juce::Justification::centredLeft);

    // Title main (bright orange - iconic 303 MICRO ACID color!)
    g.setColour(juce::Colour(0xffff6600));
    g.setFont(juce::Font(juce::FontOptions("Helvetica", 36.0f, juce::Font::bold)));
    g.drawText("303 MICRO ACID", titleTextBounds, juce::Justification::centredLeft);

    // Subtitle
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::Font(juce::FontOptions("Helvetica", 12.0f, juce::Font::plain)));
    g.drawText("BASSLINE SYNTHESIZER", titleTextBounds.translated(0, 20), juce::Justification::centredLeft);

    // Vintage "Roland-style" decoration line
    auto decorX = (float)getWidth() - 100.0f;
    g.setColour(juce::Colour(0xffff6600).withAlpha(0.5f));
    g.drawLine(decorX, (float)titleBounds.getCentreY() - 15.0f, decorX, (float)titleBounds.getCentreY() + 15.0f, 3.0f);

    // ========== PANEL SECTIONS ==========
    auto bounds = getLocalBounds().withTrimmedTop(60).reduced(10);

    // Top row: OSCILLATOR, FILTER, ENVELOPE
    auto topRow = bounds.removeFromTop(bounds.getHeight() / 3 - 4);

    auto oscSection = topRow.removeFromLeft(topRow.getWidth() / 3 - 6);
    drawSection(g, oscSection, "OSCILLATOR");

    topRow.removeFromLeft(8);
    auto filterSection = topRow.removeFromLeft(topRow.getWidth() / 2 - 4);
    drawSection(g, filterSection, "FILTER");

    topRow.removeFromLeft(8);
    auto envSection = topRow;
    drawSection(g, envSection, "ENVELOPE");

    bounds.removeFromTop(8);

    // Middle row: ARPEGGIATOR (full width)
    auto middleRow = bounds.removeFromTop(bounds.getHeight() / 2 - 4);
    drawSection(g, middleRow, "ARPEGGIATOR");

    bounds.removeFromTop(8);

    // Bottom row: OVERDRIVE, EFFECTS, OUTPUT
    auto bottomRow = bounds;

    auto driveSection = bottomRow.removeFromLeft(bottomRow.getWidth() / 4 - 6);
    drawSection(g, driveSection, "OVERDRIVE");

    bottomRow.removeFromLeft(8);
    auto fxSection = bottomRow.removeFromLeft(bottomRow.getWidth() * 2 / 3 - 4);
    drawSection(g, fxSection, "EFFECTS");

    bottomRow.removeFromLeft(8);
    auto outputSection = bottomRow;
    drawSection(g, outputSection, "OUTPUT");

    //==============================================================================
    // VISUALIZATION & KEYBOARD SECTIONS (v1.1)
    // These are drawn after resized() calculates the bounds
    // The actual drawing happens via the visualization methods called in paint()

    // Calculate visualization area (below bottom row, above keyboard)
    auto fullBounds = getLocalBounds().withTrimmedTop(60).reduced(10);
    int usedHeight = (fullBounds.getHeight() / 3 - 4) + 8 +  // top row
                     (fullBounds.getHeight() / 3 - 4) + 8 +  // middle row (arp)
                     (fullBounds.getHeight() / 3);            // bottom row

    auto lowerArea = getLocalBounds().withTrimmedTop(60 + usedHeight + 20).reduced(10);

    // Draw visualization panel background
    auto vizArea = lowerArea.removeFromTop(80);
    drawSection(g, vizArea, "ANALYZER");

    // Draw oscilloscope
    auto oscBounds = vizArea.reduced(12, 30);
    auto scopeArea = oscBounds.removeFromLeft(oscBounds.getWidth() / 2 - 20);
    drawOscilloscope(g, scopeArea);

    // Draw level meters
    auto metersArea = oscBounds.removeFromLeft(40);
    metersArea.reduce(4, 4);
    auto meterL = metersArea.removeFromLeft(14);
    metersArea.removeFromLeft(4);
    auto meterR = metersArea.removeFromLeft(14);
    drawLevelMeter(g, meterL, m_displayPeakL, true);
    drawLevelMeter(g, meterR, m_displayPeakR, false);

    // Draw filter curve
    oscBounds.removeFromLeft(10);
    drawFilterCurve(g, oscBounds);

    // Draw keyboard section background
    lowerArea.removeFromTop(8);
    auto keyboardArea = lowerArea;
    drawSection(g, keyboardArea, "KEYBOARD (QWERTY: Z-M & Q-P)");
}

void MicroAcid303AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().withTrimmedTop(60).reduced(10);

    //==============================================================================
    // TOP ROW LAYOUT (OSCILLATOR, FILTER, ENVELOPE)
    auto topRow = bounds.removeFromTop(bounds.getHeight() / 3 - 4);

    // OSCILLATOR SECTION
    auto oscSection = topRow.removeFromLeft(topRow.getWidth() / 3 - 6).reduced(12, 32);

    auto oscRow1 = oscSection.removeFromTop(26);
    m_waveformLabel.setBounds(oscRow1.removeFromLeft(70));
    m_waveformSelector.setBounds(oscRow1);

    oscSection.removeFromTop(6);
    auto tuneKnobArea = oscSection.removeFromTop(70);
    m_fineTuneSlider.setBounds(tuneKnobArea);

    m_fineTuneLabel.setBounds(oscSection.removeFromTop(16));
    m_fineTuneValueLabel.setBounds(oscSection.removeFromTop(14));

    topRow.removeFromLeft(8);

    // FILTER SECTION (smaller knobs)
    auto filterSection = topRow.removeFromLeft(topRow.getWidth() / 2 - 4).reduced(12, 28);

    auto filterKnobSize = 55;
    auto filterKnobWidth = filterSection.getWidth() / 3;

    auto cutoffArea = filterSection.removeFromLeft(filterKnobWidth);
    m_cutoffSlider.setBounds(cutoffArea.removeFromTop(filterKnobSize));
    m_cutoffLabel.setBounds(cutoffArea.removeFromTop(16));
    m_cutoffValueLabel.setBounds(cutoffArea.removeFromTop(14));

    auto resArea = filterSection.removeFromLeft(filterKnobWidth);
    m_resonanceSlider.setBounds(resArea.removeFromTop(filterKnobSize));
    m_resonanceLabel.setBounds(resArea.removeFromTop(16));
    m_resonanceValueLabel.setBounds(resArea.removeFromTop(14));

    auto envModArea = filterSection;
    m_envModSlider.setBounds(envModArea.removeFromTop(filterKnobSize));
    m_envModLabel.setBounds(envModArea.removeFromTop(16));
    m_envModValueLabel.setBounds(envModArea.removeFromTop(14));

    topRow.removeFromLeft(8);

    // ENVELOPE SECTION (smaller knobs with proper space for labels)
    auto envSection = topRow.reduced(12, 28);

    auto envKnobSize = 55;
    auto envKnobWidth = envSection.getWidth() / 3;

    auto decayArea = envSection.removeFromLeft(envKnobWidth);
    m_decaySlider.setBounds(decayArea.removeFromTop(envKnobSize));
    m_decayLabel.setBounds(decayArea.removeFromTop(16));
    m_decayValueLabel.setBounds(decayArea.removeFromTop(14));

    auto accentArea = envSection.removeFromLeft(envKnobWidth);
    m_accentSlider.setBounds(accentArea.removeFromTop(envKnobSize));
    m_accentLabel.setBounds(accentArea.removeFromTop(16));
    m_accentValueLabel.setBounds(accentArea.removeFromTop(14));

    auto slideArea = envSection;
    m_slideTimeSlider.setBounds(slideArea.removeFromTop(envKnobSize));
    m_slideTimeLabel.setBounds(slideArea.removeFromTop(16));
    m_slideTimeValueLabel.setBounds(slideArea.removeFromTop(14));

    bounds.removeFromTop(8);

    //==============================================================================
    // MIDDLE ROW LAYOUT (ARPEGGIATOR)
    auto middleRow = bounds.removeFromTop(bounds.getHeight() / 2 - 4).reduced(12, 32);

    // Arp enable button
    auto arpEnableArea = middleRow.removeFromLeft(80);
    m_arpEnabledButton.setBounds(arpEnableArea.reduced(4, 20));

    middleRow.removeFromLeft(10);

    // Mode selector
    auto modeArea = middleRow.removeFromLeft(100);
    m_arpModeLabel.setBounds(modeArea.removeFromTop(16));
    m_arpModeSelector.setBounds(modeArea.removeFromTop(26));

    middleRow.removeFromLeft(10);

    // Division selector
    auto divArea = middleRow.removeFromLeft(100);
    m_arpDivisionLabel.setBounds(divArea.removeFromTop(16));
    m_arpDivisionSelector.setBounds(divArea.removeFromTop(26));

    middleRow.removeFromLeft(20);

    // Gate, Octaves, Swing knobs
    auto arpKnobSize = 60;
    auto arpKnobRow = middleRow;

    auto gateArea = arpKnobRow.removeFromLeft(arpKnobRow.getWidth() / 3);
    m_arpGateSlider.setBounds(gateArea.removeFromTop(arpKnobSize));
    m_arpGateLabel.setBounds(gateArea.removeFromTop(14));
    m_arpGateValueLabel.setBounds(gateArea.removeFromTop(12));

    auto octArea = arpKnobRow.removeFromLeft(arpKnobRow.getWidth() / 2);
    m_arpOctavesSlider.setBounds(octArea.removeFromTop(arpKnobSize));
    m_arpOctavesLabel.setBounds(octArea.removeFromTop(14));
    m_arpOctavesValueLabel.setBounds(octArea.removeFromTop(12));

    m_arpSwingSlider.setBounds(arpKnobRow.removeFromTop(arpKnobSize));
    m_arpSwingLabel.setBounds(arpKnobRow.removeFromTop(14));
    m_arpSwingValueLabel.setBounds(arpKnobRow.removeFromTop(12));

    bounds.removeFromTop(8);

    //==============================================================================
    // BOTTOM ROW LAYOUT (OVERDRIVE, EFFECTS, OUTPUT)
    auto bottomRow = bounds;

    // OVERDRIVE SECTION
    auto driveSection = bottomRow.removeFromLeft(bottomRow.getWidth() / 4 - 6).reduced(12, 32);

    auto driveKnobArea = driveSection.removeFromTop(65);
    m_driveSlider.setBounds(driveKnobArea);
    m_driveLabel.setBounds(driveSection.removeFromTop(14));
    m_driveValueLabel.setBounds(driveSection.removeFromTop(12));

    driveSection.removeFromTop(6);
    auto driveModeRow = driveSection.removeFromTop(26);
    m_driveModeLabel.setBounds(driveModeRow.removeFromLeft(45));
    m_driveModeSelector.setBounds(driveModeRow);

    bottomRow.removeFromLeft(8);

    // EFFECTS SECTION (proper layout with space for labels)
    auto fxSection = bottomRow.removeFromLeft(bottomRow.getWidth() * 2 / 3 - 4).reduced(12, 28);

    auto fxTypeRow = fxSection.removeFromTop(26);
    m_fxTypeLabel.setBounds(fxTypeRow.removeFromLeft(65));
    m_fxTypeSelector.setBounds(fxTypeRow);

    fxSection.removeFromTop(6);

    auto fxKnobSize = 50;
    auto fxKnobWidth = fxSection.getWidth() / 3;

    auto timeArea = fxSection.removeFromLeft(fxKnobWidth);
    m_fxTimeSlider.setBounds(timeArea.removeFromTop(fxKnobSize));
    m_fxTimeLabel.setBounds(timeArea.removeFromTop(14));
    m_fxTimeValueLabel.setBounds(timeArea.removeFromTop(12));

    auto fbArea = fxSection.removeFromLeft(fxKnobWidth);
    m_fxFeedbackSlider.setBounds(fbArea.removeFromTop(fxKnobSize));
    m_fxFeedbackLabel.setBounds(fbArea.removeFromTop(14));
    m_fxFeedbackValueLabel.setBounds(fbArea.removeFromTop(12));

    auto mixArea = fxSection;
    m_fxMixSlider.setBounds(mixArea.removeFromTop(fxKnobSize));
    m_fxMixLabel.setBounds(mixArea.removeFromTop(14));
    m_fxMixValueLabel.setBounds(mixArea.removeFromTop(12));

    bottomRow.removeFromLeft(8);

    // OUTPUT SECTION
    auto outputSection = bottomRow.reduced(12, 32);

    m_outputGainSlider.setBounds(outputSection.removeFromTop(100));
    outputSection.removeFromTop(4);
    m_outputGainLabel.setBounds(outputSection.removeFromTop(16));
    m_outputGainValueLabel.setBounds(outputSection.removeFromTop(14));

    //==============================================================================
    // KEYBOARD SECTION (v1.1) - positioned at bottom of window
    auto fullBounds = getLocalBounds().withTrimmedTop(60).reduced(10);

    // Calculate space used by synth controls
    int synthControlsHeight = (fullBounds.getHeight() / 3 - 4) * 3 + 16; // 3 rows + spacing

    // Keyboard goes at the very bottom
    auto keyboardBounds = getLocalBounds().reduced(10);
    keyboardBounds = keyboardBounds.removeFromBottom(70).reduced(12, 8);
    m_midiKeyboard.setBounds(keyboardBounds);

    // Keyboard label above it
    auto labelBounds = getLocalBounds().reduced(10);
    labelBounds = labelBounds.removeFromBottom(90);
    m_keyboardLabel.setBounds(labelBounds.removeFromTop(16).reduced(10, 0));
}

void MicroAcid303AudioProcessorEditor::timerCallback()
{
    // Update value labels
    auto& params = m_audioProcessor.getValueTreeState();

    m_fineTuneValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::FINE_TUNE)->load(), 1) + " ct",
        juce::dontSendNotification);

    m_cutoffValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::CUTOFF)->load(), 0) + " Hz",
        juce::dontSendNotification);

    m_resonanceValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::RESONANCE)->load() * 100)) + " %",
        juce::dontSendNotification);

    m_envModValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::ENV_MOD)->load() * 100)) + " %",
        juce::dontSendNotification);

    m_decayValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::DECAY)->load(), 2) + " s",
        juce::dontSendNotification);

    m_accentValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::ACCENT)->load() * 100)) + " %",
        juce::dontSendNotification);

    m_slideTimeValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::SLIDE_TIME)->load(), 2) + " s",
        juce::dontSendNotification);

    m_driveValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::DRIVE)->load(), 1),
        juce::dontSendNotification);

    m_fxTimeValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::FX_TIME)->load(), 0) + " ms",
        juce::dontSendNotification);

    m_fxFeedbackValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::FX_FEEDBACK)->load() * 100)) + " %",
        juce::dontSendNotification);

    m_fxMixValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::FX_MIX)->load() * 100)) + " %",
        juce::dontSendNotification);

    m_outputGainValueLabel.setText(
        juce::String(params.getRawParameterValue(MicroAcidParameters::IDs::OUTPUT_GAIN)->load(), 1) + " dB",
        juce::dontSendNotification);

    // Arpeggiator values
    m_arpGateValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::ARP_GATE)->load() * 100)) + " %",
        juce::dontSendNotification);

    m_arpOctavesValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::ARP_OCTAVES)->load())),
        juce::dontSendNotification);

    m_arpSwingValueLabel.setText(
        juce::String(int(params.getRawParameterValue(MicroAcidParameters::IDs::ARP_SWING)->load() * 100)) + " %",
        juce::dontSendNotification);

    //==============================================================================
    // UPDATE VISUALIZATION DATA (v1.1)
    m_displayPeakL = m_audioProcessor.getOutputPeakL();
    m_displayPeakR = m_audioProcessor.getOutputPeakR();

    // Copy waveform data for oscilloscope
    const auto& waveformBuffer = m_audioProcessor.getWaveformBuffer();
    int writeIdx = m_audioProcessor.getWaveformWriteIndex();
    for (int i = 0; i < 512; ++i)
    {
        m_oscilloscopeData[i] = waveformBuffer[(writeIdx + i) % 512];
    }

    // Trigger repaint for visualizations
    repaint();
}

//==============================================================================
// Helper Methods
//==============================================================================

void MicroAcid303AudioProcessorEditor::setupRotarySlider(juce::Slider& slider, const juce::String& /*suffix*/)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                               juce::MathConstants<float>::pi * 2.8f,
                               true);
    slider.setVelocityBasedMode(true);
    slider.setVelocityModeParameters(0.8, 1, 0.09, false);
    slider.setDoubleClickReturnValue(true, slider.getMinimum() + (slider.getMaximum() - slider.getMinimum()) / 2.0);
    addAndMakeVisible(slider);
}

void MicroAcid303AudioProcessorEditor::setupLinearSlider(juce::Slider& slider, const juce::String& /*suffix*/)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setVelocityBasedMode(true);
    slider.setVelocityModeParameters(0.8, 1, 0.09, false);
    addAndMakeVisible(slider);
}

void MicroAcid303AudioProcessorEditor::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions("Helvetica", 12.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
}

void MicroAcid303AudioProcessorEditor::drawSection(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
{
    auto cornerSize = 8.0f;

    // ========== RECESSED PANEL (authentic hardware look) ==========

    // Outer shadow (makes panel look recessed into chassis)
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRoundedRectangle(bounds.toFloat().translated(2.0f, 2.0f), cornerSize);

    // Inner shadow (top-left)
    g.setColour(juce::Colour(0xff000000).withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(1.0f, 1.0f), cornerSize, 2.0f);

    // Panel background (dark recessed area)
    juce::ColourGradient panelGradient(
        juce::Colour(0xff2a2a2a), (float)bounds.getX(), (float)bounds.getY(),
        juce::Colour(0xff1a1a1a), (float)bounds.getX(), (float)bounds.getBottom(),
        false
    );
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

    // Panel edge highlight (bottom-right - creates 3D beveled effect)
    g.setColour(juce::Colour(0xff555555).withAlpha(0.5f));
    juce::Path bottomRightHighlight;
    auto reducedBounds = bounds.toFloat().reduced(1.0f);
    bottomRightHighlight.addRoundedRectangle(reducedBounds, cornerSize);
    g.strokePath(bottomRightHighlight, juce::PathStrokeType(1.0f));

    // Subtle texture lines on panel (like brushed plastic/metal)
    g.setColour(juce::Colours::white.withAlpha(0.02f));
    for (int i = bounds.getY() + 5; i < bounds.getBottom() - 5; i += 3)
    {
        g.drawLine((float)bounds.getX() + 5.0f, (float)i, (float)bounds.getRight() - 5.0f, (float)i, 1.0f);
    }

    // ========== SECTION TITLE (engraved look) ==========
    auto titleBounds = bounds.removeFromTop(30);

    // Title background bar (slightly raised)
    g.setColour(juce::Colour(0xff333333));
    g.fillRoundedRectangle((float)titleBounds.getX() + 8.0f, (float)titleBounds.getY() + 8.0f,
                          (float)titleBounds.getWidth() - 16.0f, (float)titleBounds.getHeight() - 8.0f, 4.0f);

    // Title text shadow (engraved effect)
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.setFont(juce::Font(juce::FontOptions("Helvetica", 14.0f, juce::Font::bold)));
    g.drawText(title, titleBounds.translated(0, 1), juce::Justification::centred);

    // Title text main (bright orange)
    g.setColour(juce::Colour(0xffff6600));
    g.setFont(juce::Font(juce::FontOptions("Helvetica", 14.0f, juce::Font::bold)));
    g.drawText(title, titleBounds, juce::Justification::centred);

    // Decorative screws in corners (authentic hardware detail!)
    auto drawScrew = [&g](float x, float y)
    {
        auto screwRadius = 3.0f;
        // Screw shadow
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(x - screwRadius + 0.5f, y - screwRadius + 0.5f, screwRadius * 2.0f, screwRadius * 2.0f);

        // Screw body
        juce::ColourGradient screwGradient(
            juce::Colour(0xff999999), x - screwRadius, y - screwRadius,
            juce::Colour(0xff555555), x + screwRadius, y + screwRadius, true);
        g.setGradientFill(screwGradient);
        g.fillEllipse(x - screwRadius, y - screwRadius, screwRadius * 2.0f, screwRadius * 2.0f);

        // Screw slot
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.drawLine(x - screwRadius + 1.0f, y, x + screwRadius - 1.0f, y, 1.0f);
    };

    auto screwInset = 8.0f;
    drawScrew((float)bounds.getX() + screwInset, (float)bounds.getY() + screwInset);
    drawScrew((float)bounds.getRight() - screwInset, (float)bounds.getY() + screwInset);
    drawScrew((float)bounds.getX() + screwInset, (float)bounds.getBottom() - screwInset);
    drawScrew((float)bounds.getRight() - screwInset, (float)bounds.getBottom() - screwInset);
}

//==============================================================================
// VISUALIZATION DRAWING METHODS (v1.1)
//==============================================================================

void MicroAcid303AudioProcessorEditor::drawOscilloscope(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Draw background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Draw border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);

    // Draw grid lines
    g.setColour(juce::Colour(0xff2a2a2a));
    int gridLines = 4;
    for (int i = 1; i < gridLines; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() * i / gridLines);
        g.drawLine((float)bounds.getX() + 2, y, (float)bounds.getRight() - 2, y, 1.0f);
    }

    // Draw center line
    g.setColour(juce::Colour(0xff3a3a3a));
    float centerY = bounds.getCentreY();
    g.drawLine((float)bounds.getX() + 2, centerY, (float)bounds.getRight() - 2, centerY, 1.0f);

    // Draw waveform
    juce::Path waveformPath;
    float w = (float)bounds.getWidth() - 4;
    float h = (float)bounds.getHeight() - 4;
    float xStart = (float)bounds.getX() + 2;
    float yCenter = (float)bounds.getCentreY();

    bool firstPoint = true;
    for (int i = 0; i < 512; ++i)
    {
        float x = xStart + (w * i / 512.0f);
        float sample = m_oscilloscopeData[i];
        float y = yCenter - (sample * h * 0.45f);

        if (firstPoint)
        {
            waveformPath.startNewSubPath(x, y);
            firstPoint = false;
        }
        else
        {
            waveformPath.lineTo(x, y);
        }
    }

    // Glow effect
    g.setColour(juce::Colour(0xff00aaff).withAlpha(0.3f));
    g.strokePath(waveformPath, juce::PathStrokeType(3.0f));

    // Main waveform line
    g.setColour(juce::Colour(0xff00aaff));
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}

void MicroAcid303AudioProcessorEditor::drawLevelMeter(juce::Graphics& g, juce::Rectangle<int> bounds, float level, bool isLeft)
{
    juce::ignoreUnused(isLeft);

    // Draw background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(bounds.toFloat(), 2.0f);

    // Draw border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRoundedRectangle(bounds.toFloat(), 2.0f, 1.0f);

    // Calculate meter height
    float meterLevel = juce::jlimit(0.0f, 1.0f, level);
    int meterHeight = (int)(bounds.getHeight() * meterLevel);
    auto meterBounds = bounds.removeFromBottom(meterHeight).reduced(2, 2);

    // Draw meter fill with gradient (green to yellow to red)
    if (meterHeight > 0)
    {
        juce::ColourGradient gradient(
            juce::Colour(0xff00ff00), 0, (float)bounds.getBottom(),
            juce::Colour(0xffff0000), 0, (float)bounds.getY(), false);
        gradient.addColour(0.7, juce::Colour(0xffffff00));
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(meterBounds.toFloat(), 1.0f);
    }

    // Peak indicator
    if (level > 0.9f)
    {
        g.setColour(juce::Colour(0xffff0000));
        g.fillRoundedRectangle(bounds.removeFromTop(4).reduced(2, 0).toFloat(), 1.0f);
    }
}

void MicroAcid303AudioProcessorEditor::drawFilterCurve(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Draw background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Draw border
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);

    // Get filter parameters
    float cutoff = m_audioProcessor.getFilterCutoff();
    float resonance = m_audioProcessor.getFilterResonance();

    // Draw filter response curve
    juce::Path filterPath;
    float w = (float)bounds.getWidth() - 4;
    float h = (float)bounds.getHeight() - 4;
    float xStart = (float)bounds.getX() + 2;
    float yBottom = (float)bounds.getBottom() - 2;

    // Normalize cutoff to 0-1 range (20-4000 Hz, logarithmic)
    float normalizedCutoff = std::log10(cutoff / 20.0f) / std::log10(4000.0f / 20.0f);
    normalizedCutoff = juce::jlimit(0.0f, 1.0f, normalizedCutoff);

    filterPath.startNewSubPath(xStart, yBottom - h * 0.8f);

    for (int i = 0; i <= 100; ++i)
    {
        float freqNorm = i / 100.0f;
        float x = xStart + w * freqNorm;

        // Simple lowpass response approximation
        float response = 1.0f;
        if (freqNorm > normalizedCutoff)
        {
            float diff = freqNorm - normalizedCutoff;
            response = std::exp(-diff * 10.0f);
        }

        // Add resonance peak
        float resPeak = std::exp(-std::pow((freqNorm - normalizedCutoff) * 10.0f, 2.0f)) * resonance * 0.5f;
        response += resPeak;
        response = juce::jlimit(0.0f, 1.2f, response);

        float y = yBottom - h * response * 0.7f;
        filterPath.lineTo(x, y);
    }

    // Glow
    g.setColour(juce::Colour(0xffff6600).withAlpha(0.3f));
    g.strokePath(filterPath, juce::PathStrokeType(3.0f));

    // Main line
    g.setColour(juce::Colour(0xffff6600));
    g.strokePath(filterPath, juce::PathStrokeType(1.5f));

    // Label
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText(juce::String((int)cutoff) + " Hz", bounds.reduced(4), juce::Justification::topLeft);
}

//==============================================================================
// QWERTY KEYBOARD INPUT (v1.1)
//==============================================================================

int MicroAcid303AudioProcessorEditor::getKeyboardNoteForKey(int keyCode)
{
    // Lower row: Z-M = C3 (48) to B3 (59)
    // Upper row: Q-P = C4 (60) to B4 (71)
    switch (keyCode)
    {
        // Lower octave (C3-B3)
        case 'Z': return 48;  // C3
        case 'S': return 49;  // C#3
        case 'X': return 50;  // D3
        case 'D': return 51;  // D#3
        case 'C': return 52;  // E3
        case 'V': return 53;  // F3
        case 'G': return 54;  // F#3
        case 'B': return 55;  // G3
        case 'H': return 56;  // G#3
        case 'N': return 57;  // A3
        case 'J': return 58;  // A#3
        case 'M': return 59;  // B3

        // Upper octave (C4-B4)
        case 'Q': return 60;  // C4
        case '2': return 61;  // C#4
        case 'W': return 62;  // D4
        case '3': return 63;  // D#4
        case 'E': return 64;  // E4
        case 'R': return 65;  // F4
        case '5': return 66;  // F#4
        case 'T': return 67;  // G4
        case '6': return 68;  // G#4
        case 'Y': return 69;  // A4
        case '7': return 70;  // A#4
        case 'U': return 71;  // B4
        case 'I': return 72;  // C5
        case '9': return 73;  // C#5
        case 'O': return 74;  // D5
        case '0': return 75;  // D#5
        case 'P': return 76;  // E5

        default: return -1;
    }
}

bool MicroAcid303AudioProcessorEditor::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    int keyCode = key.getTextCharacter();
    if (keyCode >= 'a' && keyCode <= 'z')
        keyCode = keyCode - 'a' + 'A';  // Convert to uppercase

    int note = getKeyboardNoteForKey(keyCode);
    if (note >= 0 && m_keysDown.find(note) == m_keysDown.end())
    {
        m_keysDown.insert(note);
        auto message = juce::MidiMessage::noteOn(1, note, (juce::uint8)100);
        m_audioProcessor.getKeyboardState().noteOn(1, note, 1.0f);
        return true;
    }
    return false;
}

bool MicroAcid303AudioProcessorEditor::keyStateChanged(bool isKeyDown, juce::Component*)
{
    if (!isKeyDown)
    {
        // Check which keys were released
        std::vector<int> toRemove;
        for (int note : m_keysDown)
        {
            // Check if the corresponding key is still pressed
            int keyCode = 0;
            // Reverse lookup - find key for note
            for (int k = 'A'; k <= 'Z'; ++k)
            {
                if (getKeyboardNoteForKey(k) == note)
                {
                    keyCode = k;
                    break;
                }
            }
            for (int k = '0'; k <= '9'; ++k)
            {
                if (getKeyboardNoteForKey(k) == note)
                {
                    keyCode = k;
                    break;
                }
            }

            if (keyCode != 0 && !juce::KeyPress::isKeyCurrentlyDown(keyCode))
            {
                toRemove.push_back(note);
                m_audioProcessor.getKeyboardState().noteOff(1, note, 1.0f);
            }
        }
        for (int note : toRemove)
            m_keysDown.erase(note);

        return !toRemove.empty();
    }
    return false;
}
