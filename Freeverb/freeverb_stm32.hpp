/*
 * freeverb_stm32.hpp
 *
 *  Created on: Aug 25, 2025
 *      Author: Jezar + ChatGPT
 */

#pragma once
// Freeverb stéréo "réaliste" pour STM32F4 (Cortex-M4F)
// - Pas de STL / allocations dynamiques
// - L/R décorrelés (+23 samples) + crossfeed via 'width'
// - Paramètres : wet/dry, roomSize, damp, width
// Flags conseillés : -O3 -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard

#include <cstring>
#include <cstdint>

// Décommente pour une version plus compacte en RAM (légère perte de densité)
#define FREEVERB_REDUCED

#define FREEVERB_ANTI_DENORMAL 1

class FreeverbStereoSTM32 {
public:
    FreeverbStereoSTM32() { defaultParams(); setupBuffers(); clearState(); }

    void init() {                      // à appeler si tu veux réinitialiser
        defaultParams();
        setupBuffers();
        clearState();
    }

    // Réglages
    void setWetDry(float wetRatio) { wet = clamp01(wetRatio); dry = 1.0f - wet; updateWetMix(); }
    void setRoomSize(float v)      { roomSize = clamp01(v); updateFeedback(); }
    void setDamp(float v)          { damp = clamp01(v); }
    void setWidth(float v)         { width = clamp01(v); updateWetMix(); }  // 0=mono, 1=large

    // Traitement par échantillon
    inline void process(float inL, float inR, float &outL, float &outR) {

#if FREEVERB_ANTI_DENORMAL
        static uint32_t seed = 0x12345678u;
        seed = 1664525u * seed + 1013904223u;
        const float anti = ((seed >> 9) & 1) ? 1.0e-20f : -1.0e-20f;
#else
        const float anti = 0.0f;
#endif
        // Excitation mono (comme Freeverb)
        const float input = 0.5f * (inL + inR) + anti;

        float accL = 0.0f, accR = 0.0f;

        // 8 (ou 4 en reduced) peignes par canal
        for (int i = 0; i < COMB_COUNT; ++i) {
            accL += combL[i].process(input, damp, feedback);
            accR += combR[i].process(input, damp, feedback);
        }
        // 4 (ou 2) allpass en série
        for (int i = 0; i < ALLPASS_COUNT; ++i) {
            accL = allpassL[i].process(accL);
            accR = allpassR[i].process(accR);
        }

        // Sortie : crossfeed contrôlé par width (wet1/wet2, modèle Freeverb)
        // wet1 = wet*(width/2 + 0.5), wet2 = wet*((1-width)/2)
        const float wetL = wet1 * accL + wet2 * accR;
        const float wetR = wet1 * accR + wet2 * accL;

        // Pad léger pour éviter clipping si wet/dry élevés
        const float pad = 0.95f;
        outL = dry * inL + pad * wetL;
        outR = dry * inR + pad * wetR;
    }

    // Traitement par bloc (recommandé en DMA/I2S)
    inline void processBlock(const float* inL, const float* inR,
                             float* outL, float* outR, int n) {
        for (int i = 0; i < n; ++i) process(inL[i], inR[i], outL[i], outR[i]);
    }

private:
#ifndef FREEVERB_REDUCED
    // Tunings Freeverb originaux (L) + décalage R = +23 samples
    static constexpr int COMB_COUNT    = 8;
    static constexpr int ALLPASS_COUNT = 4;

    static constexpr int combTuningL[COMB_COUNT]     = {1116,1188,1277,1356,1422,1491,1557,1617};
    static constexpr int combTuningR[COMB_COUNT]     = {1116+23,1188+23,1277+23,1356+23,1422+23,1491+23,1557+23,1617+23};

    static constexpr int allpassTuningL[ALLPASS_COUNT]= {556,441,341,225};
    static constexpr int allpassTuningR[ALLPASS_COUNT]= {556+23,441+23,341+23,225+23};
#else
    // Version réduite RAM (reste stéréo/décorrelée)
    static constexpr int COMB_COUNT    = 4;
    static constexpr int ALLPASS_COUNT = 2;

    static constexpr int combTuningL[COMB_COUNT]      = {400,470,530,590};
    static constexpr int combTuningR[COMB_COUNT]      = {423,493,553,613};
    static constexpr int allpassTuningL[ALLPASS_COUNT]= {120, 89};
    static constexpr int allpassTuningR[ALLPASS_COUNT]= {143,112};
#endif

    struct Comb {
        float* buf; int size; int idx; float filterStore;
        inline void attach(float* b, int s){ buf=b; size=s; idx=0; filterStore=0.0f; }
        inline float process(float x, float damp, float feedback){
            const float y = buf[idx];
            // lowpass dans la boucle de feedback (damp)
            filterStore = y * (1.0f - damp) + filterStore * damp;
            buf[idx] = x + filterStore * feedback;
            if (++idx >= size) idx = 0;
            return y;
        }
    };
    struct Allpass {
        float* buf; int size; int idx; static constexpr float fb = 0.5f;
        inline void attach(float* b, int s){ buf=b; size=s; idx=0; }
        inline float process(float x){
            const float w = buf[idx];
            const float y = -x + w;
            buf[idx] = x + w * fb;
            if (++idx >= size) idx = 0;
            return y;
        }
    };

    // Sommes exactes pour allouer au plus juste
#ifndef FREEVERB_REDUCED
    static constexpr int sumCombL = combTuningL[0]+combTuningL[1]+combTuningL[2]+combTuningL[3]
                                  + combTuningL[4]+combTuningL[5]+combTuningL[6]+combTuningL[7];
    static constexpr int sumCombR = combTuningR[0]+combTuningR[1]+combTuningR[2]+combTuningR[3]
                                  + combTuningR[4]+combTuningR[5]+combTuningR[6]+combTuningR[7];
    static constexpr int sumAllL  = allpassTuningL[0]+allpassTuningL[1]+allpassTuningL[2]+allpassTuningL[3];
    static constexpr int sumAllR  = allpassTuningR[0]+allpassTuningR[1]+allpassTuningR[2]+allpassTuningR[3];
#else
    static constexpr int sumCombL = combTuningL[0]+combTuningL[1]+combTuningL[2]+combTuningL[3];
    static constexpr int sumCombR = combTuningR[0]+combTuningR[1]+combTuningR[2]+combTuningR[3];
    static constexpr int sumAllL  = allpassTuningL[0]+allpassTuningL[1];
    static constexpr int sumAllR  = allpassTuningR[0]+allpassTuningR[1];
#endif

    // Stockage brut (en .bss)
    float combBufL[sumCombL];
    float combBufR[sumCombR];
    float allpassBufL[sumAllL];
    float allpassBufR[sumAllR];

    // Instances
    Comb    combL[COMB_COUNT];
    Comb    combR[COMB_COUNT];
    Allpass allpassL[ALLPASS_COUNT];
    Allpass allpassR[ALLPASS_COUNT];

    // Paramètres
    float roomSize, damp, feedback;
    float wet, dry, width;   // width contrôle le crossfeed
    float wet1, wet2;        // mix L/R stéréo dérivés de wet & width

    // Helpers
    static inline float clamp01(float v){ return v<0.f?0.f:(v>1.f?1.f:v); }
    inline void updateFeedback(){ feedback = roomSize * 0.28f + 0.70f; }
    inline void updateWetMix(){
        // Modèle Freeverb : wet1 = wet*(width/2 + 0.5), wet2 = wet*((1-width)/2)
        wet1 = wet * (0.5f * width + 0.5f);
        wet2 = wet * (0.5f * (1.0f - width));
    }
    inline void defaultParams(){
        roomSize = 0.5f; damp = 0.2f; wet = 0.3f; dry = 0.7f; width = 1.0f;
        updateFeedback(); updateWetMix();
    }
    inline void clearState(){
        std::memset(combBufL,    0, sizeof(combBufL));
        std::memset(combBufR,    0, sizeof(combBufR));
        std::memset(allpassBufL, 0, sizeof(allpassBufL));
        std::memset(allpassBufR, 0, sizeof(allpassBufR));
        for (int i=0;i<COMB_COUNT;i++){ combL[i].idx=0; combL[i].filterStore=0.0f;
                                        combR[i].idx=0; combR[i].filterStore=0.0f; }
        for (int i=0;i<ALLPASS_COUNT;i++){ allpassL[i].idx=0; allpassR[i].idx=0; }
    }
    inline void setupBuffers(){
        int off = 0;
        for (int i=0;i<COMB_COUNT;i++){ combL[i].attach(combBufL + off, combTuningL[i]); off += combTuningL[i]; }
        off = 0;
        for (int i=0;i<COMB_COUNT;i++){ combR[i].attach(combBufR + off, combTuningR[i]); off += combTuningR[i]; }
        off = 0;
        for (int i=0;i<ALLPASS_COUNT;i++){ allpassL[i].attach(allpassBufL + off, allpassTuningL[i]); off += allpassTuningL[i]; }
        off = 0;
        for (int i=0;i<ALLPASS_COUNT;i++){ allpassR[i].attach(allpassBufR + off, allpassTuningR[i]); off += allpassTuningR[i]; }
    }
};
