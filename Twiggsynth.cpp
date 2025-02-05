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
#include <vector>

using namespace daisy;
using namespace daisysp;

/**
 * @brief Analog control definitions
 */
enum AnalogControlName {
    VOLUME,
    LFO_FREQ,
    SUBOSC_FREQ_DETUNE,
    LAST_CONTROL
};

struct AnalogControlDefn {
    AnalogControlName name;
    Pin pin;
    float min;
    float max;
    Parameter::Curve curve;
    bool flipped;

    // Constructor
    AnalogControlDefn(AnalogControlName name, Pin pin, float min, float max, Parameter::Curve curve, bool flipped = false)
        : name(name), pin(pin), min(min), max(max), curve(curve), flipped(flipped) {}
};

std::vector<AnalogControlDefn> analogControlDefns = {
    { VOLUME, seed::D15, 0, 1, Parameter::LINEAR, true },
    { LFO_FREQ, seed::D16, 0, 20, Parameter::EXPONENTIAL, true },
    { SUBOSC_FREQ_DETUNE, seed::D25, 0, 24, Parameter::LINEAR }
};

AnalogControl analogControls[LAST_CONTROL];
Parameter params[LAST_CONTROL];

/**
 * @brief Digital control definitions
 */
enum TripleToggle {
  TRIPTOGGLE_1,
  TRIPTOGGLE_LAST
};

Switch3 tripleToggle1,
    *tripleToggles[TRIPTOGGLE_LAST];

constexpr Pin TRIPTOGGLE_1_UP_PIN = seed::D14;
constexpr Pin TRIPTOGGLE_1_DN_PIN = seed::D13;

/****************************************************************************************************
 * Declare a DaisySeed object called hardware
 */
static DaisySeed  hardware;
static Oscillator osc, subOsc, lfo;
static MoogLadder flt;
static MidiUsbHandler midi;
static Port       slew;


/**
 * @brief List of currently pressed notes
 * 
 * "Except for the more recent dual core MCUs, most STM32 are limited to running only one thing at a time"
 * No guards against modify-during-read. Good thing as _mutex_ isn't available to the compiler!
 */
static std::list<float> noteOnList;

/**
 * Slew time.
 */
static float DEFAULT_SLEW_TIME = 0.03f;

// TODO: slew/glide between notes.
void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
  float subOsc_freq, lfo_freq, filt_freq, osc_out, subOsc_out, filtered_out, volume, slewed_freq;

  ProcessAllControls();

  float noteOn = noteOnList.empty()
    ? 0.0f
    : noteOnList.front();

  auto modeToggle = tripleToggle1.Read();

  if (noteOn > 0.0f) {
    volume = params[VOLUME].Process();
    slewed_freq = slew.Process(mtof(noteOn));
    slew.SetHtime(DEFAULT_SLEW_TIME);
  } else {
    volume = 0;
    // Force slew to timeout if there is no note.
    slewed_freq = mtof(noteOn);
    slew.SetHtime(0);
  }

  osc.SetFreq(slewed_freq);

  // Convert the slered freq back to midi to calc the subosc freq by
  // inverting the mtof calc: powf(2, (m - 69.0f) / 12.0f) * 440.0f;
  float slew_midi = (std::log((slewed_freq / 440.0f))/std::log(2)*12)+69.0f;
  subOsc_freq = mtof(slew_midi - params[SUBOSC_FREQ_DETUNE].Process());

  subOsc.SetWaveform(modeToggle == Switch3::POS_UP
    ? subOsc.WAVE_POLYBLEP_SAW
    : subOsc.WAVE_SIN);
  subOsc.SetFreq(subOsc_freq > 0.0f ? subOsc_freq : 0.0f);

  lfo_freq = params[LFO_FREQ].Process();
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
  hardware.StartLog();
  hardware.PrintLine("Initializing ...");

  InitAnalogControls();
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
          hardware.PrintLine("NoteOn: " FLT_FMT3, FLT_VAR3(note_msg.note));
        }
      }
      break;
      case NoteOff:
      {
        auto note_msg = msg.AsNoteOff();
        noteOnList.remove(note_msg.note);
        hardware.PrintLine("NoteOff: " FLT_FMT3, FLT_VAR3(note_msg.note));
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
  osc.Init(samplerate);
  osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
  osc.SetAmp(1.f);
  osc.SetFreq(0);

  subOsc.Init(samplerate);
  subOsc.SetWaveform(subOsc.WAVE_POLYBLEP_SAW);
  subOsc.SetAmp(1.f);
  subOsc.SetFreq(0);

  lfo.Init(samplerate);
  lfo.SetWaveform(lfo.WAVE_POLYBLEP_TRI);
  lfo.SetFreq(0.1);

  flt.Init(samplerate);
  flt.SetRes(0.7);

  slew.Init(samplerate, DEFAULT_SLEW_TIME);
}

void InitAnalogControls() {
  AdcChannelConfig channels[analogControlDefns.size()];

  int i = 0;
  for (auto defn : analogControlDefns) {
    channels[i++].InitSingle(defn.pin);
  }

  hardware.adc.Init(channels, analogControlDefns.size());

  // Initialize controls.

  for (auto defn : analogControlDefns) {
    analogControls[defn.name].Init(hardware.adc.GetPtr(defn.name), hardware.AudioCallbackRate(), defn.flipped);
  }

  // Initialize parameters - they normalize values to a range and curve.

  for (auto defn : analogControlDefns) {
    params[defn.name].Init(analogControls[defn.name], defn.min, defn.max, defn.curve);
  }
}

void ProcessAnalogControls()
{
  for (auto defn : analogControlDefns)
    analogControls[defn.name].Process();
}

void ProcessDigitalControls()
{
  // None at the moment.
}

void InitSwitches() {
  tripleToggle1.Init(TRIPTOGGLE_1_UP_PIN, TRIPTOGGLE_1_DN_PIN);

  tripleToggles[TRIPTOGGLE_1] = &tripleToggle1;
}
