#pragma once
#include <array>
#include <cmath>

class FreeverbStereo {
public:
    FreeverbStereo(float sampleRate = 48000.0f) {
        setRoomSize(0.5f);
        setDamp(0.2f);
        setGain(0.015f);
        clearBuffers();
    }

    void setRoomSize(float value) { roomSize = value; updateFeedback(); }
    void setDamp(float value) { damp = value; }
    void setGain(float value) { gain = value; }

    /// Traitement stéréo
    void process(float inL, float inR, float &outL, float &outR) {
        outL = processChannel(inL, true);
        outR = processChannel(inR, false);
    }

    void clearBuffers() {
        clearChannel(left);
        clearChannel(right);
    }

private:
    // ===================
    // Paramètres internes
    // ===================
    static constexpr int numCombs = 4;
    static constexpr int numAllpasses = 2;

    // Délais pour chaque canal (légèrement différents)
    static constexpr int combSizeL[numCombs] = {1116, 1188, 1277, 1356};
    static constexpr int combSizeR[numCombs] = {1119, 1211, 1267, 1379};

    static constexpr int allpassSizeL[numAllpasses] = {556, 441};
    static constexpr int allpassSizeR[numAllpasses] = {579, 464};

    float roomSize = 0.5f;
    float damp = 0.2f;
    float feedback = 0.0f;
    float gain = 0.015f;

    // ===================
    // Structure interne
    // ===================
    struct Channel {
        std::array<float, combSizeL[0]> comb1;
        std::array<float, combSizeL[1]> comb2;
        std::array<float, combSizeL[2]> comb3;
        std::array<float, combSizeL[3]> comb4;
        std::array<float, allpassSizeL[0]> allpass1;
        std::array<float, allpassSizeL[1]> allpass2;

        int combIdx[numCombs]{};
        int allpassIdx[numAllpasses]{};
        float filterStore[numCombs]{};
    };

    Channel left;
    Channel right;

    // ===================
    // Fonctions internes
    // ===================
    void updateFeedback() {
        feedback = roomSize * 0.28f + 0.7f;
    }

    void clearChannel(Channel &ch) {
        ch.comb1.fill(0.0f);
        ch.comb2.fill(0.0f);
        ch.comb3.fill(0.0f);
        ch.comb4.fill(0.0f);
        ch.allpass1.fill(0.0f);
        ch.allpass2.fill(0.0f);
        for (int i = 0; i < numCombs; ++i) {
            ch.combIdx[i] = 0;
            ch.filterStore[i] = 0.0f;
        }
        for (int i = 0; i < numAllpasses; ++i) {
            ch.allpassIdx[i] = 0;
        }
    }

    // Traitement d’un canal
    float processChannel(float input, bool isLeft) {
        float out = 0.0f;
        Channel &ch = isLeft ? left : right;

        // Sélection des tailles selon canal
        const int *combSize = isLeft ? combSizeL : combSizeR;
        const int *allpassSize = isLeft ? allpassSizeL : allpassSizeR;

        // Combs
        out += combProcess(input, ch.comb1, ch.combIdx[0], combSize[0], ch.filterStore[0]);
        out += combProcess(input, ch.comb2, ch.combIdx[1], combSize[1], ch.filterStore[1]);
        out += combProcess(input, ch.comb3, ch.combIdx[2], combSize[2], ch.filterStore[2]);
        out += combProcess(input, ch.comb4, ch.combIdx[3], combSize[3], ch.filterStore[3]);

        // Allpasses
        out = allpassProcess(out, ch.allpass1, ch.allpassIdx[0], allpassSize[0]);
        out = allpassProcess(out, ch.allpass2, ch.allpassIdx[1], allpassSize[1]);

        return out * gain;
    }

    // Comb filter
    template <size_t N>
    float combProcess(float input, std::array<float, N> &buf, int &idx, int size, float &store) {
        float output = buf[idx];
        store = (output * (1 - damp)) + (store * damp);
        buf[idx] = input + (store * feedback);
        if (++idx >= size) idx = 0;
        return output;
    }

    // Allpass filter
    template <size_t N>
    float allpassProcess(float input, std::array<float, N> &buf, int &idx, int size) {
        float bufout = buf[idx];
        float output = -input + bufout;
        buf[idx] = input + (bufout * 0.5f);
        if (++idx >= size) idx = 0;
        return output;
    }
};
