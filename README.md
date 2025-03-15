# Twiggsynth

## Author

Gtwiggs

## Description

A digital synthesis musical instrument.

_Twigg**synth**_ is a learning platform for _so many things_. Some loose goals:

- Organizing code to make the addition/removal and declaration of hardware components easy.
- Software-based routing between instrument components based on panel input.
- Self-playing, maybe with seed note(s).
- Paraphonic.

## State Of Play

**_3/14/2025_**

- Monophonic synthesizer.
  - Suboscillator tied to main oscillator.
  - New notes replace prior notes.
  - If a note is held, it will resume playing after later notes are released.
  - Portamento for held notes. Resets when no notes are active. Fixed at 0.05s.
- Mono output.
- Note input via UART MIDI.
  - Listens on all channels.
- Hardware controls:
  - Volume
  - LFO frequency: LFO modulates the Moog Ladder filter cutoff.
  - Sub-oscillator detune: 0 to -2 octaves. Waveform: Sawtooth
  - Moog Ladder flter resonance.
