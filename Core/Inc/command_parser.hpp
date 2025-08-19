/*
 * command_parser.hpp
 *
 *  Created on: Aug 10, 2025
 *
 *      Author: Xavier Halgand & ChatGPT !
 */

#ifndef INC_COMMAND_PARSER_HPP_
#define INC_COMMAND_PARSER_HPP_

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

//*********************************************************************************************
class CommandParser {

public:
	using Handler = void(*)(int argc, int argv[]);

	struct Entry {
		const char *name;
		Handler handler;
		uint8_t maxArgs;
	};

	CommandParser(const Entry *table = nullptr, size_t tableSize = 0) :
			commands(table), commandCount(tableSize), index(0) {
		buffer[0] = '\0';
	}

	void setTable(const Entry *table, size_t tableSize) {
		commands = table;
		commandCount = tableSize;
	}

	// Appelé à chaque caractère reçu
	void feedChar(char c) {
		if (c == '\r' || c == '\n') {
			if (index > 0) {
				buffer[index] = '\0';
				execute(buffer);
				index = 0;
			}
		} else if ((unsigned char) c >= 32 && index < (sizeof(buffer) - 1)) {
			buffer[index++] = c;
		}
	}

private:
	static constexpr size_t MAX_BUF = 128;
	static constexpr int MAX_PARSED_ARGS = 8;

	const Entry *commands = nullptr;
	size_t commandCount = 0;

	char buffer[MAX_BUF];
	size_t index;
//-----------------------------------------------------------------------------------------
	// Parse numbers from argsStr into out[] and return count
	int parseNumbers(const char *s, int out[], int maxArgs) {
		int cnt = 0;
		const char *p = s;
		while (*p && cnt < maxArgs) {
			// skip non-digit and non-sign
			while (*p && !((*p >= '0' && *p <= '9') || *p == '-' || *p == '+'))
				++p;
			if (!*p)
				break;
			int sign = 1;
			if (*p == '-') {
				sign = -1;
				++p;
			} else if (*p == '+') {
				++p;
			}
			int v = 0;
			bool found = false;
			while (*p >= '0' && *p <= '9') {
				found = true;
				v = v * 10 + (*p - '0');
				++p;
			}
			if (found)
				out[cnt++] = sign * v;
		}
		return cnt;
	}
//---------------------------------------------------------------------------------------
	void execute(const char *line) {
		// get command letters prefix
		size_t i = 0;
		while (line[i] && std::isalpha(static_cast<unsigned char>(line[i])))
			++i;
		if (i == 0) {
			printf("Commande vide ou invalide.\r\n");
			return;
		}

		char cmdName[32];
		size_t copyLen = (i < sizeof(cmdName) - 1) ? i : (sizeof(cmdName) - 1);
		memcpy(cmdName, line, copyLen);
		cmdName[copyLen] = '\0';

		const char *argsStr = line + i;

		// Search command table (case-sensitive). Convert to uppercase if desired.
		for (size_t k = 0; k < commandCount; ++k) {
			if (commands[k].name && strcmp(cmdName, commands[k].name) == 0) {
				int argv[MAX_PARSED_ARGS];
				int argc = parseNumbers(argsStr, argv, commands[k].maxArgs);
				// call handler
				if (commands[k].handler)
					commands[k].handler(argc, argv);
				return;
			}
		}

		printf("Commande inconnue: %s\r\n", cmdName);
	}
};

#endif // COMMAND_PARSER_HPP

