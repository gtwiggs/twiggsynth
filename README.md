# Twiggsynth

## Author

Gtwiggs

## Description

A digital synthesis musical instrument.

_Twigg**synth**_ is a learning platform for _so many things_. Some loose goals:

- Organizing code to make the addition/removal and declaration of on-board components easy.
- Software-based routing between instrument components based on panel input.
- Self-playing, maybe with seed note(s).
- Monophonic, perhaps paraphonic.

## State Of Play

**_2/3/2024_**

- Monophonic dual oscillator synthesis.
  - New notes supercede prior notes.
  - If a note is held, it will resume playing after later notes are released.
- Note input via USB MIDI on the builtin connector.
  - Listens on all channels.
- Leftmost Knob controls the sub-oscillator pitch.
  - This oscillator contributes at a pitch from 0 to two octaves below the MIDI note.
- Center knob controls LFO Rate
- LFO modulates the Moog Ladder filter cutoff.
- Rightmost knob controls volume.
- 3-position toggle switch chooses the subosc contribution:
  - UP: subosc is a Sawtooth waveform.
  - CENTER: subosc is a Sine waveform.
  - DOWN: subosc is disabled.
- Mini jack is L/Mono line level out.
