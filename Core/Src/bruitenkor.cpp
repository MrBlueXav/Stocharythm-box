/*
 * bruitenkor.cpp
 *
 *  Created on: Aug 5, 2025
 *      Author: Xavier Halgand
 */

#include "bruitenkor.h"
#include "constants.h"
#include <math.h>
#include "daisysp.h"

using namespace daisysp;

enum Source
{
	NONE, WH_NOISE, PARTICLE, GRAIN_OSC, DUST, CK_NOISE, END
};

static WhiteNoise _CCM_ w_noise;
static Particle _CCM_ particle;
static GrainletOscillator _CCM_ gr_osc;
static Dust _CCM_ dust;
static ClockedNoise _CCM_ ck_noise;
static Adsr _CCM_ adsr1;
static Adsr _CCM_ adsr2;
static Svf _CCM_ filter;

static float sample_rate = SAMPLERATE;
static Source source = NONE;

/*----------------------------------------------------------------------------------------------*/
extern "C" void InterpretKey(uint8_t key)
{
	switch (key)
	{
	case '0':
		source = NONE;
		break;

	case '1':
		source = WH_NOISE;
		break;

	case '2':
		source = PARTICLE;
		break;

	case '3':
		source = GRAIN_OSC;
		break;

	case '4':
		source = DUST;
		break;

	case '5':
		source = CK_NOISE;
		break;

	default:
		break;
	}
}
/*----------------------------------------------------------------------------------------------*/
extern "C" void SoundGeneratorInit(void)
{
	w_noise.Init();
	particle.Init(sample_rate);
	gr_osc.Init(sample_rate);
	gr_osc.SetFreq(110.f);
	gr_osc.SetFormantFreq(300.f);
	dust.Init();
	ck_noise.Init(sample_rate);
	adsr1.Init(sample_rate);
	adsr2.Init(sample_rate);
	filter.Init(sample_rate);
}

/*-------------------------------------------------------------------
 * buf : audio buffer pointer which contains frames. One frame is one left 16 bits sample + one right 16 bits sample (32 bits)
 * lenght : number of frames to be computed
 *
 * ----------------------------------------------------------------------------------------------------------------------------*/
extern "C" void MakeSound(uint16_t *buf, uint16_t length) //
{

	uint16_t pos;
	uint16_t *outp;
	float y = 0;
	float yL, yR;

	uint16_t valueL, valueR;

	outp = buf;

	for (pos = 0; pos < length; pos++)
	{

		/*--- Generate waveform ---*/
		switch (source)
		{
		case NONE :
			y = 0.f;
			break;

		case WH_NOISE:
			y = w_noise.Process();
			break;

		case PARTICLE:
			y = particle.Process();
			break;

		case GRAIN_OSC:
			y = gr_osc.Process();
			break;

		case DUST:
			y = dust.Process();
			break;

		case CK_NOISE:
			y = ck_noise.Process();
			break;

		default:
			break;
		}

		yL = yR = y;

		/*--- clipping ---*/
		yL = (yL > 1.0f) ? 1.0f : yL; //clip too loud left samples
		yL = (yL < -1.0f) ? -1.0f : yL;

		yR = (yR > 1.0f) ? 1.0f : yR; //clip too loud right samples
		yR = (yR < -1.0f) ? -1.0f : yR;

		/****** Convert the new samples to integers *******/
		valueL = (uint16_t) ((int16_t) ((32767.0f) * yL)); // conversion float -> int
		valueR = (uint16_t) ((int16_t) ((32767.0f) * yR));

		*outp++ = valueL; // left channel sample
		*outp++ = valueR; // right channel sample
	}
}
