/*
 * keyb_command_parser2.cpp
 *
 *  Created on: Aug 19, 2025
 *      Author:  Xavier Halgand
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>   // pour isdigit()

#include "keyb_command_parser2.h"
#include "sequencer.h"

// Paramètres généraux

#define MAX_COMMANDS   20     // nombre max de commandes définies
#define MAX_CMD_LEN    8      // longueur max du nom de commande
#define MAX_ARGS       8      // nombre max d’arguments par commande
#define INPUT_BUFFER   32     // longueur max de la ligne saisie


//----------------- Structure de commande  ------------------------------------
struct CommandDef {
    const char name[MAX_CMD_LEN];
    int expectedArgs;
    void (*handler)(int argc, int argv[]);
};

//-----------------------------------------------------------------------------
extern EventSequencer seq;
static char input_buf[INPUT_BUFFER] = {'\0'};
static int inputIndex = 0;


//******************************   Command functions  *************************************************

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
//	Command : b<n>,<instr>		Creates n regular events in the loop for instrument #instr
//-----------------------------------------------------------------------------
void addRegPattern(int argc, int argv[]) {
    if (argc >= 1) {
        printf("b: add %d new regular events for instrument %d\r\n", argv[0], argv[1]);
        seq.AddRegularPattern(argv[0], argv[1]);
    } else {
        printf(">>>> Error.  Usage: b<number>,<instr>\r\n");
    }
}

/*************************************** Command table **************************************************/

// Table statique des commandes : {"nom de la commande", nombre d'arguments, nom de la fonction associée}
static const CommandDef commandTable[MAX_COMMANDS] = {

    { "c",	1, NewPatterns    },
    { "l",	1, NewLoop  },
    { "b",	2, addRegPattern },
    // ajouter d'autres ici...
};

/********************************************************************************************************/


/*---------------------------------------------------------------------------------------------*/
void parseCommand(const char *input) {
    char cmdName[MAX_CMD_LEN];
    int args[MAX_ARGS];
    int argc = 0;

    // --- extraire le nom de commande (jusqu’à un chiffre ou une virgule) ---
    int i = 0;
    while (input[i] != '\0' && !isdigit((unsigned char)input[i]) && input[i] != ',' && i < MAX_CMD_LEN - 1) {
        cmdName[i] = input[i];
        i++;
    }
    cmdName[i] = '\0';

    // --- extraire les nombres ---
    argc = 0;
    while (input[i] != '\0' && argc < MAX_ARGS) {
        if (isdigit((unsigned char)input[i])) {
            args[argc++] = strtol(&input[i], nullptr, 10);
            while (isdigit((unsigned char)input[i])) i++;
        }
        if (input[i] == ',') i++;
        else i++;
    }

    // --- rechercher la commande dans la table ---
    for (unsigned int c = 0; c < MAX_COMMANDS; c++) {
        if (commandTable[c].name[0] == '\0') break; // fin de table
        if (strcmp(cmdName, commandTable[c].name) == 0) {
            if (argc == commandTable[c].expectedArgs) {
                commandTable[c].handler(argc, args);	// Action !
            } else {
                printf("Erreur: commande '%s' attend %d args, recu %d\n",
                       cmdName, commandTable[c].expectedArgs, argc);
            }
            return;
        }
    }

    printf("Commande inconnue: %s\n", cmdName);
}

// --- feedChar ---
void feedChar(char c) {
    if (c == '\r' || c == '\n') {  // fin de commande
    	input_buf[inputIndex] = '\0';
        parseCommand(input_buf);
        inputIndex = 0;  // reset pour prochaine commande
    } else if (inputIndex < INPUT_BUFFER - 1) {
    	input_buf[inputIndex++] = c;
    } else {
        // buffer plein, réinitialiser
        inputIndex = 0;
    }
}
// --- Exemple d’utilisation ---
//int main() {
//    parseCommand("b3,2,5");
//    parseCommand("led1");
//    parseCommand("move10,20");
//    parseCommand("b3,2"); // Erreur: pas assez d’arguments
//}



