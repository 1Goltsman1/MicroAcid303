# TB-Style Bassline Plugin (VST3) for Ableton Live 11 – Windows

A **local VST3 instrument plugin** built in C++ that recreates the *behavior and sound style* of a classic TB-style bassline synth (inspired by TB-03 / TB-303 workflow), designed to run **natively inside Ableton Live 11 on Windows**.

> ⚠️ Legal note  
> This plugin must be implemented **from scratch**.  
> Do NOT copy firmware, ROMs, schematics, UI graphics, fonts, or names from Roland.  
> Use “TB-style / Bassline-style” wording only.

---

## 1. Target Environment
- OS: **Windows 10 / 11**
- DAW: **Ableton Live 11**
- Plugin format: **VST3 (Instrument)**
- Language: **C++**
- Framework: **JUCE**
- Compiler: **Visual Studio 2022 (x64)**

---

## 2. What This Plugin Does
- Generates TB-style bass sounds
- Accepts MIDI from Ableton
- Optional internal step sequencer
- Includes overdrive + delay/reverb FX
- Saves state with Ableton projects
- Runs fully locally (no cloud, no servers)

---

## 3. Recommended Tech Stack
### Required
- **JUCE Framework**
- **Visual Studio 2022**
- **CMake**
- **VST3 SDK** (JUCE manages this)

### Why JUCE
- Handles VST3 boilerplate
- Handles Ableton automation
- Handles state saving
- Cross-platform if you expand later

---

## 4. Folder Structure
```
tb-style-bassline/
├─ CMakeLists.txt
├─ /Source
│  ├─ PluginProcessor.h
│  ├─ PluginProcessor.cpp
│  ├─ PluginEditor.h
│  ├─ PluginEditor.cpp
│  ├─ dsp/
│  │  ├─ Oscillator.h
│  │  ├─ Filter.h
│  │  ├─ Envelope.h
│  │  ├─ Slide.h
│  │  ├─ Sequencer.h        (optional)
│  │  └─ Fx.h
├─ /JUCE
└─ README.md
```

---

## 5. Plugin Type
- **Instrument plugin**
- MIDI In → Audio Out
- Mono output (authentic TB behavior)
- Optional stereo FX output

JUCE flags:
- `JucePlugin_IsSynth = 1`
- `JucePlugin_WantsMidiInput = 1`
- `JucePlugin_ProducesMidiOutput = 0`

---

## 6. Build Instructions (Windows)
### 6.1 Install Tools
1. Install **Visual Studio 2022**
   - Workload: *Desktop development with C++*
   - Include Windows SDK
2. Install **Git**
3. Install **CMake**
4. Download or clone **JUCE**

---

### 6.2 Configure Build
Open **x64 Native Tools Command Prompt for VS 2022**

```bash
cd tb-style-bassline
cmake -B build -G "Visual Studio 17 2022" -A x64
```

---

### 6.3 Build Plugin
```bash
cmake --build build --config Release
```

Output:
```
build/.../Release/TBStyleBassline.vst3
```

---

## 7. Install Plugin for Ableton
Copy the `.vst3` folder to:
```
C:\Program Files\Common Files\VST3
```

Final path example:
```
C:\Program Files\Common Files\VST3\TBStyleBassline.vst3
```

---

## 8. Enable in Ableton Live 11
1. Open Ableton
2. Preferences → Plug-ins
3. Enable **VST3 System Folders**
4. Click **Rescan**
5. Plugin appears under:
   ```
   Plug-ins → VST3 → TBStyleBassline
   ```

---

## 9. Sound Engine Design (Core)

### 9.1 Oscillator
- Waveforms:
  - Saw
  - Square
- Monophonic
- Band-limited (PolyBLEP or MinBLEP)
- Fine tune ±50 cents

---

### 9.2 Filter (TB-Style)
- 24 dB low-pass ladder-inspired filter
- Controls:
  - Cutoff
  - Resonance
  - Envelope Mod Amount
- Non-linear saturation
- Self-oscillation limited for safety

---

## 10. FX Section

### 10.1 Overdrive
- Drive knob
- 3 modes:
  1. Soft overdrive
  2. Classic distortion
  3. Saturated + compressed

Implemented as:
- Waveshaper + pre/post filtering

---

### 10.2 Delay / Reverb
Mode switch:
- Tape-style delay
- Digital delay
- Reverb-style diffusion

Parameters:
- Time (ms or tempo-sync)
- Feedback
- Mix

Host BPM from Ableton supported.

---

## 11. Optional Internal Sequencer
(You may skip this and rely on Ableton MIDI)

### Features
- 16-step pattern
- Per step:
  - Note
  - Accent
  - Slide
- Shuffle
- Sync to host tempo
- Saved with project state

---

## 12. Parameters (Automation-Ready)

### Synth
- Waveform
- Tune
- Cutoff
- Resonance
- Env Mod
- Decay
- Accent
- Slide Time
- Output Gain

### FX
- Drive
- Drive Mode
- FX Type
- FX Time
- Feedback
- Mix

---

## 13. State Saving (Mandatory)
Ableton expects full recall.

Implement in JUCE:
- `getStateInformation()`
- `setStateInformation()`

Store:
- All parameters
- Sequencer pattern data (if used)

Use `AudioProcessorValueTreeState`.

---

## 14. Performance Rules
- No memory allocation in audio thread
- Parameter smoothing
- Fixed buffer processing
- Output soft-clipper or limiter
- Mono signal path before FX

---

## 15. Development Roadmap
### Phase 1
- JUCE plugin loads in Ableton
- Oscillator + amp

### Phase 2
- Filter + envelope
- Accent + slide

### Phase 3
- Overdrive
- Delay / reverb

### Phase 4 (Optional)
- Internal sequencer
- Preset system

---

## 16. Debugging
- Test first in **JUCE AudioPluginHost**
- Use Debug build for crashes
- Check Ableton crash logs if needed

---

## 17. Branding & Safety
✔ Allowed:
- “TB-style bassline synth”
- “Inspired by classic bassline workflow”

✖ Not allowed:
- Roland name/logo
- TB-03 UI copies
- Firmware cloning

---

## 18. Result
You end up with:
- A **native Windows VST3**
- Fully local
- Loads in Ableton Live 11
- Authentic TB-style bass sound
- Extendable for future versions
