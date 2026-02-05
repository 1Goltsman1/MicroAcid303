#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>
#include <random>

/**
 * Arpeggiator with multiple modes, tempo sync, and gate control
 */
class Arpeggiator {
public:
    enum class Mode {
        Up = 0,
        Down,
        UpDown,
        DownUp,
        Random,
        Order,       // Play in order received
        Chord        // Play all notes simultaneously
    };

    enum class Division {
        Whole = 0,   // 1/1
        Half,        // 1/2
        Quarter,     // 1/4
        Eighth,      // 1/8
        Sixteenth,   // 1/16
        ThirtySecond,// 1/32
        DottedQuarter,
        DottedEighth,
        TripletQuarter,
        TripletEighth
    };

    Arpeggiator();
    ~Arpeggiator() = default;

    void prepare(double sampleRate);
    void reset();

    // Process one sample's worth of timing
    // Returns true if a new note should trigger
    bool process(double bpm, int64_t samplePosition);

    // Note management
    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void allNotesOff();

    // Get current note to play
    int getCurrentNote() const { return m_currentNote; }
    float getCurrentVelocity() const { return m_currentVelocity; }
    bool isNoteActive() const { return m_gateOpen && !m_heldNotes.empty(); }
    bool shouldTrigger() const { return m_shouldTrigger; }

    // Parameters
    void setEnabled(bool enabled);
    void setMode(Mode mode);
    void setMode(int index);
    void setDivision(Division div);
    void setDivision(int index);
    void setGate(float gate);        // 0-1 (percentage of note length)
    void setOctaves(int octaves);    // 1-4
    void setSwing(float swing);      // 0-1

    bool isEnabled() const { return m_enabled.load(); }

private:
    void advanceStep();
    void sortNotes();
    double getDivisionInBeats(Division div);

    double m_sampleRate = 44100.0;

    // Note storage
    std::vector<std::pair<int, float>> m_heldNotes;  // note, velocity pairs
    std::vector<int> m_sortedNotes;
    int m_currentStep = 0;
    int m_currentOctave = 0;
    bool m_ascending = true;  // For up/down mode

    // Current output
    int m_currentNote = -1;
    float m_currentVelocity = 0.0f;
    bool m_gateOpen = false;
    bool m_shouldTrigger = false;

    // Timing
    double m_samplesPerBeat = 22050.0;
    double m_sampleCounter = 0.0;
    double m_noteLengthSamples = 0.0;
    double m_gateLengthSamples = 0.0;
    bool m_noteJustTriggered = false;

    // Random
    std::mt19937 m_rng;

    // Parameters
    std::atomic<bool> m_enabled{false};
    std::atomic<Mode> m_mode{Mode::Up};
    std::atomic<Division> m_division{Division::Eighth};
    std::atomic<float> m_gate{0.5f};
    std::atomic<int> m_octaves{1};
    std::atomic<float> m_swing{0.0f};
};
