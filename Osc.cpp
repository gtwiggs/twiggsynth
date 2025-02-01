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

#include "daisy_seed.h"
#include "daisysp.h"
// #include "Filters/moogladder.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
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
static Oscillator osc, lfo;
static MoogLadder flt;
static Parameter  p_vco_freq;
static Parameter  p_lfo_freq;
static Parameter  p_volume;

AnalogControl knob1,
    knob2,
    knob3,
    *knobs[KNOB_LAST];

Switch3 tripleToggle1,
    *tripleToggles[TRIPTOGGLE_LAST];

void ProcessAnalogControls();
void ProcessDigitalControls();
void InitSynth(float samplerate);
void InitKnobs();
void InitSwitches();
float GetKnobValue(Knob k);

/** Process Analog and Digital Controls */
inline void ProcessAllControls()
{
  ProcessAnalogControls();
  ProcessDigitalControls();
}


void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
  float vco_freq, lfo_freq, filt_freq, osc_out, filtered_out, volume;

  ProcessAllControls();

  if (tripleToggle1.Read() == Switch3::POS_UP) {
    volume = 1 - p_volume.Process();
  } else {
    volume = 0;
  }

  vco_freq = mtof(127 - p_vco_freq.Process());
  osc.SetFreq(vco_freq);

  lfo_freq = 20 - p_lfo_freq.Process();
  lfo.SetFreq(lfo_freq);

  //Fill the block with samples
  for(size_t i = 0; i < size; i += 2)
  {
    osc_out = osc.Process();

    filt_freq = 5000 + (lfo.Process() * 5000);
    flt.SetFreq(filt_freq);

    filtered_out = flt.Process(osc_out);

    //Set the left and right outputs
    out[i]     = filtered_out * volume;
    out[i + 1] = filtered_out * volume;
  }
}

int main(void)
{
  hardware.Init();

  InitKnobs();
  InitSwitches();

  hardware.SetAudioBlockSize(4);

  InitSynth(hardware.AudioSampleRate());

  hardware.adc.Start();

  hardware.StartAudio(AudioCallback);

  while(1) {}
}

void InitSynth(float samplerate)
{
  // Init freq Parameter to knob1 using MIDI note numbers
  // min 10, max 127, curve linear
  p_vco_freq.Init(knob3, 0, 127, Parameter::LINEAR);
  p_lfo_freq.Init(knob2, 0, 20, Parameter::LINEAR);
  p_volume.Init(knob1, 0, 1, Parameter::LINEAR);

  osc.Init(samplerate);
  osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
  osc.SetAmp(1.f);
  osc.SetFreq(1000);

  lfo.Init(samplerate);
  lfo.SetWaveform(osc.WAVE_POLYBLEP_TRI);
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
