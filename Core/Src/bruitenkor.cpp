/*
 * bruitenkor.cpp
 *
 *  Created on: Aug 5, 2025
 *      Author: Xavier Halgand
 */

#include <Freeverb.hpp>
#include "bruitenkor.h"
#include "bruitenkor.hpp"
#include "sequencer.h"
#include "constants.h"
#include "audio_play.h"
#include "command_parser.hpp"

#include <math.h>
#include "daisysp.h"
#include "MiniFreeverb.h"

using namespace daisysp;

//-----------------------------------------------------------------------------------------------
enum Source {
	NONE, WH_NOISE, PARTICLE, GRAIN_OSC, DUST, CK_NOISE, END
};

//-----------------------------------------------------------------------------------------------
EventSequencer seq _CCM_;

// Command table provider from commands.cpp
extern const CommandParser::Entry* getCommandTable(size_t &outSize);

//-----------------------------------------------------------------------------------------------

static float sample_rate = SAMPLERATE;
static CommandParser parser;
static WhiteNoise _CCM_ w_noise;
static float wnoiseVol _CCM_;

//static SyntheticBassDrum bd _CCM_;
//static bool bdTrig;

//static AnalogBassDrum anaBD _CCM_;
static SyntheticBassDrum synBD _CCM_;
//static AnalogSnareDrum anaSD _CCM_;
//static SyntheticSnareDrum synSD _CCM_;
//static HiHat hh _CCM_;

static float vol _CCM_;

static AdEnv _CCM_ ad1;
static Adsr _CCM_ adsr1;
static Adsr _CCM_ adsr2;
static bool _CCM_ adsr1_gate;
static bool _CCM_ adsr2_gate;
static Svf _CCM_ filter;
//static Freeverb rev1;	// Freeverb (stereo) : 100kB in RAM
static MiniFreeverb rev _CCM_; // Mini Freeverb (mono) : 23kB in RAM

/*----------------------------------------------------------------------------------------------*/
void SoundGeneratorInit(void) {

	// set command table
	size_t ts;
	const CommandParser::Entry *table = getCommandTable(ts);
	parser.setTable(table, ts);

	vol = 2.2f;
	wnoiseVol = 1.0f;

	w_noise.Init();
	//particle.Init(sample_rate);
	//gr_osc.Init(sample_rate);
	//gr_osc.SetFreq(110.f);
	//gr_osc.SetFormantFreq(300.f);
	//dust.Init();
	//ck_noise.Init(sample_rate);

//	bd.Init(sample_rate);
//	bd.SetFreq(50.f);
//	bd.SetDirtiness(.5f);
//	bd.SetFmEnvelopeAmount(.6f);
//	bdTrig = false;
//
//	anaBD.Init(sample_rate);
//	anaSD.Init(sample_rate);
//	synSD.Init(sample_rate);
//	hh.Init(sample_rate);

	synBD.Init(sample_rate);

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

	case '(':				// First, one key commands.
		vol *= 1.1f;
		printf("Source volume = %d \r\n", static_cast<uint16_t>(vol * 100));
		break;

	case ')':
		vol *= 0.9f;
		printf("Source volume = %d \r\n", static_cast<uint16_t>(vol * 100));
		break;

	case '/':
		synBD.Trig();
		break;

	case '*':
		adsr1.Retrigger(true);
		break;

	case '+':
		incVol();
		break;

	case '-':
		decVol();
		break;

	case 'n':
		seq.CreatePattern(4);
		seq.DisplayPattern();
		break;

	case 'h':
		seq.AddOneEvent(0x19);
		break;

	case 'j':
		seq.AddOneEvent(0x09);
		break;

	case '.':
		seq.Clear();
		printf("All events cleared !\r\n");
		break;

	case 'd':
		seq.DisplayPattern();
		break;

	case 'v':
		seq.RandomizeVelo();
		printf("New random velocities !\r\n");
		break;

	case 'a':
		seq.automode = !seq.automode;
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
		if ((ev->type) == 0x09) {	// NoteOn on cable 0
			wnoiseVol = (ev->data3) / 127.f;
			adsr1.Retrigger(true);
			adsr1_gate = true;

		} else if ((ev->type) == 0x19) {	// NoteOn on cable 1

			synBD.SetAccent((ev->data3) / 127.f);
			synBD.Trig();
		}
		break;

	case NoteOff:
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

		/*--- Generate waveforms ---*/

		//const float C = 1.f / 6.f; // reverb mix coefficient
		auto y0 = w_noise.Process() * adsr1.Process(adsr1_gate) * wnoiseVol;
		//auto y1 = anaBD.Process();
		auto y2 = synBD.Process();
		//auto y3 = anaSD.Process();
		//auto y4 = synSD.Process();
		//auto y5 = hh.Process();
		//anaBDTrig = synBDTrig = anaSDTrig = synSDTrig = hhTrig = false; // reset triggers

		//y = C * (y0 + y1 + y2 + y3 + y4 + y5);

		y = vol * (y0 + y2 * 2.f);

		y = 0.5f * y + 0.5f * rev.process(y);

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
