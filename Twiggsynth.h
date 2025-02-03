void InitMidi();
void InitSynth(float samplerate);
void InitKnobs();
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
