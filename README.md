# Twigg*synth*

## Author

@Gtwiggs

## Description

A learning platform for music synthesis using a Daisy Seed.

## State Of Play

**_3/19/2025_**

- 2 Oscillator Monophonic synthesizer.
  - Oscillator and suboscillator waveform: sawtooth.
  - Suboscillator frequency referenced to main oscillator.
  - Latest note priority.
  - If a note is held, it will resume playing after later notes are released.
  - Portamento across entire range. Resets when no notes are active. Fixed glide time of 0.05s.
- Mono output.
- LFO Mod source; waveform: triangle
- Ladder Filter.
  - Low Pass 2 pole filter.
- ADSR envelope.
- MIDI Support:
  - UART MIDI ~ 5 pin DIN.
  - Note on/off.
  - Pitch bend +/- 1 octave.
  - Listens on all channels.
- Hardware controls:
  - `K1`: Volume
  - `K2`: LFO frequency: modulates the filter cutoff.
  - `K3`: Sub-oscillator detune: 0 to -2 octaves.
  - `K4`: Flter resonance.
  - `K5`: Attack time.
  - `K6`: Release time.
  - `B1 + K6`: Portamento time.

```
MIDI Rx   |
MIDI Tx   |   B1
----------+----------------------
       K1 |
          |  K2  K3  K4   K5  K6
    S1 S2 |
```
