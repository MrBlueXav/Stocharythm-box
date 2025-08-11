/*
 * bruitenkor.cpp
 *
 *  Created on: Aug 5, 2025
 *      Author: Xavier Halgand
 */

#include "bruitenkor.h"
#include "bruitenkor.hpp"
#include "sequencer.h"
#include "constants.h"
#include "audio_play.h"
#include "command_parser.hpp"

#include <math.h>
#include "daisysp.h"

using namespace daisysp;

//-----------------------------------------------------------------------------------------------
enum Source {
	NONE, WH_NOISE, PARTICLE, GRAIN_OSC, DUST, CK_NOISE, END
};

//-----------------------------------------------------------------------------------------------
_CCM_ EventSequencer seq;

// Command table provider from commands.cpp
extern const CommandParser::Entry* getCommandTable(size_t &outSize);

//-----------------------------------------------------------------------------------------------

static float sample_rate = SAMPLERATE;
static CommandParser parser;
static WhiteNoise _CCM_ w_noise;
static Particle _CCM_ particle;
static GrainletOscillator _CCM_ gr_osc;
static Dust _CCM_ dust;
static ClockedNoise _CCM_ ck_noise;
static Source source = WH_NOISE;
static float vol;
static AdEnv _CCM_ ad1;
static Adsr _CCM_ adsr1;
static Adsr _CCM_ adsr2;
static bool _CCM_ adsr1_gate;
static bool _CCM_ adsr2_gate;
static Svf _CCM_ filter;

/*----------------------------------------------------------------------------------------------*/
void SoundGeneratorInit(void) {

	// set command table
	size_t ts;
	const CommandParser::Entry *table = getCommandTable(ts);
	parser.setTable(table, ts);

	vol = 1.0f;

	w_noise.Init();
	particle.Init(sample_rate);
	gr_osc.Init(sample_rate);
	gr_osc.SetFreq(110.f);
	gr_osc.SetFormantFreq(300.f);
	dust.Init();
	ck_noise.Init(sample_rate);
	adsr1.Init(sample_rate);
	adsr1_gate = false;
	adsr1.SetTime(ADSR_SEG_ATTACK, 0.0f);
	adsr1.SetTime(ADSR_SEG_DECAY, 0.03f);
	adsr1.SetTime(ADSR_SEG_RELEASE, 0.01f);
	adsr1.SetSustainLevel(0.f);
	adsr2.Init(sample_rate);
	adsr2_gate = false;

	ad1.Init(sample_rate);
	ad1.SetTime(ADENV_SEG_ATTACK, 0.0f);
	ad1.SetTime(ADENV_SEG_DECAY, 0.03f);

	filter.Init(sample_rate);
	seq.Init(sample_rate);
}

/*----------------------------------------------------------------------------------------------*/
void InterpretKey(uint8_t key) {

	switch (key) {

//	case '0':
//		source = NONE;
//		break;
//
//	case '1':
//		source = WH_NOISE;
//		break;
//
//	case '2':
//		source = PARTICLE;
//		break;
//
//	case '3':
//		source = GRAIN_OSC;
//		break;
//
//	case '4':
//		source = DUST;
//		break;
//
//	case '5':
//		source = CK_NOISE;
//		break;

	case '+':
		incVol();
		break;

	case '-':
		decVol();
		break;

	case '*':
		seq.CreatePattern(4);
		seq.DisplayPattern();
		break;

	case '/':
		seq.AddOneEvent();
		break;

	case '.':
		seq.Clear();
		break;

	case 'd':
		seq.DisplayPattern();
		break;

	case 'v':
		seq.RandomizeVelo();
		break;

	case 's':
		seq.DisplayStatus();
		break;

	case ' ':
		seq.isRunning = !seq.isRunning;
		break;

	default:
		parser.feedChar(key);
		break;
	}
}

/*----------------------------------------------------------------------------------------------*/
void InterpretEvent(MIDIevent *ev) {
	switch ((ev->type) & 0x0F) {
	case NoteOn:
		vol = (ev->data3) / 127.f;
		adsr1.Retrigger(true);
		adsr1_gate = true;
		break;

	case NoteOff:
		//adsr1_gate = false;
		break;

	case ControlChange:
		break;

	default:
		break;
	}
}

/*----------------------------------------------------------------------------------------------*/
void MakeSound(uint16_t *buf, uint16_t length) //
		/*-------------------------------------------------------------------
		 * buf : audio buffer pointer which contains frames. One frame is one left 16 bits sample + one right 16 bits sample (32 bits)
		 * length : number of frames to be computed
		 *
		 * ----------------------------------------------------------------------------------------------------------------------------*/
		{
	uint16_t pos;
	uint16_t *outp;
	float y = 0;
	float yL, yR;

	uint16_t valueL, valueR;

	outp = buf;

	for (pos = 0; pos < length; pos++) {

		seq.Process();

		/*--- Generate waveform ---*/
		switch (source) {
		case NONE:
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
		y = y * vol * adsr1.Process(adsr1_gate);

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

/*----------------------------------------------------------------------------------------------*/
void PrintALine(void) {
	printf(
			"--------------------------------------------------------------------------------\r\n");

}

//**********************************************************************************************
