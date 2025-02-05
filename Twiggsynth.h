#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

enum AnalogControlName : unsigned int;

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

void InitMidi();
void InitSynth(float samplerate);
void InitAnalogControls();
void InitSwitches();

void ProcessAnalogControls();
void ProcessDigitalControls();
void ProcessMidi();

/** Process Analog and Digital Controls */
inline void ProcessAllControls()
{
  ProcessAnalogControls();
  ProcessDigitalControls();
}
