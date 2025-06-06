# Twigg*synth*

## Author

@Gtwiggs

## Description

A learning platform for music synthesis using a Daisy Seed.

## State Of Play

**_4/15/2025_**

-   2 Oscillator Monophonic synthesizer.
    -   Oscillator and suboscillator waveform: sawtooth.
    -   Suboscillator frequency referenced to main oscillator.
    -   Latest note priority.
    -   If a note is held, it will resume playing after later notes are released.
    -   Portamento across entire range. Resets when no notes are active. Fixed glide time of 0.05s.
-   Mono output.
-   LFO Mod source; waveform: triangle.
-   Ladder Filter.
    -   Low Pass 2 pole filter.
-   Tremolo accessed via MIDI channel pressure. 50% wet/dry, Triangle waveform.
-   ADSR envelope.
    -   Attack 0s - 5s.
    -   Decay not settable.
    -   Sustain level 1 - 0.
    -   Release 0s - note hold.
-   Slider to select waveforms.
    -   Sets toe waveforms for the main and sub oscillators.
    -   The slider provides access t all 9 combinations of Sawtooth, Square and Triangle waveforms.
-   MIDI Support:
    -   UART MIDI ~ 5 pin DIN connectors. _IN_ and _OUT_ connectors are provided, but only _IN_ is implemented.
    -   Note on/off.
    -   Pitch bend +/- 1 octave.
    -   Channel Pressure modulates Tremolo
    -   Listens on all channels.

## Hardware Controls

| Label | Pin | Function                                                  |
| :---: | :-: | --------------------------------------------------------- |
|  k1   | D24 | Volume                                                    |
|  k2   | D23 | Flter cutoff.                                             |
|  k3   | D20 | Flter resonance.                                          |
|  k4   | D15 | Attack time.                                              |
|  k5   | D16 | Sustain level.                                            |
|  k6   | D18 | Release time.                                             |
|  k7   | D22 | LFO frequency: modulates the filter cutoff. Disabled @ 0. |
|  k8   | D21 | Sub-oscillator detune: 0 to -2 octaves.                   |
|  k9   | D19 | Portamento time.                                          |
|  s1   | D17 | Waveform Picker for main and sub oscillators.             |
|  led  |     | Power indicator                                           |

```
+----------------------+
|  k1         led      |
|        k2            |
|              k3      |
|   k7                 |
|                 k4   |
|         k8           |
|                 k5   |
|                      |
|             k6    /  |
|     k9          s1   |
|                /     |
+----------------------+
```
