/*
 * freeverb_stm32.hpp
 *
 *  Created on: Aug 25, 2025
 *      Author: XavSab
 */

#pragma once
// Freeverb minimal optimisé pour STM32F4 (pas de STL, buffers statiques)
// - compile en C++11 ou plus
// - prérequis : -mfpu=fpv4-sp-d16 -mfloat-abi=hard (pour utiliser la FPU)
// - pour réduire la RAM, définir REDUCED_MODE avant inclusion.

#include <cstring> // memset
#include <cmath>   // std::sqrt, facultatif si tu ne l'utilises pas

#define REDUCED_MODE    // décommenter pour version allégée (moins de RAM)

class FreeverbSTM32 {
public:
    FreeverbSTM32() {
        setRoomSize(0.5f);
        setDamp(0.2f);
        setWetDry(0.3f);
        init();
    }

    // call once after allocation (or at startup)
    void init() {
        // clear all buffers and indexes
        std::memset(this, 0, sizeof(*this)); // simple et sûr ici car pas d'objets non-POD
        // set parameters again because memset cleared them
        setRoomSize(roomSize);
        setDamp(damp);
        setWetDry(wet);
        updateFeedback();
    }

    // Réglages
    void setRoomSize(float v) {
        roomSize = clamp01(v);
        updateFeedback();
    }
    void setDamp(float v) { damp = clamp01(v); }
    void setWetDry(float wetRatio) {
        wet = clamp01(wetRatio);
        dry = 1.0f - wet;
    }

    // Traitement par échantillon (float)
    // inL/inR : entrées stéréo (float)
    // outL/outR : sorties stéréo
    inline void process(float inL, float inR, float &outL, float &outR) {
        // Excitation mono (comme dans Freeverb original)
        float input = 0.5f * (inL + inR);

#ifdef REDUCED_MODE
        // Réduction : 4 combs / 2 allpasses par canal
        float accL = 0.0f, accR = 0.0f;
        for (int i = 0; i < 4; ++i) {
            accL += combL[i].process(input, combTuningL[i], damp, feedback);
            accR += combR[i].process(input, combTuningR[i], damp, feedback);
        }
        for (int i = 0; i < 2; ++i) {
            accL = allpassL[i].process(accL, allpassTuningL[i]);
            accR = allpassR[i].process(accR, allpassTuningR[i]);
        }
#else
        // Version complète : 8 combs / 4 allpasses par canal
        float accL = 0.0f, accR = 0.0f;
        for (int i = 0; i < 8; ++i) {
            accL += combL[i].process(input, combTuningL[i], damp, feedback);
            accR += combR[i].process(input, combTuningR[i], damp, feedback);
        }
        for (int i = 0; i < 4; ++i) {
            accL = allpassL[i].process(accL, allpassTuningL[i]);
            accR = allpassR[i].process(accR, allpassTuningR[i]);
        }
#endif

        // mix wet/dry with original input channels
        outL = dry * inL + wet * accL;
        outR = dry * inR + wet * accR;
    }

private:
    // ---------- utilities ----------
    static inline float clamp01(float v) {
        if (v <= 0.0f) return 0.0f;
        if (v >= 1.0f) return 1.0f;
        return v;
    }

    // ---------- Comb filter (structure sans allocation dynamique) ----------
    struct CombStatic {
        // buffer declared as max size per comb instance (size chosen per tuning)
        // We'll allocate each buffer with the exact size in the class by using distinct members.
        float *buf = nullptr; // pointer to buffer (points into the large static arrays below)
        int bufSize = 0;
        int idx = 0;
        float filterstore = 0.0f;

        inline float process(float inp, int size, float damp, float feedback) {
            // size is guaranteed <= bufSize
            float output = buf[idx];
            filterstore = (output * (1.0f - damp)) + (filterstore * damp);
            buf[idx] = inp + filterstore * feedback;
            if (++idx >= size) idx = 0;
            return output;
        }
    };

    // ---------- Allpass filter ----------
    struct AllpassStatic {
        float *buf = nullptr;
        int bufSize = 0;
        int idx = 0;
        float feedback = 0.5f;

        inline float process(float inp, int size) {
            float bufout = buf[idx];
            float output = -inp + bufout;
            buf[idx] = inp + bufout * feedback;
            if (++idx >= size) idx = 0;
            return output;
        }
    };

    // ---------- Delay tunings (original Freeverb-like) ----------
#ifndef REDUCED_MODE
    static constexpr int combTuningL[8] = {1116,1188,1277,1356,1422,1491,1557,1617};
    static constexpr int combTuningR[8] = {1116+23,1188+23,1277+23,1356+23,1422+23,1491+23,1557+23,1617+23};

    static constexpr int allpassTuningL[4] = {556,441,341,225};
    static constexpr int allpassTuningR[4] = {556+23,441+23,341+23,225+23};
#else
    // Reduced sizes (approximate, keep small memory footprint)
    static constexpr int combTuningL[4] = { 400, 470, 530, 590 };
    static constexpr int combTuningR[4] = { 423, 493, 553, 613 }; // +23 offset
    static constexpr int allpassTuningL[2] = { 120, 89 };
    static constexpr int allpassTuningR[2] = { 143, 112 };
#endif

    // ---------- static buffers (declared as raw arrays to avoid heap) ----------
#ifndef REDUCED_MODE
    // For the full version we need to store buffers for 8 combs L/R and 4 allpass L/R
    // We declare flat arrays and then point each CombStatic/AllpassStatic to slices.
    // Sizes computed from tuning arrays above.
    static constexpr int combCount = 8;
    static constexpr int allpassCount = 4;
    // compute sum sizes at compile-time (C++11 limitation: do it manually)
    // We reserve the max size per comb as the largest tuning + 1 (safety)
    static constexpr int MAX_COMB_SIZE = 1645; // just above largest (1617+23)
    static constexpr int MAX_ALLPASS_SIZE = 600; // above 579
    // Allocate contiguous memory for simplicity:
    float combBufStorageL[combCount * MAX_COMB_SIZE];
    float combBufStorageR[combCount * MAX_COMB_SIZE];
    float allpassBufStorageL[allpassCount * MAX_ALLPASS_SIZE];
    float allpassBufStorageR[allpassCount * MAX_ALLPASS_SIZE];
#else
    static constexpr int combCount = 4;
    static constexpr int allpassCount = 2;
    static constexpr int MAX_COMB_SIZE = 640;
    static constexpr int MAX_ALLPASS_SIZE = 160;
    float combBufStorageL[combCount * MAX_COMB_SIZE];
    float combBufStorageR[combCount * MAX_COMB_SIZE];
    float allpassBufStorageL[allpassCount * MAX_ALLPASS_SIZE];
    float allpassBufStorageR[allpassCount * MAX_ALLPASS_SIZE];
#endif

    // ---------- Comb and Allpass instances ----------
    CombStatic combL[combCount];
    CombStatic combR[combCount];
    AllpassStatic allpassL[allpassCount];
    AllpassStatic allpassR[allpassCount];

    // ---------- parameters ----------
    float roomSize = 0.5f;
    float damp = 0.2f;
    float feedback = 0.0f;
    float wet = 0.3f;
    float dry = 0.7f;

    // ---------- helpers ----------
    void updateFeedback() {
        // freeverb-ish mapping
        feedback = roomSize * 0.28f + 0.7f;
    }

    // set up pointers into the storage arrays and zero them
    // must be called once (or after a memset that cleared pointers)
    void setupBuffers() {
#ifndef REDUCED_MODE
        // clear storages
        std::memset(combBufStorageL, 0, sizeof(combBufStorageL));
        std::memset(combBufStorageR, 0, sizeof(combBufStorageR));
        std::memset(allpassBufStorageL, 0, sizeof(allpassBufStorageL));
        std::memset(allpassBufStorageR, 0, sizeof(allpassBufStorageR));

        for (int i = 0; i < combCount; ++i) {
            combL[i].buf = &combBufStorageL[i * MAX_COMB_SIZE];
            combL[i].bufSize = MAX_COMB_SIZE;
            combR[i].buf = &combBufStorageR[i * MAX_COMB_SIZE];
            combR[i].bufSize = MAX_COMB_SIZE;
        }
        for (int i = 0; i < allpassCount; ++i) {
            allpassL[i].buf = &allpassBufStorageL[i * MAX_ALLPASS_SIZE];
            allpassL[i].bufSize = MAX_ALLPASS_SIZE;
            allpassR[i].buf = &allpassBufStorageR[i * MAX_ALLPASS_SIZE];
            allpassR[i].bufSize = MAX_ALLPASS_SIZE;
        }
#else
        std::memset(combBufStorageL, 0, sizeof(combBufStorageL));
        std::memset(combBufStorageR, 0, sizeof(combBufStorageR));
        std::memset(allpassBufStorageL, 0, sizeof(allpassBufStorageL));
        std::memset(allpassBufStorageR, 0, sizeof(allpassBufStorageR));

        for (int i = 0; i < combCount; ++i) {
            combL[i].buf = &combBufStorageL[i * MAX_COMB_SIZE];
            combL[i].bufSize = MAX_COMB_SIZE;
            combR[i].buf = &combBufStorageR[i * MAX_COMB_SIZE];
            combR[i].bufSize = MAX_COMB_SIZE;
        }
        for (int i = 0; i < allpassCount; ++i) {
            allpassL[i].buf = &allpassBufStorageL[i * MAX_ALLPASS_SIZE];
            allpassL[i].bufSize = MAX_ALLPASS_SIZE;
            allpassR[i].buf = &allpassBufStorageR[i * MAX_ALLPASS_SIZE];
            allpassR[i].bufSize = MAX_ALLPASS_SIZE;
        }
#endif
    }

    // ensure buffers are setup before first use
    // call from init() if you used memset(this,..) in init
    // we call it lazily on first process (simple)
    bool buffersInitialized = false;
    inline void ensureBuffers() {
        if (!buffersInitialized) {
            setupBuffers();
            buffersInitialized = true;
        }
    }
};
