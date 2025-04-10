#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

enum AnalogControlName : unsigned int;

struct AnalogControlDefn
{
  AnalogControlName name;
  Pin               pin;
  float             min;
  float             max;
  Parameter::Curve  curve;
  bool              flipped;
  float             slew_seconds;

  // Constructor
  AnalogControlDefn(AnalogControlName name,
                    Pin               pin,
                    float             min,
                    float             max,
                    Parameter::Curve  curve,
                    bool              flipped,
                    float             slew_seconds = 0.0f)
  : name(name),
    pin(pin),
    min(min),
    max(max),
    curve(curve),
    flipped(flipped),
    slew_seconds(slew_seconds)
  {
  }
};

void InitMidi();
void InitSynth(float samplerate);
void InitAnalogControls();
void InitDigitalControls(float samplerate);

void ProcessAnalogControls();
void ProcessDigitalControls();
void ProcessMidi();

/** Process Analog and Digital Controls */
inline void ProcessAllControls()
{
  ProcessAnalogControls();
  ProcessDigitalControls();
}
