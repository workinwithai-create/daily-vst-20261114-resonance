# November Resonance

**Today’s Instrument: November Resonance**  
A warm polyphonic pad synthesizer that breathes with channel aftertouch (modulating filter cutoff and subtle vibrato) and respects the sustain pedal for natural, live-feeling note holds. Dark cinematic atmospheres.

## Build Notes

### Prerequisites
- CMake 3.22+
- C++20 compiler (Clang, GCC, or MSVC)
- Git

### Local Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Artefacts appear under:
- `build/NovemberResonance_artefacts/Release/VST3/`
- `build/NovemberResonance_artefacts/Release/Standalone/`

### Features
- Polyphonic (8 voices) dual-oscillator subtractive synth
- Resonant lowpass filter
- Full ADSR
- Channel aftertouch modulates cutoff and vibrato
- Sustain pedal (CC64) support
- Safety limiter
- Full state save/load via APVTS
- Dark cinematic UI

### Company
MPJRecords  
Manufacturer Code: MPJR  
Plugin Code: N14R

Built with JUCE 8 via FetchContent.
