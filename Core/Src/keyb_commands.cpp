/*
 * keyb_commands.cpp
 *
 * List of multi key commands, followed by "enter" key
 *
 *  Created on: Aug 10, 2025
 *      Author: Xavier Halgand & ChatGPT !
 */

/**************************************************************************/

#include "command_parser.hpp"
#include "sequencer.h"
#include <cstdio>

//-----------------------------------------------------------------------------

extern EventSequencer seq;

//******************************   Command functions  *************************

//-----------------------------------------------------------------------------
//	Command : c<n>		Ex : c6 followed by "enter" key -> creates 6 events
//-----------------------------------------------------------------------------
void NewPatterns(int argc, int argv[]) {
    if (argc >= 1) {
        printf("c: create %d new events\r\n", argv[0]);
        seq.CreatePattern(argv[0]);
    } else {
        printf(">>>> Error.  Usage: c<number>\r\n");
    }
}

//-----------------------------------------------------------------------------
//	Command : l<n>		Sets the loop duration at n * 0.1 seconds
//-----------------------------------------------------------------------------
void NewLoop(int argc, int argv[]) {
    if (argc >= 1) {
        printf("l: create new loop : %d units\r\n", argv[0]);
        seq.NewLoop(argv[0]);
    } else {
        printf(">>>> Error.  Usage: l<number>\r\n");
    }
}

//-----------------------------------------------------------------------------
//	Command : b<n>		Creates n regular events in the loop
//-----------------------------------------------------------------------------
void addRegPattern(int argc, int argv[]) {
    if (argc >= 1) {
        printf("b: add %d new regular events for instrument %d\r\n", argv[0], argv[1]);
        seq.AddRegularPattern(argv[0], argv[1]);
    } else {
        printf(">>>> Error.  Usage: b<number> <instr>\r\n");
    }
}

//******************************************************************************
// Table de commandes
static const CommandParser::Entry g_commandTable[] = {

	{"c", NewPatterns, 1},

	{"l", NewLoop, 1},

	{"b", addRegPattern, 2},

    // ajouter d'autres commandes ici
};

//******************************************************************************
const CommandParser::Entry* getCommandTable(size_t &outSize) {
    outSize = sizeof(g_commandTable) / sizeof(g_commandTable[0]);
    return g_commandTable;
}


