/*
 * sequencer.cpp
 *
 *  Created on: Aug 7, 2025
 *      Author: Xavier Halgand
 */
/*---------------------------------------------------------------------------------------------*/
#include "sequencer.h"

#include "bruitenkor.hpp"
#include "bruitenkor.h"
#include "objectpool.hpp"
#include "rng.h"
#include "tim.h"
#include "usart.h"

#include <stdio.h>
#include <cmath>
#include <inttypes.h> // pour PRIu32

/*---------------------------------------------------------------------------------------------*/
// MIDIevent ev1(0, 0x09, 0x90, 57, 127); // At tick 0, Note On A3 velocity = 127
// MIDIevent ev2(333, 0x08, 0x80, 57, 127); // At tick 333, Note Off A3 velocity = 127
ObjectPool<MIDIevent, MAX_EVENT_NB> pool _CCM_;

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Init(float sr, uint16_t reso, uint32_t max_len) {

	Clear();
	isRunning = true;
	livingmode = true;
	isRecording = false;
	Restart();
	CreateEvents(4);
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Clear() {

	event_list_.clear();
	pool.clear();
	event_counter_ = 0;
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::RandomizeVelo() {

	for (auto ev : event_list_) {
		auto v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		ev->data3 = v;
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Quantize(uint16_t div) {

	auto step = (loop_len_ / (float) (div));
	for (auto ev : event_list_) {
		auto pos = static_cast<uint32_t>(std::round(((ev->position) / step)) * std::round(step));
		if (pos >= loop_len_)
			ev->position = 0;
		else
			ev->position = pos;
	}
	TimeSort();	// Because of position 0
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Add(MIDIevent *ev) {

	if (event_counter_ < max_event_) {
		event_list_.push_front(ev);
		event_counter_++;
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::TimeSort() {

	event_list_.sort([](MIDIevent *a, MIDIevent *b) {
		return a->position < b->position;
	});
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::CreateEvents(uint16_t ev_nb) {

	for (uint16_t i = 0; i < ev_nb; i++) {
		auto t = (GetRandom32bits() % loop_len_);
		auto type = GetRandomInstr();
		auto v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		AddOneMidiEvent(t, type, 7, 8, v);
	}
	//TimeSort();
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::DeleteEvents(uint16_t ev_nb) {

	if (event_list_.empty() || ev_nb == 0)
		return;

	while (ev_nb-- > 0 && !event_list_.empty()) {

		size_t index = GetRandomInteger(0, event_list_.size() - 1);

		// Avancer jusqu’à l’élément choisi
		auto it = event_list_.begin();
		std::advance(it, index);

		// Libérer l’objet dans le pool
		pool.free(*it);

		// Enlever le pointeur de la liste
		it = event_list_.erase(it);
		event_counter_--;
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddGeneralPattern(uint16_t ev_nb) {

	uint32_t t;
	uint8_t type;
	uint8_t v;

	for (uint16_t i = 0; i < ev_nb; i++) {

		for (uint8_t j = 0; j < SP_VOICE_NB; j++) {

			t = (GetRandom32bits() % loop_len_);
			type = 9 + 16 * j;
			v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
			AddOneMidiEvent(t, type, 5, 6, v);
		}
		t = (GetRandom32bits() % loop_len_);
		type = 0xA9;
		v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		AddOneMidiEvent(t, type, 5, 6, v);

		t = (GetRandom32bits() % loop_len_);
		type = 0xB9;
		v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		AddOneMidiEvent(t, type, 5, 6, v);

		t = (GetRandom32bits() % loop_len_);
		type = 0xC9;
		v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		AddOneMidiEvent(t, type, 5, 6, v);

	}
	//TimeSort();

}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddRegularPattern(uint16_t ev_nb, int inst) {

	if (inst >= 0 && inst < SP_VOICE_NB) {

		auto step = static_cast<uint16_t>(std::round(loop_len_ / (float) (ev_nb)));
		auto shift = GetRandomInteger(0, step - 1);

		for (int i = 0; i < ev_nb; i++) {
			auto v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
			auto t = i * step + shift;
			auto ev = pool.allocate(t, 16 * inst + 9, 3, 4, v);
			Add(ev);
		}
		TimeSort();
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::NewLoop(uint32_t units) { // units = dixième de secondes (1 unit = 0.1 sec)

	auto f = sample_rate_ * units / resolution_ / 10;
	auto len = static_cast<uint32_t>(std::round(f));
	printf("loop length = %ld seq_ticks\r\n", len);
	if (len <= max_len_ && len >= 10) {
		loop_len_ = len;
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Restart() {

	sample_counter_ = 0;
	tick_counter_ = 0;
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::ModifySpeed(float coef) {

	auto len = static_cast<uint32_t>(std::round(loop_len_ * coef));
	if (len <= max_len_ && len >= 10) {
		loop_len_ = len;
		for (auto ev : event_list_) {
			ev->position = static_cast<uint32_t>(std::round(ev->position * coef));
		}
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddOneMidiEvent(uint32_t position, uint8_t type, uint8_t data1, uint8_t data2, uint8_t data3) {

	if (event_counter_ < max_event_) {
		auto ev = pool.allocate(position, type, data1, data2, data3);
		event_list_.push_front(ev);
		TimeSort();
		event_counter_++;
	}
}
/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddOneEvent(uint8_t type) {

	if (event_counter_ < max_event_) {

		auto t = (GetRandom32bits() % loop_len_);
		auto v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		auto ev = pool.allocate(t, type, 1, 2, v);
		event_list_.push_front(ev);
		TimeSort();
		event_counter_++;

//		printf("New event ! : position = %" PRIu32
//		" || type = %#04X || data1 = %u || data2 = %u || data3 = %u ||\r\n",
//				ev->position, ev->type, ev->data1, ev->data2, ev->data3);
//		printf(">>>>>>>  Number of Events : %u\r\n", event_counter_);
//		PrintALine();
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddOneEventNow(uint8_t type) {

	if (event_counter_ < max_event_) {

		auto t = tick_counter_;
		auto v = GetRandomInteger(MINI_VELO, MIDI_MAXi);
		auto ev = pool.allocate(t, type, 1, 2, v);
		event_list_.push_front(ev);
		TimeSort();
		event_counter_++;

//		printf("New event now ! : position = %" PRIu32
//		" || type = %#04X || data1 = %u || data2 = %u || data3 = %u ||\r\n",
//				ev->position, ev->type, ev->data1, ev->data2, ev->data3);
//		printf(">>>>>>>  Number of Events : %u\r\n", event_counter_);
//		PrintALine();
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::MixUp() {

	for (auto ev : event_list_) {
		auto t = (GetRandom32bits() % loop_len_);
		ev->position = t;
	}
	TimeSort();
}
/*---------------------------------------------------------------------------------------------*/
void EventSequencer::DisplayPattern() {

	if (event_list_.empty()) {
		printf("Loop is empty ! \r\n");

	}
	else {
		uart_printf(">>>>>>>  Number of Events : %u\r\n", event_counter_);
		for (auto ev : event_list_) {
			if (ev == nullptr) {
				printf("Event : null pointer ! \r\n");
				continue;
			}
			uart_printf("Event : position = %" PRIu32
			" || type = %#04X || data1 = %u || data2 = %u || data3 = %u ||\r\n", ev->position, ev->type, ev->data1,
					ev->data2, ev->data3);
		}
	}
	PrintALine();
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::DisplayStatus() {

	uart_printf("/////////// Sequencer status : ////////////\r\n");
	uart_printf("// Seq is running : %d \r\n", isRunning);
	uart_printf("// Seq is in automode : %d \r\n", livingmode);
	uart_printf("// Seq is recording : %d \r\n", isRecording);
	uart_printf("// Sample rate = %ld\r\n", static_cast<uint32_t>(sample_rate_));
	uart_printf("// Resolution = %d  sample ticks.\r\n", resolution_);
	uart_printf("// Loop length = %ld  seq ticks.\r\n", loop_len_);
	uart_printf("// Maximum loop length = %ld  seq ticks.\r\n", max_len_);
	uart_printf("// Maximum number of events = %d .\r\n", max_event_);
	uart_printf("// Number of registered events = %d .\r\n", event_counter_);
	uart_printf("////////////\r\n");
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::TickAction() {

	for (auto ev : event_list_) {
		if (ev->position > tick_counter_)	// event list is ordered !
			break;
		if (tick_counter_ == ev->position)
			InterpretEvent(ev);
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::LoopAction() {
	//printf("New loop !\r\n");
	pulseLED();
	if (livingmode) {
		//printf("Automode is ON !\r\n");
		RandomizeVelo();
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Process() {
	//TestPrintCounters();

	if (isRunning) {

		// Time to do something ?
		if (sample_counter_ == 0) {	// *******  New tick !
			TickAction();
			if (tick_counter_ == 0)
				LoopAction();	// **********  New loop !
		}

		// increment counters :
		if (sample_counter_ < resolution_ - 1)
			sample_counter_++;
		else {
			sample_counter_ = 0;
			if (tick_counter_ < loop_len_ - 1)
				tick_counter_++;
			else
				tick_counter_ = 0;
		}
	}
}

/*---------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::TestPrintCounters() {

	printf("samples = %d || ticks = %ld\r\n", sample_counter_, tick_counter_);
}

/*---------------------------------------------------------------------------------------------*/
