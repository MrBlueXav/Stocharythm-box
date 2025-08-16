
#pragma once
#include <array>
#include <cmath>

class MiniFreeverb {

public:

	MiniFreeverb(float sampleRate = 48000.0f) {
		setRoomSize(0.7f);
		setDamp(0.2f);
		setGain(0.015f);
		clearBuffers();
	}

	void setRoomSize(float value) { roomSize = value; updateFeedback(); }
	void setDamp(float value) { damp = value; }
	void setGain(float value) { gain = value; }

	float process(float input) {
		float out = 0.0f;

		// Combs
		out += combProcess(input, combBuffer1, combIdx[0], combSize[0], filterStore[0]);
		out += combProcess(input, combBuffer2, combIdx[1], combSize[1], filterStore[1]);
		out += combProcess(input, combBuffer3, combIdx[2], combSize[2], filterStore[2]);
		out += combProcess(input, combBuffer4, combIdx[3], combSize[3], filterStore[3]);

		// Allpasses
		out = allpassProcess(out, allpassBuffer1, allpassIdx[0], allpassSize[0]);
		out = allpassProcess(out, allpassBuffer2, allpassIdx[1], allpassSize[1]);

		return out * gain;
	}

	void clearBuffers() {
		combBuffer1.fill(0.0f);
		combBuffer2.fill(0.0f);
		combBuffer3.fill(0.0f);
		combBuffer4.fill(0.0f);
		allpassBuffer1.fill(0.0f);
		allpassBuffer2.fill(0.0f);
		for (int i = 0; i < numCombs; ++i) {
			combIdx[i] = 0;
			filterStore[i] = 0.0f;
		}
		for (int i = 0; i < numAllpasses; ++i) {
			allpassIdx[i] = 0;
		}
	}

private:
	static constexpr int numCombs = 4;
	static constexpr int numAllpasses = 2;

	static constexpr int combSize[numCombs] = { 1116, 1188, 1277, 1356 };
	static constexpr int allpassSize[numAllpasses] = { 556, 441 };

	std::array<float, combSize[0]> combBuffer1;
	std::array<float, combSize[1]> combBuffer2;
	std::array<float, combSize[2]> combBuffer3;
	std::array<float, combSize[3]> combBuffer4;

	std::array<float, allpassSize[0]> allpassBuffer1;
	std::array<float, allpassSize[1]> allpassBuffer2;

	int combIdx[numCombs];
	int allpassIdx[numAllpasses];
	float filterStore[numCombs];

	float roomSize = 0.5f;
	float damp = 0.2f;
	float feedback = 0.0f;
	float gain = 0.015f;

	void updateFeedback() {
		feedback = roomSize * 0.28f + 0.7f;
	}

	template <size_t N>
	float combProcess(float input, std::array<float, N>& buf, int& idx, int size, float& store) {
		float output = buf[idx];
		store = (output * (1 - damp)) + (store * damp);
		buf[idx] = input + (store * feedback);
		if (++idx >= size) idx = 0;
		return output;
	}

	template <size_t N>
	float allpassProcess(float input, std::array<float, N>& buf, int& idx, int size) {
		float bufout = buf[idx];
		float output = -input + bufout;
		buf[idx] = input + (bufout * 0.5f);
		if (++idx >= size) idx = 0;
		return output;
	}
};
