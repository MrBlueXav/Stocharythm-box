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

#include <stdio.h>
#include <cmath>
#include <inttypes.h> // pour PRIu32

/*---------------------------------------------------------------------------------------------*/
// MIDIevent ev1(0, 0x09, 0x90, 57, 127); // At tick 0, Note On A3 velocity = 127
// MIDIevent ev2(333, 0x08, 0x80, 57, 127); // At tick 333, Note Off A3 velocity = 127
ObjectPool<MIDIevent, MAX_EVENT_NB> pool _CCM_;

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::Init(float sr, uint16_t reso, uint32_t max_len) {

	pool.clear();
	CreatePattern(4);
	isRunning = true;
	automode = false;
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
		auto v = (GetRandom32bits() % 83) + 45;
		ev->data3 = v;
	}
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
void EventSequencer::CreatePattern(uint16_t ev_nb) {

	for (uint16_t i = 0; i < ev_nb; i++) {
		auto x = (GetRandom32bits() % loop_len_) + 1;
		auto v = (GetRandom32bits() % 83) + 45;
		auto ev = pool.allocate(x, 0x09, 1, 2, v);
		Add(ev);
		//printf("random position = %ld \r\n", x);
	}
	TimeSort();
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddRegularPattern(uint16_t ev_nb, int inst) {

	if (inst >= 0 && inst < SP_VOICE_NB) {

		auto step = static_cast<uint16_t>(std::round(
				loop_len_ / (float) (ev_nb)));

		for (int i = 0; i < ev_nb; i++) {
			auto v = (GetRandom32bits() % 83) + 45;
			auto ev = pool.allocate(i * step, 16 * inst + 9, 3, 4, v);
			Add(ev);
		}
		TimeSort();
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::NewLoop(uint32_t units) {	// units = dixième de secondes

	auto f = sample_rate_ * units / resolution_ / 10;
	auto len = static_cast<uint32_t>(std::round(f));
	printf("loop length = %ld seq_ticks\r\n", len);
	if (len <= max_len_ && len >= 10) {
		loop_len_ = len;
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::ModifySpeed(float coef) {

	auto len = static_cast<uint32_t>(std::round(loop_len_ * coef));
	if (len <= max_len_ && len >= 10) {
		loop_len_ = len;
		for (auto ev : event_list_) {
			ev->position =
					static_cast<uint32_t>(std::round(ev->position * coef));
		}
	}

}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::AddOneEvent(uint8_t type) {

	if (event_counter_ < max_event_) {

		auto t = (GetRandom32bits() % loop_len_);
		auto v = (GetRandom32bits() % 83) + 45;
		auto ev = pool.allocate(t, type, 1, 2, v);
		event_list_.push_front(ev);
		TimeSort();
		event_counter_++;
		printf("New event ! : position = %" PRIu32
		" || type = %#04X || data1 = %u || data2 = %u || data3 = %u ||\r\n",
				ev->position, ev->type, ev->data1, ev->data2, ev->data3);
		printf(">>>>>>>  Number of Events : %u\r\n", event_counter_);
		PrintALine();
	}
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::DisplayPattern() {

	if (event_list_.empty()) {
		printf("Pattern vide ! \r\n");

	} else {
		printf(">>>>>>>  Number of Events : %u\r\n", event_counter_);
		for (auto ev : event_list_) {
			if (ev == nullptr) {
				printf("Event : null pointer ! \r\n");
				continue;
			}
			printf("Event : position = %" PRIu32
			" || type = %#04X || data1 = %u || data2 = %u || data3 = %u ||\r\n",
					ev->position, ev->type, ev->data1, ev->data2, ev->data3);
		}
	}
	PrintALine();
}

/*---------------------------------------------------------------------------------------------*/
void EventSequencer::DisplayStatus() {

	printf("/////////// Sequencer status : ////////////\r\n");
	printf("// Seq is running : %d \r\n", isRunning);
	printf("// Seq is in automode : %d \r\n", automode);
	printf("// Sample rate = %ld\r\n", static_cast<uint32_t>(sample_rate_));
	printf("// Resolution = %d  sample ticks.\r\n", resolution_);
	printf("// Loop length = %ld  seq ticks.\r\n", loop_len_);
	printf("// Maximum loop length = %ld  seq ticks.\r\n", max_len_);
	printf("// Number of registered events = %d .\r\n", event_counter_);
	printf("// Maximum number of events = %d .\r\n", max_event_);
	printf("////////////\r\n");
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
	if (automode) {
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
