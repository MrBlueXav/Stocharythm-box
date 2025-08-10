/*
 * sequencer.h
 *
 *  Created on: Aug 7, 2025
 *      Author: Xavier Halgand
 */

#ifndef INC_SEQUENCER_H_
#define INC_SEQUENCER_H_

#include <stdint.h>

#include "midi_types.h"

/*-----------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus

#include <list>
using namespace std;

/*-----------------------------------------------------------------------------------------------------------*/
class EventSequencer {
public:
	EventSequencer() {
	}
	~EventSequencer() {
	}

	void Init(float sr = 48'000.f, uint16_t reso = 48,
			uint32_t max_len = 10'000);
	void Process();
	void CreatePattern(uint16_t evnb);
	void DisplayPattern();
	void AddOneEvent();
	void Clear();
	void RandomizeVelo();

private:
	float sample_rate_;
	uint16_t resolution_;		// Number of samples between each sequencer tick
	uint16_t sample_counter_;
	uint32_t tick_counter_;
	uint32_t max_len_;			// Maximum number of ticks in loop
	uint32_t loop_len_;			// Actual number of ticks in loop
	uint16_t event_counter_;
	uint16_t max_event_;
	list<MIDIevent*> event_list_;

	void TestPrintCounters();
	void TimeSort();
	void TickAction();
	void Add(MIDIevent *ev);
};

/*-----------------------------------------------------------------------------------------------------------*/

#endif

#endif /* INC_SEQUENCER_H_ */
