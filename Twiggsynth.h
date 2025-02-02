void ProcessAnalogControls();
void ProcessDigitalControls();
void InitSynth(float samplerate);
void InitKnobs();
void InitSwitches();

/** Process Analog and Digital Controls */
inline void ProcessAllControls()
{
  ProcessAnalogControls();
  ProcessDigitalControls();
}
