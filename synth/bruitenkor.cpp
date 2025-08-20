/***********************************************************************************************
 * bruitenkor.cpp
 *
 * File for sound creation and control
 *
 *  Created on: Aug 5, 2025
 *      Author: Xavier Halgand
 *
 **********************************************************************************************/

#include <math.h>

#include "Freeverb.hpp"
#include "bruitenkor.h"
#include "bruitenkor.hpp"
#include "sequencer.h"
#include "constants.h"
#include "audio_play.h"
#include "keyb_command_parser2.h"
#include "daisysp.h"
#include "MiniFreeverb.h"
#include "wave_data.h"
#include "SamplePlayer.h"
#include "rng.h"
#include "stereo.hpp"

using namespace daisysp;

//-----------------------------------------------------------------------------------------------
EventSequencer seq _CCM_;

// Command table provider from commands.cpp
//extern const CommandParser::Entry* getCommandTable(size_t &outSize);

//-----------------------------------------------------------------------------------------------

static float sample_rate = SAMPLERATE;
static float vol _CCM_;

//static CommandParser parser;

static WhiteNoise _CCM_ w_noise;
static float wnoiseVol _CCM_;
static SyntheticBassDrum synBD _CCM_;
static SamplePlayer snare _CCM_;
static SamplePlayer sp[SP_VOICE_NB] _CCM_;

static AdEnv _CCM_ ad1;
static Adsr _CCM_ adsr1;
static Adsr _CCM_ adsr2;
static bool _CCM_ adsr1_gate;
static bool _CCM_ adsr2_gate;
static Svf _CCM_ filter;

//static Freeverb rev1;	// Freeverb (stereo) : 100kB in RAM
static MiniFreeverb rev _CCM_; // Mini Freeverb (mono) : 23kB in RAM

/*----------------------------------------------------------------------------------------------*/
void samplePlayerRandomInit() {

	for (uint8_t i = 0; i < SP_VOICE_NB; i++) {
		const Sample &sb = sampleBank[GetRandom32bits() % sampleBankCount];
		auto samplebuffer = (const int16_t*) sb.data;
		auto length = sb.length;
		sp[i].Init(sample_rate, samplebuffer, length);
	}
}

/*----------------------------------------------------------------------------------------------*/
void SoundGeneratorInit(void) {

	vol = 13.f;
	wnoiseVol = 1.0f;

	w_noise.Init();
	synBD.Init(sample_rate);
	snare.Init(sample_rate, Snare_808, Snare_808_len);
	samplePlayerRandomInit();

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
void InterpretKey(uint8_t key, uint8_t keycode) {

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

	case '!':
		snare.Trig();
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

	case 't':
		seq.AddOneEvent(0x09);
		break;

	case 'y':
		seq.AddOneEvent(0x19);
		break;

	case 'u':
		seq.AddOneEvent(0x29);
		break;

	case 'i':
		seq.AddOneEvent(0x39);
		break;

	case 'o':
		seq.AddOneEvent(0x49);
		break;

	case 'p':
		seq.AddOneEvent(0x59);
		break;

	case 'n':
		seq.CreatePattern(4);
		seq.DisplayPattern();
		break;

	case 'h':
		seq.AddOneEvent(0xB9);
		break;

	case 'j':
		seq.AddOneEvent(0xA9);
		break;

	case 'k':
		seq.AddOneEvent(0xC9);
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
		seq.livingmode = !seq.livingmode;
		break;

	case 's':
		seq.DisplayStatus();
		break;

	case 'z':
		samplePlayerRandomInit();
		break;

	case ' ':
		seq.isRunning = !seq.isRunning;
		break;

	default:
		if (keycode == 79) {		// left arrow

			seq.ModifySpeed(0.95f);
			printf("Speed up !\r\n");

		} else if (keycode == 80) {	// right arrow

			seq.ModifySpeed(1.05f);
			printf("Slow down !\r\n");

		} else
			feedChar(key);			// Second, multi key commands -> parser
		break;
	}
}

/*----------------------------------------------------------------------------------------------*/
void InterpretEvent(MIDIevent *ev) {

	switch ((ev->type) & 0x0F) {

	case NoteOn:

		if ((ev->type) == 0xA9) {	// NoteOn on cable 10
			wnoiseVol = (ev->data3) / 127.f;
			adsr1.Retrigger(true);
			adsr1_gate = true;

		} else if ((ev->type) == 0xB9) {	// NoteOn on cable 11

			synBD.SetAccent((ev->data3) / 127.f);
			synBD.Trig();

		} else if ((ev->type) == 0xC9) {	// NoteOn on cable 12

			snare.SetAmp((ev->data3) / 127.f);
			snare.Trig();

		} else if ((ev->type) == 0x09) {	// NoteOn on cable 0

			sp[0].SetAmp((ev->data3) / 127.f);
			sp[0].Trig();

		} else if ((ev->type) == 0x19) {	// NoteOn on cable 1

			sp[1].SetAmp((ev->data3) / 127.f);
			sp[1].Trig();

		} else if ((ev->type) == 0x29) {	// NoteOn on cable 2

			sp[2].SetAmp((ev->data3) / 127.f);
			sp[2].Trig();

		} else if ((ev->type) == 0x39) {	// NoteOn on cable 3

			sp[3].SetAmp((ev->data3) / 127.f);
			sp[3].Trig();

		} else if ((ev->type) == 0x49) {	// NoteOn on cable 4

			sp[4].SetAmp((ev->data3) / 127.f);
			sp[4].Trig();

		} else if ((ev->type) == 0x59) {	// NoteOn on cable 5

			sp[5].SetAmp((ev->data3) / 127.f);
			sp[5].Trig();
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
		auto y2 = synBD.Process();
		auto y3 = snare.Process();
		auto z0 = sp[0].Process();
		auto z1 = sp[1].Process();
		auto z2 = sp[2].Process();
		auto z3 = sp[3].Process();
		auto z4 = sp[4].Process();
		auto z5 = sp[5].Process();

		//y = C * (y0 + y1 + y2 + y3 + y4 + y5);

		y = vol * (y0 + y2 * 2.f + y3 + z0 + z1 + z2 + z3 + z4 + z5) / 10.f;

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

///////////////////////// Better but slower :  ///////////////////////////
//		uint16_t valueL = static_cast<uint16_t>(
//		                      static_cast<int16_t>(
//		                          std::lroundf(32767.0f * yL)
//		                      )
//		                  );
//		uint16_t valueR = ...
/////////////////////////////////////////////////////////////////////////

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
