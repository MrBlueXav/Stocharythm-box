/*
 * stereo.cpp
 *
 *  Created on: Aug 20, 2025
 *      Author: Xavier Halgand
 */

#include "stereo.hpp"


/******************************************************************************************************/
/// Panoramique stéréo optimisé microcontrôleur
/// @param in   : signal d'entrée mono
/// @param pan  : -1.0 = gauche, 0.0 = centre, +1.0 = droite
/// @param outL, outR : sorties stéréo
void panStereoFast(float in, float pan, float &outL, float &outR) {
    // Clamp entre -1 et 1
    if (pan < -1.0f) pan = -1.0f;
    if (pan >  1.0f) pan =  1.0f;

    // pan [-1..1] → mix [0..1]
    float mix = (pan + 1.0f) * 0.5f;

    // Loi de panoramique à puissance constante
    float gainL = std::sqrt(1.0f - mix);
    float gainR = std::sqrt(mix);

    outL = in * gainL;
    outR = in * gainR;
}

/// Fonction de panoramique stéréo (constant power)
/// @param in  : signal d'entrée mono
/// @param pan : -1.0 = gauche, 0.0 = centre, +1.0 = droite
/// @param outL, outR : sorties stéréo
void panStereo(float in, float pan, float &outL, float &outR) {
    // Clamp entre -1 et 1
    if (pan < -1.0f) pan = -1.0f;
    if (pan >  1.0f) pan =  1.0f;

    // Convertit pan [-1..1] en angle [0..pi/2]
    float angle = (pan + 1.0f) * 0.25f * M_PI;

    float gainL = std::cos(angle); // gauche diminue avec le pan
    float gainR = std::sin(angle); // droite augmente avec le pan

    outL = in * gainL;
    outR = in * gainR;
}


/// Mixeur stéréo avec panning constant power
/// Entrées : liste de canaux
/// Sorties : outL, outR
void mixStereo(const InputChannel *inputs, int count,
                      float &outL, float &outR)
{
    outL = 0.0f;
    outR = 0.0f;

    for (int i = 0; i < count; ++i) {
        float pan = inputs[i].pan;
        if (pan < -1.0f) pan = -1.0f;
        if (pan >  1.0f) pan =  1.0f;

        float mix = (pan + 1.0f) * 0.5f;
        float gainL = std::sqrt(1.0f - mix);
        float gainR = std::sqrt(mix);

        outL += inputs[i].sample * inputs[i].gain * gainL;
        outR += inputs[i].sample * inputs[i].gain * gainR;
    }
}
