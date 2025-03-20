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

#include "Twiggsynth.h"
#include "TsPort.h"
#include <list>
#include <vector>

using namespace daisy;
using namespace daisysp;

#define LOG_WRITE_ENABLED 0

/**************************************************************************************************
 * @brief Analog control definitions
 */
enum AnalogControlName : unsigned int {
    VOLUME,
    LFO_FREQ,
    SUBOSC_FREQ_DETUNE,
    RESONANCE,
    LAST_CONTROL
};

std::vector<AnalogControlDefn> analogControlDefns = {
    { VOLUME,             seed::D15, 0.0f,  1.0f, Parameter::LINEAR           },
    { LFO_FREQ,           seed::D16, 0.0f, 20.0f, Parameter::LINEAR           },
    { SUBOSC_FREQ_DETUNE, seed::D17, 0.0f, 24.0f, Parameter::LINEAR,     true },
    { RESONANCE,          seed::D18, 0.0f,  1.8f, Parameter::EXPONENTIAL      }
};

AnalogControl analogControls[LAST_CONTROL];
Parameter params[LAST_CONTROL];

/**************************************************************************************************
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

/**************************************************************************************************
 * Declare a DaisySeed object called hardware
 */
static DaisySeed        hardware;
static Oscillator       osc, subOsc, lfo;
static LadderFilter     flt;
static MidiUartHandler  midi;
static TsPort           slew;
static Adsr             env;
static float            playing_note = 0.0f;
static float            pitch_bend_as_semitone = 0.0f;

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

/** FIFO to hold messages as we're ready to print them */
FIFO<MidiEvent, 128> event_log;

// TODO: slew/glide between notes.
void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
  float subOsc_freq, lfo_freq, resonance, osc_out, subOsc_out, filtered_out, env_out, volume, slewed_freq, cutoff;
  bool gate;

  ProcessAllControls();

  // Fill output buffer with samples
  for(size_t i = 0; i < size; i += 2)
  {
    float noteOn = noteOnList.empty()
      ? 0.0f
      : noteOnList.front();

    if (noteOn > 0.0f) {
      if (playing_note == 0.0f) {
        slew.SetFreq(mtof(noteOn + pitch_bend_as_semitone));
      }
      playing_note = noteOn;
      slewed_freq = slew.Process(mtof(playing_note + pitch_bend_as_semitone));
      gate = true;
    } else {
      if (env.IsRunning()) {
        slewed_freq = slew.Process(mtof(playing_note + pitch_bend_as_semitone));
      } else {
        playing_note = 0.0f;
        slewed_freq = mtof(0.0f);
      }
      gate = false;
    }

    volume = params[VOLUME].Process();

    env_out = env.Process(gate);

    osc.SetAmp(env_out);
    subOsc.SetAmp(env_out);

    osc.SetFreq(slewed_freq);

    // Convert the slewed freq back to midi to calc the subosc freq.
    // invert the mtof calc: powf(2, (m - 69.0f) / 12.0f) * 440.0f;
    // TODO: replace with more efficient mechanism.
    float slew_midi = (std::log((slewed_freq / 440.0f))/std::log(2)*12)+69.0f;
    subOsc_freq = mtof(slew_midi - params[SUBOSC_FREQ_DETUNE].Process());

    subOsc.SetFreq(subOsc_freq);

    lfo_freq = params[LFO_FREQ].Process();
    lfo.SetFreq(lfo_freq);

    resonance = params[RESONANCE].Process();
    flt.SetRes(resonance);

    osc_out = osc.Process();
    subOsc_out = subOsc.Process();

    cutoff = 2000.0f;
    cutoff *= exp2f(lfo.Process() * 2.0f);
    flt.SetFreq(cutoff);

    filtered_out = flt.Process(osc_out + subOsc_out) / 2;

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
  hardware.PrintLine("Starting Twiggsynth...");

  InitAnalogControls();
  InitSwitches();

  hardware.SetAudioBlockSize(4);

  InitSynth(hardware.AudioSampleRate());
  InitMidi();

  // let everything settle
	System::Delay(100);

  hardware.adc.Start();

  hardware.StartAudio(AudioCallback);

  ProcessMidi();
}

void ProcessMidi() {
  auto now      = System::GetNow();
  auto log_time = System::GetNow();

  while(1) {
    now = System::GetNow();

    /** Listen to MIDI for new changes */
    midi.Listen();

    /** When there are messages waiting in the queue... */
    while(midi.HasEvents())
    {
      /** Pull the oldest one from the list... */
      MidiEvent msg = midi.PopEvent();
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
        case PitchBend:
        {
          auto bend_msg = msg.AsPitchBend();
          // Normalize pitch bend to 12 semitones.
          pitch_bend_as_semitone = (bend_msg.value / 8192.0f) * 12.0f;
        }
        break;
        // Ignore all other message types
        default: break;
      }

      /** Regardless of message, let's add the message data to our queue to output */
      event_log.PushBack(msg);
    }

    /** Now separately, every 5ms we'll print the top message in our queue if there is one */
    if(now - log_time > 5)
    {
        log_time = now;
        if(!event_log.IsEmpty())
        {
          // Remove message from queue but only log it if logging is enabled. 
          // This allows the queue to be cleaned-up even if logging is disabled.
          MidiEvent msg = event_log.PopFront();

          if (LOG_WRITE_ENABLED) {
            char outstr[128];
            const char* type_str = MidiEvent::GetTypeAsString(msg);
            sprintf(outstr,
                    "time:\t%ld\ttype: %s\tChannel:  %d\tData MSB: "
                    "%d\tData LSB: %d\n",
                    now,
                    type_str,
                    msg.channel,
                    msg.data[0],
                    msg.data[1]);
            hardware.PrintLine(outstr);
          }
        }
    }
  }
}

void InitMidi() {
  MidiUartHandler::Config midi_cfg;
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
  lfo.SetAmp(1.0f);

  flt.Init(samplerate);
  flt.SetRes(0.5);
  flt.SetInputDrive(1.0f);
  flt.SetFilterMode(LadderFilter::FilterMode::LP12);

  env.Init(samplerate);
  env.SetTime(ADSR_SEG_ATTACK, .005);
  env.SetTime(ADSR_SEG_DECAY, .0);
  env.SetTime(ADSR_SEG_RELEASE, .005);
  env.SetSustainLevel(1.0);

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
  for (auto ctl : analogControls)
    ctl.Process();
}

void ProcessDigitalControls()
{
  // None at the moment.
}

void InitSwitches() {
  tripleToggle1.Init(TRIPTOGGLE_1_UP_PIN, TRIPTOGGLE_1_DN_PIN);

  tripleToggles[TRIPTOGGLE_1] = &tripleToggle1;
}
