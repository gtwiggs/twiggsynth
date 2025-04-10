# Twigg*synth*

## Author

@Gtwiggs

## Description

A learning platform for music synthesis using a Daisy Seed.

## State Of Play

**_3/19/2025_**

-   2 Oscillator Monophonic synthesizer.
    -   Oscillator and suboscillator waveform: sawtooth.
    -   Suboscillator frequency referenced to main oscillator.
    -   Latest note priority.
    -   If a note is held, it will resume playing after later notes are released.
    -   Portamento across entire range. Resets when no notes are active. Fixed glide time of 0.05s.
-   Mono output.
-   LFO Mod source; waveform: triangle
-   Ladder Filter.
    -   Low Pass 2 pole filter.
-   ADSR envelope.
-   MIDI Support:
    -   UART MIDI ~ 5 pin DIN.
    -   Note on/off.
    -   Pitch bend +/- 1 octave.
    -   Channel Pressure modulates Tremolo
    -   Listens on all channels.

## Hardware Controls

| Label | Pin | Function                                                  |
| :---: | :-: | --------------------------------------------------------- |
|  k1   | D24 | Volume                                                    |
|  k2   | D23 | Flter resonance.                                          |
|  k3   | D20 | Attack time.                                              |
|  k4   | D15 | Release time.                                             |
|  k5   | D16 | Portamento time.                                          |
|  k6   | D18 | _unassigned_                                              |
|  k7   | D22 | LFO frequency: modulates the filter cutoff. Disabled @ 0. |
|  k8   | D21 | Sub-oscillator detune: 0 to -2 octaves.                   |
|  k9   | D19 | _unassigned_                                              |
|  s1   | D17 | _unassigned_                                              |
|  led  |     | Power                                                     |

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
