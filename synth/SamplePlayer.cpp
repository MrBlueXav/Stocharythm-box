/*
 * FlashWavPlayer.cpp
 *
 *  Created on: Aug 18, 2025
 *      Author: Xavier Halgand
 */

#include "SamplePlayer.h"

//=======================================================================

SamplePlayer::SamplePlayer() {

	sr_ = 48000.f;
	index_ = 0;
	len_ = 0;
	smp_buf_ = smp_ = nullptr;
	amp_ = 1.f;
	trigged_ = false;
}

//--------------------------------------------------------------------------
void SamplePlayer::Init(float sample_rate, const int16_t *samplebuffer,
		uint32_t length) {

	sr_ = sample_rate;
	smp_buf_ = samplebuffer;
	smp_ = samplebuffer;
	len_ = length;
	index_ = 0;
	amp_ = 1.f;
	trigged_ = false;

}

//--------------------------------------------------------------------------
/** Get the next sample. */
float SamplePlayer::Process() {

	if (trigged_ == true) {
		auto out = amp_ * (*smp_) / 32768.f;
		if (index_ >= (len_ - 1)) {
			index_ = 0;
			smp_ = smp_buf_;
			trigged_ = false;
		} else {
			index_++;
			smp_++;
		}
		return out;
	} else
		return 0.f;

}

//--------------------------------------------------------------------------
/** Trigger the drum */
void SamplePlayer::Trig() {

	index_ = 0;
	smp_ = smp_buf_;
	trigged_ = true;

}

//--------------------------------------------------------------------------
void SamplePlayer::SetAmp(float amp) {

	amp_ = amp;
}

//=======================================================================
