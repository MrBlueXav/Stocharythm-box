/*
 * keyb_commands.cpp
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

//*****************************************************************************
// Command functions
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
void NewLoop(int argc, int argv[]) {
    if (argc >= 1) {
        printf("l: create new loop : %d units\r\n", argv[0]);
        seq.NewLoop(argv[0]);
    } else {
        printf(">>>> Error.  Usage: l<number>\r\n");
    }
}

//-----------------------------------------------------------------------------
void addRegPattern(int argc, int argv[]) {
    if (argc >= 1) {
        printf("r: add %d new regular events\r\n", argv[0]);
        seq.AddRegularPattern(argv[0]);
    } else {
        printf(">>>> Error.  Usage: r<number>\r\n");
    }
}

//******************************************************************************
// Table de commandes
static const CommandParser::Entry g_commandTable[] = {

	{"c", NewPatterns, 1},

	{"l", NewLoop, 1},

	{"r", addRegPattern, 1},

    // ajouter d'autres commandes ici
};

//******************************************************************************
const CommandParser::Entry* getCommandTable(size_t &outSize) {
    outSize = sizeof(g_commandTable) / sizeof(g_commandTable[0]);
    return g_commandTable;
}


