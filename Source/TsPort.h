/*
Copyright (c) 2023 Electrosmith, Corp, Robbin Whittle, John ffitch, Paul Batchelor, Glenn Twiggs

Use of this source code is governed by the LGPL V2.1
license that can be found in the LICENSE file or at
https://opensource.org/license/lgpl-2-1/
*/
#ifndef TSPORT_H
#define TSPORT_H

namespace daisysp
{
/** Applies portamento to an input signal. 

At each new step value, the input is low-pass filtered to 
move towards that value at a rate determined by ihtim. ihtim is the half-time of the 
function (in seconds), during which the curve will traverse half the distance towards the new value, 
then half as much again, etc., theoretically never reaching its asymptote.

Modified by @gtwiggs to include the SetFreq method.
*/
class TsPort
{
  public:
    TsPort() {}
    ~TsPort() {}
    /** Initializes Port module

        \param sample_rate: sample rate of audio engine
        \param htime: half-time of the function, in seconds.
    */

    void Init(float sample_rate, float htime);

    /** Applies portamento to input signal and returns processed signal. 
        \return slewed output signal
    */
    float Process(float in);


    /** Sets htime
    */
    inline void SetHtime(float htime) { htime_ = htime; }
    /** returns current value of htime
    */
    inline float GetHtime() { return htime_; }

    /**
     * Sets the starting point frequency.
     * Useful for "resetting" the frequency glide to the current note.
     * 
     * \param f: frequency to set the starting point for portamento.
     */
    void SetFreq(float f);

  private:
    float htime_;
    float c1_, c2_, yt1_, prvhtim_;
    float sample_rate_, onedsr_;
};
} // namespace daisysp

#endif // TSPORT_H