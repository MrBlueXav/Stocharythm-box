/*
 * FlashWavPlayer.h
 *
 *  Created on: Aug 18, 2025
 *      Author: Xavier Halgand
 */

#ifndef SRC_FLASHWAVPLAYER_H_
#define SRC_FLASHWAVPLAYER_H_

#include <cstdint>

class FlashWavPlayer {
public:
	FlashWavPlayer();
	~FlashWavPlayer() {};

	/** Init the module	 */
	void Init(float sample_rate, const int16_t *samplebuffer, uint32_t length);

	/** Get the next sample. */
	float Process();

	/** Trigger the drum */
	void Trig();

	void SetAmp(float amp);

private:
	float sr_;
	const int16_t *smp_buf_;
	const int16_t *smp_;
	uint32_t len_;
	uint32_t index_;
	float amp_;
	bool trigged_;
};

#endif /* SRC_FLASHWAVPLAYER_H_ */
