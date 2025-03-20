# Twigg*synth*

## Author

@Gtwiggs

## Description

A learning platform for music synthesis using a Daisy Seed.

## State Of Play

**_3/19/2025_**

- 2 Oscillator Monophonic synthesizer.
  - Suboscillator tied to main oscillator.
  - Oscillator and suboscillator waveform: sawtooth.
  - Latest note priority.
  - If a note is held, it will resume playing after later notes are released.
  - Portamento across entire range. Resets when no notes are active. Fixed glide time of 0.05s.
- Mono output.
- LFO Mod source; waveform: triangle
- Ladder Filter.
  - Low Pass 2 pole filter.
- Fixed ADSR envelope.
- MIDI Support:
  - UART MIDI ~ 5 pin DIN.
  - Note on/off.
  - Pitch bend +/- 1 octave.
  - Listens on all channels.
- Hardware controls:
  - Volume
  - LFO frequency: modulates the filter cutoff.
  - Sub-oscillator detune: 0 to -2 octaves.
  - Flter resonance.
