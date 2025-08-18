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
#include "constants.h"

/*-----------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus

#include <list>
using namespace std;

/*-----------------------------------------------------------------------------------------------------------*/
class EventSequencer {

public:

	EventSequencer() :
			isRunning(false),
			automode(false),
			sample_rate_(SAMPLERATE),
			resolution_(48),
			sample_counter_(0),
			tick_counter_(0),
			max_len_(20'000),
			loop_len_(1000),
			event_counter_(0),
			max_event_(MAX_EVENT_NB)
	{
		event_list_.clear();
	}
	~EventSequencer() {
	}

	bool isRunning;
	bool automode;

	void Init(float sr = 48'000.f, uint16_t reso = 48,
			uint32_t max_len = 20'000);
	void Process();
	void CreatePattern(uint16_t ev_nb);
	void AddRegularPattern(uint16_t ev_nb);
	void NewLoop(uint32_t units);	// units = dixième de secondes
	void ModifySpeed(float coef);
	void DisplayPattern();
	void DisplayStatus();
	void AddOneEvent(uint8_t type);
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
	void LoopAction();
	void Add(MIDIevent *ev);
};

/*-----------------------------------------------------------------------------------------------------------*/

#endif

#endif /* INC_SEQUENCER_H_ */
