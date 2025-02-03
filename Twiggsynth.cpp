//////////////////////////////////////////////
////// SIMPLE X DAISY PINOUT CHEATSHEET //////

// 3v3           29  |       |   20    AGND
// D15 / A0      30  |       |   19    OUT 01
// D16 / A1      31  |       |   18    OUT 00
// D17 / A2      32  |       |   17    IN 01
// D18 / A3      33  |       |   16    IN 00
// D19 / A4      34  |       |   15    D14
// D20 / A5      35  |       |   14    D13
// D21 / A6      36  |       |   13    D12
// D22 / A7      37  |       |   12    D11
// D23 / A8      38  |       |   11    D10
// D24 / A9      39  |       |   10    D9
// D25 / A10     40  |       |   09    D8
// D26           41  |       |   08    D7
// D27           42  |       |   07    D6
// D28 / A11     43  |       |   06    D5
// D29           44  |       |   05    D4
// D30           45  |       |   04    D3
// 3v3 Digital   46  |       |   03    D2
// VIN           47  |       |   02    D1
// DGND          48  |       |   01    D0

#ifndef USE_DAISYSP_LGPL
#define USE_DAISYSP_LGPL
#endif

#include "daisy_seed.h"
#include "daisysp.h"
#include "Twiggsynth.h"
#include <list>

using namespace daisy;
using namespace daisysp;

enum Knob {
  KNOB_1,
  KNOB_2,
  KNOB_3,
  KNOB_LAST
};

enum TripleToggle {
  TRIPTOGGLE_1,
  TRIPTOGGLE_LAST
};

constexpr Pin KNOB_1_PIN = seed::D15; // simple board: 30, Volume
constexpr Pin KNOB_2_PIN = seed::D16; // simple board: 31, LFO Freq
constexpr Pin KNOB_3_PIN = seed::D25; // simple board: 40, VCO Freq

constexpr Pin TRIPTOGGLE_1_UP_PIN = seed::D14;
constexpr Pin TRIPTOGGLE_1_DN_PIN = seed::D13;

// Declare a DaisySeed object called hardware
static DaisySeed  hardware;
static Oscillator osc, subOsc, lfo;
static MoogLadder flt;
static Parameter  p_subOsc_freq, p_lfo_freq, p_volume;
static MidiUsbHandler midi;

// "Except for the more recent dual core MCUs, most STM32 are limited to running only one thing at a time"
// so, no guards against modify-during-read. Good thing as _mutex_ aren't available to the compiler!
static std::list<float> noteOnList;

AnalogControl knob1,
    knob2,
    knob3,
    *knobs[KNOB_LAST];

Switch3 tripleToggle1,
    *tripleToggles[TRIPTOGGLE_LAST];

// TODO: Make this method more efficient. 
// I think I can shortcircuit a bunch of processing.
void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
  float subOsc_freq, lfo_freq, filt_freq, osc_out, subOsc_out, filtered_out, volume;

  ProcessAllControls();

  float noteOn = noteOnList.empty()
    ? 0.0f
    : noteOnList.front();

  auto modeToggle = tripleToggle1.Read();

  // IDEA: slew/glide between notes.
  // IDEA: more (sub)harmonic oscs!

  if (noteOn > 0.0f) {
    volume = 1 - p_volume.Process();
  } else {
    volume = 0;
  }

  osc.SetFreq(mtof(noteOn));

  subOsc_freq = mtof(noteOn - p_subOsc_freq.Process());

  subOsc.SetWaveform(modeToggle == Switch3::POS_UP
    ? subOsc.WAVE_POLYBLEP_SAW
    : subOsc.WAVE_SIN);
  subOsc.SetFreq(subOsc_freq > 0.0f ? subOsc_freq : 0.0f);

  lfo_freq = 20 - p_lfo_freq.Process();
  lfo.SetFreq(lfo_freq);

  // Fill output buffer with samples
  for(size_t i = 0; i < size; i += 2)
  {
    osc_out = osc.Process();
    subOsc_out = subOsc.Process();

    filt_freq = 5000 + (lfo.Process() * 5000);
    flt.SetFreq(filt_freq); // hi pass filter cutoff

    if (modeToggle == Switch3::POS_DOWN) {
      filtered_out = flt.Process(osc_out);
    } else {
      filtered_out = flt.Process(osc_out + subOsc_out) / 2;
      volume *= 1.25;
    }

    //Set the left and right outputs
    out[i]     = filtered_out * volume;
    out[i + 1] = filtered_out * volume;
  }
}

int main(void)
{
  noteOnList = std::list<float>();
  
  hardware.Init();

  InitKnobs();
  InitSwitches();

  hardware.SetAudioBlockSize(4);

  InitSynth(hardware.AudioSampleRate());
  InitMidi();

  // let everything settle
	System::Delay(100);

  hardware.adc.Start();

  hardware.StartAudio(AudioCallback);

  while(1) { ProcessMidi(); }
}

void ProcessMidi() {
  /** Listen to MIDI for new changes */
  midi.Listen();

  /** When there are messages waiting in the queue... */
  while(midi.HasEvents())
  {
    /** Pull the oldest one from the list... */
    auto msg = midi.PopEvent();
    switch(msg.type)
    {
      case NoteOn:
      {
        auto note_msg = msg.AsNoteOn();
        if(note_msg.velocity != 0) {
          noteOnList.push_front(note_msg.note);
        }
      }
      break;
      case NoteOff:
      {
        auto note_msg = msg.AsNoteOff();
        noteOnList.remove(note_msg.note);
      }
      break;
        // Ignore all other message types
      default: break;
    }
  }
}

/** Initialize USB Midi 
 *  by default this is set to use the built in (USB FS) peripheral.
 * 
 *  by setting midi_cfg.transport_config.periph = MidiUsbTransport::Config::EXTERNAL
 *  the USB HS pins can be used (as FS) for MIDI 
 */
void InitMidi() {
  MidiUsbHandler::Config midi_cfg;
  midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
  midi.Init(midi_cfg);
}

void InitSynth(float samplerate)
{
  // Init freq Parameter to knob1 using MIDI note numbers
  // min 10, max 127, curve linear
  p_volume.Init(knob1, 0, 1, Parameter::LINEAR);
  p_lfo_freq.Init(knob2, 0, 20, Parameter::EXPONENTIAL);
  p_subOsc_freq.Init(knob3, 0, 24, Parameter::LINEAR);

  osc.Init(samplerate);
  osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
  osc.SetAmp(1.f);
  osc.SetFreq(0);

  subOsc.Init(samplerate);
  subOsc.SetWaveform(subOsc.WAVE_POLYBLEP_SAW);
  subOsc.SetAmp(1.f); // maybe lower the subosc vol? Or add a mixer?
  subOsc.SetFreq(0);

  lfo.Init(samplerate);
  lfo.SetWaveform(lfo.WAVE_POLYBLEP_TRI);
  lfo.SetFreq(0.1);

  flt.Init(samplerate);
  flt.SetRes(0.7);
}

void ProcessAnalogControls()
{
  knob1.Process();
  knob2.Process();
  knob3.Process();
}

void ProcessDigitalControls()
{
  // None at the moment.
}

void InitKnobs()
{
  AdcChannelConfig knob_init[KNOB_LAST];

  knob_init[KNOB_1].InitSingle(KNOB_1_PIN);
  knob_init[KNOB_2].InitSingle(KNOB_2_PIN);
  knob_init[KNOB_3].InitSingle(KNOB_3_PIN);

  hardware.adc.Init(knob_init, KNOB_LAST);

  knobs[KNOB_1] = &knob1;
  knobs[KNOB_2] = &knob2;
  knobs[KNOB_3] = &knob3;

  for (int i = 0; i < KNOB_LAST; i++)
  {
    knobs[i]->Init(hardware.adc.GetPtr(i), hardware.AudioCallbackRate());
  }
}

void InitSwitches() {
  tripleToggle1.Init(TRIPTOGGLE_1_UP_PIN, TRIPTOGGLE_1_DN_PIN);

  tripleToggles[TRIPTOGGLE_1] = &tripleToggle1;
}
