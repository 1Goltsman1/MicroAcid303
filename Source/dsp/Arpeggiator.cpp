#include "Arpeggiator.h"
#include <algorithm>

Arpeggiator::Arpeggiator() : m_rng(std::random_device{}())
{
}

void Arpeggiator::prepare(double sampleRate)
{
    m_sampleRate = sampleRate;
    reset();
}

void Arpeggiator::reset()
{
    m_currentStep = 0;
    m_currentOctave = 0;
    m_ascending = true;
    m_sampleCounter = 0.0;
    m_gateOpen = false;
    m_shouldTrigger = false;
    m_currentNote = -1;
    m_currentVelocity = 0.0f;
}

bool Arpeggiator::process(double bpm, int64_t samplePosition)
{
    m_shouldTrigger = false;

    if (!m_enabled.load() || m_heldNotes.empty())
    {
        m_gateOpen = false;
        m_currentNote = -1;
        return false;
    }

    // Calculate timing
    m_samplesPerBeat = m_sampleRate * 60.0 / bpm;

    Division div = m_division.load();
    double beatsPerNote = getDivisionInBeats(div);
    m_noteLengthSamples = m_samplesPerBeat * beatsPerNote;

    // Apply swing to even steps
    float swing = m_swing.load();
    if (m_currentStep % 2 == 1 && swing > 0.0f)
    {
        m_noteLengthSamples *= (1.0 + swing * 0.5);
    }

    float gate = m_gate.load();
    m_gateLengthSamples = m_noteLengthSamples * gate;

    m_sampleCounter += 1.0;

    // Check if we need to close the gate
    if (m_gateOpen && m_sampleCounter >= m_gateLengthSamples)
    {
        m_gateOpen = false;
    }

    // Check if we need to trigger a new note
    if (m_sampleCounter >= m_noteLengthSamples)
    {
        m_sampleCounter = 0.0;
        advanceStep();
        m_gateOpen = true;
        m_shouldTrigger = true;
        return true;
    }

    return false;
}

void Arpeggiator::advanceStep()
{
    if (m_sortedNotes.empty())
    {
        sortNotes();
        if (m_sortedNotes.empty()) return;
    }

    Mode mode = m_mode.load();
    int octaves = m_octaves.load();
    int numNotes = static_cast<int>(m_sortedNotes.size());
    int totalSteps = numNotes * octaves;

    // Get base note index based on mode
    int noteIndex = 0;

    switch (mode)
    {
        case Mode::Up:
            noteIndex = m_currentStep % numNotes;
            m_currentOctave = (m_currentStep / numNotes) % octaves;
            m_currentStep = (m_currentStep + 1) % totalSteps;
            break;

        case Mode::Down:
            noteIndex = (numNotes - 1) - (m_currentStep % numNotes);
            m_currentOctave = (octaves - 1) - ((m_currentStep / numNotes) % octaves);
            m_currentStep = (m_currentStep + 1) % totalSteps;
            break;

        case Mode::UpDown:
            if (totalSteps <= 1)
            {
                noteIndex = 0;
                m_currentOctave = 0;
            }
            else
            {
                int cycle = totalSteps * 2 - 2;
                int pos = m_currentStep % cycle;
                if (pos < totalSteps)
                {
                    noteIndex = pos % numNotes;
                    m_currentOctave = pos / numNotes;
                }
                else
                {
                    int downPos = cycle - pos;
                    noteIndex = downPos % numNotes;
                    m_currentOctave = downPos / numNotes;
                }
                m_currentStep = (m_currentStep + 1) % cycle;
            }
            break;

        case Mode::DownUp:
            if (totalSteps <= 1)
            {
                noteIndex = 0;
                m_currentOctave = 0;
            }
            else
            {
                int cycle = totalSteps * 2 - 2;
                int pos = m_currentStep % cycle;
                if (pos < totalSteps)
                {
                    int downPos = totalSteps - 1 - pos;
                    noteIndex = downPos % numNotes;
                    m_currentOctave = downPos / numNotes;
                }
                else
                {
                    int upPos = pos - totalSteps + 1;
                    noteIndex = upPos % numNotes;
                    m_currentOctave = upPos / numNotes;
                }
                m_currentStep = (m_currentStep + 1) % cycle;
            }
            break;

        case Mode::Random:
        {
            std::uniform_int_distribution<int> noteDist(0, numNotes - 1);
            std::uniform_int_distribution<int> octDist(0, octaves - 1);
            noteIndex = noteDist(m_rng);
            m_currentOctave = octDist(m_rng);
            m_currentStep++;
            break;
        }

        case Mode::Order:
            // Play in order notes were pressed (use heldNotes directly)
            if (!m_heldNotes.empty())
            {
                int idx = m_currentStep % static_cast<int>(m_heldNotes.size());
                m_currentNote = m_heldNotes[idx].first + m_currentOctave * 12;
                m_currentVelocity = m_heldNotes[idx].second;
                m_currentOctave = (m_currentStep / static_cast<int>(m_heldNotes.size())) % octaves;
                m_currentStep = (m_currentStep + 1) % (static_cast<int>(m_heldNotes.size()) * octaves);
            }
            return;

        case Mode::Chord:
            // Chord mode handled differently - not really arpeggio
            noteIndex = 0;
            m_currentOctave = 0;
            break;
    }

    // Set current note
    if (noteIndex >= 0 && noteIndex < numNotes)
    {
        m_currentNote = m_sortedNotes[noteIndex] + m_currentOctave * 12;

        // Find velocity for this note
        for (const auto& pair : m_heldNotes)
        {
            if (pair.first == m_sortedNotes[noteIndex])
            {
                m_currentVelocity = pair.second;
                break;
            }
        }
    }
}

void Arpeggiator::sortNotes()
{
    m_sortedNotes.clear();
    for (const auto& pair : m_heldNotes)
    {
        m_sortedNotes.push_back(pair.first);
    }
    std::sort(m_sortedNotes.begin(), m_sortedNotes.end());
}

double Arpeggiator::getDivisionInBeats(Division div)
{
    switch (div)
    {
        case Division::Whole:         return 4.0;
        case Division::Half:          return 2.0;
        case Division::Quarter:       return 1.0;
        case Division::Eighth:        return 0.5;
        case Division::Sixteenth:     return 0.25;
        case Division::ThirtySecond:  return 0.125;
        case Division::DottedQuarter: return 1.5;
        case Division::DottedEighth:  return 0.75;
        case Division::TripletQuarter:return 2.0 / 3.0;
        case Division::TripletEighth: return 1.0 / 3.0;
        default:                      return 0.5;
    }
}

// === NOTE MANAGEMENT ===

void Arpeggiator::noteOn(int midiNote, float velocity)
{
    // Check if note already held
    for (auto& pair : m_heldNotes)
    {
        if (pair.first == midiNote)
        {
            pair.second = velocity;
            return;
        }
    }

    m_heldNotes.push_back({midiNote, velocity});
    sortNotes();

    // If this is the first note, start immediately
    if (m_heldNotes.size() == 1)
    {
        m_currentStep = 0;
        m_currentOctave = 0;
        m_sampleCounter = m_noteLengthSamples; // Trigger immediately
    }
}

void Arpeggiator::noteOff(int midiNote)
{
    m_heldNotes.erase(
        std::remove_if(m_heldNotes.begin(), m_heldNotes.end(),
            [midiNote](const std::pair<int, float>& p) { return p.first == midiNote; }),
        m_heldNotes.end());

    sortNotes();

    if (m_heldNotes.empty())
    {
        m_gateOpen = false;
        m_currentNote = -1;
    }
}

void Arpeggiator::allNotesOff()
{
    m_heldNotes.clear();
    m_sortedNotes.clear();
    m_gateOpen = false;
    m_currentNote = -1;
    m_currentStep = 0;
    m_currentOctave = 0;
}

// === PARAMETER SETTERS ===

void Arpeggiator::setEnabled(bool enabled)
{
    m_enabled.store(enabled, std::memory_order_relaxed);
    if (!enabled)
    {
        m_gateOpen = false;
    }
}

void Arpeggiator::setMode(Mode mode)
{
    m_mode.store(mode, std::memory_order_relaxed);
}

void Arpeggiator::setMode(int index)
{
    if (index >= 0 && index <= 6)
        m_mode.store(static_cast<Mode>(index), std::memory_order_relaxed);
}

void Arpeggiator::setDivision(Division div)
{
    m_division.store(div, std::memory_order_relaxed);
}

void Arpeggiator::setDivision(int index)
{
    if (index >= 0 && index <= 9)
        m_division.store(static_cast<Division>(index), std::memory_order_relaxed);
}

void Arpeggiator::setGate(float gate)
{
    m_gate.store(std::max(0.1f, std::min(1.0f, gate)), std::memory_order_relaxed);
}

void Arpeggiator::setOctaves(int octaves)
{
    m_octaves.store(std::max(1, std::min(4, octaves)), std::memory_order_relaxed);
}

void Arpeggiator::setSwing(float swing)
{
    m_swing.store(std::max(0.0f, std::min(1.0f, swing)), std::memory_order_relaxed);
}
