/***********************************************************************************************
 * bruitenkor.cpp
 *
 * File for sound creation and control
 *
 *  Created on: Aug 5, 2025
 *      Author: Xavier Halgand
 *
 **********************************************************************************************/

#include <keyb_command_parser.h>
#include <math.h>

#include "Freeverb.hpp"
#include "bruitenkor.h"
#include "bruitenkor.hpp"
#include "sequencer.h"
#include "constants.h"
#include "audio_play.h"
#include "daisysp.h"
#include "MiniFreeverb.h"
#include "freeverb_stm32.hpp"
#include "stereo.hpp"
#include "wave_data.h"
#include "SamplePlayer.h"
#include "rng.h"
#include "stereo.hpp"
#include "tim.h"
#include "usart.h"

//-----------------------------------------------------------------------------------------------

using namespace daisysp;

//-----------------------------------------------------------------------------------------------
extern bool multikey;

//-----------------------------------------------------------------------------------------------
EventSequencer seq _CCM_;

//-----------------------------------------------------------------------------------------------

static float sample_rate = SAMPLERATE;
static float source_gain _CCM_;

static WhiteNoise _CCM_ w_noise;
static float wnoiseVol _CCM_;
static SamplePlayer kick _CCM_;
static SamplePlayer snare _CCM_;
//static SamplePlayer kick _CCM_;
static SamplePlayer sp[SP_VOICE_NB] _CCM_;
static Adsr _CCM_ adsr1;
static bool _CCM_ adsr1_gate;
static Svf _CCM_ filter;

//static Freeverb rev1;	// Freeverb (stereo) : 100kB in RAM
//static MiniFreeverb rev _CCM_; // Mini Freeverb (mono) : 23kB in RAM
static FreeverbStereoSTM32 rvb _CCM_;
static InputChannel mixer[9] _CCM_;

static const uint8_t instr_code[] = { 0x09, 0x19, 0x29, 0x39, 0x49, 0x59, 0xA9, 0xB9, 0xC9 };
static constexpr size_t number_of_instr = sizeof(instr_code) / sizeof(instr_code[0]);

/*----------------------------------------------------------------------------------------------*/
void samplePlayersRandomInit() {

	for (uint8_t i = 0; i < SP_VOICE_NB; i++) {
		const Sample &sb = sampleBank[GetRandom32bits() % sampleBankCount];
		auto samplebuffer = (const int16_t*) sb.data;
		auto length = sb.length;
		sp[i].Init(sample_rate, samplebuffer, length);
	}
}

/*----------------------------------------------------------------------------------------------*/
void SoundGeneratorInit(void) {

	source_gain = 1.f;
	wnoiseVol = 1.0f;
	multikey = false;

	w_noise.Init();
	kick.Init(sample_rate, Fat_Kick, Fat_Kick_len);
	snare.Init(sample_rate, Snare_808, Snare_808_len);
	samplePlayersRandomInit();

	rvb.init();
	rvb.setWetDry(0.08f);
	rvb.setRoomSize(0.72f);
	rvb.setDamp(0.28f);
	rvb.setWidth(1.0f);

	adsr1.Init(sample_rate);
	adsr1_gate = false;
	adsr1.SetTime(ADSR_SEG_ATTACK, 0.0f);
	adsr1.SetTime(ADSR_SEG_DECAY, 0.03f);
	adsr1.SetTime(ADSR_SEG_RELEASE, 0.01f);
	adsr1.SetSustainLevel(0.f);

	filter.Init(sample_rate);
	seq.Init(sample_rate);
}

/*----------------------------------------------------------------------------------------------*/
uint8_t GetRandomInstr(void) {

	return instr_code[GetRandom32bits() % number_of_instr];

}

/*----------------------------------------------------------------------------------------------*/
void InterpretKey(uint8_t key, uint8_t keycode) {

	if (multikey == true)	// Multi key commands -> parser

		feedChar(key);

	else {		// Single key commands.

		switch (key) {

		case '&':
			uart_IT_Test();
			break;

		case 'c':							// Beginning of a multi key command
			multikey = true;
			feedChar(key);
			break;

		case '(':
			source_gain *= 1.1f;
			printf("Source volume = %d \r\n", static_cast<uint16_t>(source_gain * 100));
			break;

		case ')':
			source_gain *= 0.9f;
			printf("Source volume = %d \r\n", static_cast<uint16_t>(source_gain * 100));
			break;

		case '*':
			seq.isRecording = true;
			blinkLED(1);
			printf("Sequencer is recording ! \n\r");
			break;

		case '/':
			seq.isRecording = false;
			blinkLED(0);
			printf("Sequencer is not recording ! \n\r");
			break;

		case '!':									//	Play/stop
			seq.isRunning = !seq.isRunning;
			seq.Restart();
			break;

		case ' ':									//	Play/pause
			seq.isRunning = !seq.isRunning;
			break;

		case '+':
			incVol();
			break;

		case '-':
			decVol();
			break;

		case 'T':
			seq.AddOneEvent(0x09);
			break;

		case 't':
			sp[0].Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0x09);
			break;

		case 'Y':
			seq.AddOneEvent(0x19);
			break;

		case 'y':
			sp[1].Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0x19);
			break;

		case 'U':
			seq.AddOneEvent(0x29);
			break;

		case 'u':
			sp[2].Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0x29);
			break;

		case 'I':
			seq.AddOneEvent(0x39);
			break;

		case 'i':
			sp[3].Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0x39);
			break;

		case 'O':
			seq.AddOneEvent(0x49);
			break;

		case 'o':
			sp[4].Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0x49);
			break;

		case 'P':
			seq.AddOneEvent(0x59);
			break;

		case 'p':
			sp[5].Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0x59);
			break;

		case 'H':
			seq.AddOneEvent(0xB9);
			break;

		case 'h':
			kick.Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0xB9);
			break;

		case 'J':
			seq.AddOneEvent(0xA9);
			break;

		case 'j':
			adsr1.Retrigger(true);
			if (seq.isRecording)
				seq.AddOneEventNow(0xA9);
			break;

		case 'K':
			seq.AddOneEvent(0xC9);
			break;

		case 'k':
			snare.Trig();
			if (seq.isRecording)
				seq.AddOneEventNow(0xC9);
			break;

		case '.':
			seq.Clear();
			printf("All events cleared !\r\n");
			break;

		case 'n':
			seq.CreateEvents(2);
			break;

		case 'b':
			seq.DeleteEvents(2);
			break;

		case 'v':
			seq.RandomizeVelo();
			printf("New random velocities !\r\n");
			break;

		case 'a':
			seq.livingmode = !seq.livingmode;
			break;

		case 'm':
			seq.MixUp();
			break;

		case 'z':
			samplePlayersRandomInit();
			break;

		case 'd':
			seq.DisplayPattern();
			break;

		case 's':
			seq.DisplayStatus();
			break;

		default:
			if (keycode == 79) {		// left arrow

				seq.ModifySpeed(0.95f);
				printf("Speed up !\r\n");

			}
			else if (keycode == 80) {	// right arrow

				seq.ModifySpeed(1.05f);
				printf("Slow down !\r\n");

			}
			break;
		}
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

		}
		else if ((ev->type) == 0xB9) {	// NoteOn on cable 11

			kick.SetAmp((ev->data3) / 127.f);
			kick.Trig();

		}
		else if ((ev->type) == 0xC9) {	// NoteOn on cable 12

			snare.SetAmp((ev->data3) / 127.f);
			snare.Trig();

		}
		else if ((ev->type) == 0x09) {	// NoteOn on cable 0

			sp[0].SetAmp((ev->data3) / 127.f);
			sp[0].Trig();

		}
		else if ((ev->type) == 0x19) {	// NoteOn on cable 1

			sp[1].SetAmp((ev->data3) / 127.f);
			sp[1].Trig();

		}
		else if ((ev->type) == 0x29) {	// NoteOn on cable 2

			sp[2].SetAmp((ev->data3) / 127.f);
			sp[2].Trig();

		}
		else if ((ev->type) == 0x39) {	// NoteOn on cable 3

			sp[3].SetAmp((ev->data3) / 127.f);
			sp[3].Trig();

		}
		else if ((ev->type) == 0x49) {	// NoteOn on cable 4

			sp[4].SetAmp((ev->data3) / 127.f);
			sp[4].Trig();

		}
		else if ((ev->type) == 0x59) {	// NoteOn on cable 5

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
	//float y = 0;
	float yL, yR;
	float mL, mR;
	uint16_t valueL, valueR;
	outp = buf;

	for (pos = 0; pos < length; pos++) {

		seq.Process();

		/*--- Generate waveforms ---*/

		auto y0 = w_noise.Process() * adsr1.Process(adsr1_gate) * wnoiseVol;
		//auto y1 = 0.0f;
		auto y1 = kick.Process();
		auto y2 = snare.Process();
		auto z0 = sp[0].Process();
		auto z1 = sp[1].Process();
		auto z2 = sp[2].Process();
		auto z3 = sp[3].Process();
		auto z4 = sp[4].Process();
		auto z5 = sp[5].Process();

		mixer[0] = {z0, source_gain, -0.7f};
		mixer[1] = {z1, source_gain, -0.5f};
		mixer[2] = {z2, source_gain, -0.3f};
		mixer[3] = {z3, source_gain, 0.3f};
		mixer[4] = {z4, source_gain, 0.5f};
		mixer[5] = {z5, source_gain, 0.7f};
		mixer[6] = {y0, source_gain, -0.1f};
		mixer[7] = {y1, source_gain, 0.f};
		mixer[8] = {y2, source_gain, 0.1f};

		mixStereo(mixer, 9, mL, mR);

//		mL *= source_gain;
//		mR *= source_gain;

		//y = source_gain * (y0 + y1 + y2 + z0 + z1 + z2 + z3 + z4 + z5) / 10.f;

//		y = 0.5f * y + 0.5f * rev.process(y);

//		yL = mL;
//		yR = mR;
		rvb.process(mL, mR, yL, yR);
		//yL = yR = y;

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
	printf("--------------------------------------------------------------------------------\r\n");

}

//**********************************************************************************************
