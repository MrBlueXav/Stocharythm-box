/*
 * keyb_command_parser2.cpp
 *
 *  Created on: Aug 19, 2025
 *      Author:  Xavier Halgand
 */

#include <keyb_command_parser.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>   // pour isdigit()

#include "sequencer.h"

// Paramètres généraux

#define MAX_COMMANDS   20     // nombre max de commandes définies
#define MAX_CMD_LEN    15      // longueur max du nom de commande
#define MAX_ARGS       8      // nombre max d’arguments par commande
#define INPUT_BUFFER   32     // longueur max de la ligne saisie

//----------------- Structure de commande  ------------------------------------
struct Command {
    const char* name;
    int expectedArgs;
    void (*func)(int*, int);
};

//-----------------------------------------------------------------------------
extern EventSequencer seq;
static char input_buf[INPUT_BUFFER] = { '\0' };
static int inputIndex = 0;
bool multikey = false;

//******************************   Command functions  *************************************************

//-----------------------------------------------------------------------------
//	Command : ca<n>		Ex : ca6 followed by "enter" key -> creates 6 events for each instrument
//-----------------------------------------------------------------------------
void addGeneralPattern(int* argv, int argc) {
	if (argc == 1) {
		printf("ca: create %d new events for each instrument\r\n", argv[0]);
		seq.AddGeneralPattern(argv[0]);
	} else {
		printf(">>>> Error.  Usage: ca<number>\r\n");
	}
}

//-----------------------------------------------------------------------------
//	Command : cn<n>		Ex : cn6 followed by "enter" key -> creates 6 events
//-----------------------------------------------------------------------------
void newPattern(int* argv, int argc) {
	if (argc == 1) {
		printf("cn : create %d new events\r\n", argv[0]);
		seq.CreateEvents(argv[0]);
	} else {
		printf(">>>> Error.  Usage: cn<number>\r\n");
	}
}

//-----------------------------------------------------------------------------
//	Command :  cl<n>		Sets the loop duration at n * 0.1 seconds
//-----------------------------------------------------------------------------
void newLoop(int* argv, int argc) {
	if (argc == 1) {
		printf("cl : create new loop : %d units\r\n", argv[0]);
		seq.NewLoop(argv[0]);
	} else {
		printf(">>>> Error.  Usage: cl<number>\r\n");
	}
}

//-----------------------------------------------------------------------------
//	Command : cr<n>,<instr>		Creates n regular events in the loop for instrument #instr
//-----------------------------------------------------------------------------
void addRegPattern(int* argv, int argc) {
	if (argc == 2) {
		printf("cr : add %d new regular events for instrument %d\r\n", argv[0],
				argv[1]);
		seq.AddRegularPattern(argv[0], argv[1]);
	} else {
		printf(">>>> Error.  Usage: cr<number>,<instr>\r\n");
	}
}

//-----------------------------------------------------------------------------
//	Command : cq<n>		quantize position of all events at 1/n of loop
//-----------------------------------------------------------------------------
void quantize(int* argv, int argc) {
	if (argc == 1) {
		printf("cq : quantize events on grid : 1/%d\r\n", argv[0]);
		seq.Quantize(argv[0]);
	} else {
		printf(">>>> Error.  Usage: cq<number>\r\n");
	}
}

/*************************************** Command table **************************************************/

// Table statique des commandes : {"nom de la commande", nombre d'arguments, nom de la fonction associée}
static const Command commandTable[MAX_COMMANDS] = {

{ "cn", 1, newPattern },
{ "ca", 1, addGeneralPattern },
{ "cl", 1, newLoop },
{ "cr", 2, addRegPattern },
{ "cq", 1, quantize },
// ajouter d'autres ici...
		};

constexpr size_t commandTableSize = sizeof(commandTable) / sizeof(commandTable[0]);

/********************************************************************************************************/

/*---------------------------------------------------------------------------------------------*/
void parseCommand(const char *input) {

	char cmdName[MAX_CMD_LEN] = { 0 };
	int args[MAX_ARGS] = { 0 };
	int argc = 0;

	size_t i = 0, j = 0;

	// --- Étape 1 : Extraire le nom de commande ---
	while (input[i] != '\0'
			&& !isdigit((unsigned char) input[i])
			&& input[i] != ','
			&& j < MAX_CMD_LEN - 1) {
		cmdName[j++] = input[i++];
	}
	cmdName[j] = '\0';  // Fin de chaîne OK ✅

	if (j == 0) {
		printf("Erreur: commande vide\n");
		return;
	}

	// --- Étape 2 : Extraction des arguments ---
	while (input[i] != '\0' && argc < MAX_ARGS) {
		if (isdigit((unsigned char) input[i])) {
			char *endptr;
			long val = strtol(&input[i], &endptr, 10); // ✅ conversion sûre
			args[argc++] = (int) val;
			i = endptr - input;
		}
		if (input[i] == ',') {
			i++; // avance sur la virgule
		} else if (!isdigit((unsigned char) input[i]) && input[i] != '\0') {
			printf("Caractère inattendu: '%c'\n", input[i]);
			return;
		}
	}

	// --- Étape 3 : Recherche dans la table ---
	for (size_t k = 0; k < commandTableSize; k++) {
		if (strcmp(cmdName, commandTable[k].name) == 0) {
			if (argc == commandTable[k].expectedArgs) {
				commandTable[k].func(args, argc);
			} else {
				printf("Erreur: %s attend %d arguments, recu %d\n", cmdName,
						commandTable[k].expectedArgs, argc);
			}
			return;
		}
	}

	printf("Commande inconnue: %s\n", cmdName);
}

/*---------------------------------------------------------------------------------------------*/
void feedChar(char c) {
	if (c == '\r' || c == '\n') {  // fin de commande
		input_buf[inputIndex] = '\0';
		parseCommand(input_buf);
		inputIndex = 0;  // reset pour prochaine commande
		multikey = false;
	} else if (inputIndex < INPUT_BUFFER - 1) {
		input_buf[inputIndex++] = c;
	} else {
		// buffer plein, réinitialiser
		inputIndex = 0;
		multikey = false;
	}
}
// --- Exemple d’utilisation ---
//int main() {
//    parseCommand("b3,2,5");
//    parseCommand("led1");
//    parseCommand("move10,20");
//    parseCommand("b3,2"); // Erreur: pas assez d’arguments
//}

