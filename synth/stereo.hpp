/*
 * stereo.hpp
 *
 *  Created on: Aug 20, 2025
 *      Author: Xavier Halgand
 */

#ifndef INC_STEREO_HPP_
#define INC_STEREO_HPP_

#include <cmath>

struct InputChannel {
    float sample; // valeur audio mono
    float gain;   // volume [0..1]
    float pan;    // -1=gauche, 0=centre, +1=droite
};


void panStereoFast(float in, float pan, float &outL, float &outR);
void panStereo(float in, float pan, float &outL, float &outR);
void mixStereo(const InputChannel *inputs, int count, float &outL, float &outR);


#endif /* INC_STEREO_HPP_ */
