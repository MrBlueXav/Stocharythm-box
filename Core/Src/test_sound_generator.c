/*-----------------------------------------------------------------
 * test_sound_generator.c
 *
 *  Created on: Nov 9, 2023
 *      Author: Xavier Halgand
 *---------------------------------------------------------------------*/

/***************************** Very simple test sound functions **********************************/
/************************************************************************************************/


#include "test_sound_generator.h"
#include "constants.h"
#include <math.h>

/*----------------------------------------------------------------------------------------------*/
typedef struct {
	float amp;	// should be <= 1 for normal sound output
	float last_amp;
	float freq;	// Hertz
	float phase;	// radians
	float phi0;	// radians
	float modInd;	// Modulation Index for FM
	float mul;	// pitch frequency multiplier
	float out;	// output sample in [-1, 1]

} Oscillator_t0;

/*----------------------------------------------------------------------------------------------*/
static Oscillator_t0 vibr_lfo;
static Oscillator_t0 oscillo;

/*----------------------------------------------------------------------------------------------*/
static void osc_init0(Oscillator_t0 *op, float amp, float freq) {
	op->amp = amp;
	op->last_amp = amp;
	op->freq = freq;
	op->phase = 0;
	op->out = 0;
	op->modInd = 0;
	op->mul = 1;
}
/*----------------------------------------------------------------------------------------------*/
static void OpSetFreq0(Oscillator_t0 *op, float f) {
	op->freq = f;
}

/*----------------------------------------------------------------------------------------------*/
static float OpSampleCompute0(Oscillator_t0 *op) // accurate sine waveform
{
	float z;

	while (op->phase >= _2PI) // keep phase in [0, 2pi]
		op->phase -= _2PI;

	z = sinf(op->phase);
	op->out = op->amp * z;

	op->phase += _2PI * Ts * op->freq; // increment phase
	return op->out;
}

/*----------------------------------------------------------------------------------------------*/
void soundGeneratorInit(void)
{
	osc_init0(&oscillo, 0.9, 440);
	osc_init0(&vibr_lfo, 0.1, 4);
}

/*-------------------------------------------------------------------
 * buf : audio buffer pointer which contains frames. One frame is one left 16 bits sample + one right 16 bits sample (32 bits)
 * lenght : number of frames to be computed
 *
 * ---------------------------*/
void make_test_sound0(uint16_t *buf, uint16_t length) //
{

	uint16_t pos;
	uint16_t *outp;
	float y = 0;
	float yL, yR;
	float f1;
	uint16_t valueL, valueR;

	outp = buf;

	for (pos = 0; pos < length; pos++) {

		/*--- Generate waveform ---*/
		/*--- compute vibrato modulation ---*/
		f1 = 440 * (1 + OpSampleCompute0(&vibr_lfo));
		OpSetFreq0(&oscillo, f1);
		y = OpSampleCompute0(&oscillo);

		yL = yR = y;

		/*--- clipping ---*/
		yL = (yL > 1.0f) ? 1.0f : yL; //clip too loud left samples
		yL = (yL < -1.0f) ? -1.0f : yL;

		yR = (yR > 1.0f) ? 1.0f : yR; //clip too loud right samples
		yR = (yR < -1.0f) ? -1.0f : yR;

		/****** let's hear the new sample *******/

		valueL = (uint16_t) ((int16_t) ((32767.0f) * yL)); // conversion float -> int
		valueR = (uint16_t) ((int16_t) ((32767.0f) * yR));

		*outp++ = valueL; // left channel sample
		*outp++ = valueR; // right channel sample
	}

}
