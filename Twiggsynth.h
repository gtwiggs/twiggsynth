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

/**
 * PitchMultiplier returns a frequency multiplier based on an offset and a range in semitones.
 * The offset is a value between 0 and 1.
 */
float PitchMultiplier(float offset, int semitoneRange);

/**
 * @brief Map a float value to an int range.
 * @param val The float value to map. Must be in the range 0.0 to 1.0 inclusive.
 * @param minInt The minimum int value.
 * @param maxInt The maximum int value.
 * @return The mapped int value.
 */
int MapControlToRange(float val, int minInt, int maxInt);

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
