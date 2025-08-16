/******************************************************************************************/
// 					azerty_hid_map.h
//	Langage C
//	Encodage fichier : UTF-8
//	Table de correspondance HID USB -> caractères AZERTY FR (séquences \xNN pour Latin-15)
//
//	Xavier Halgand & ChatGPT !
//
/******************************************************************************************/
#ifndef AZERTY_HID_MAP_H
#define AZERTY_HID_MAP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct  {
    uint8_t hid_code;
    uint8_t normal;   // sans modificateur
    uint8_t shift;    // avec Shift
    uint8_t altGr;    // avec AltGr (Right Alt)
} KeyMapEntry;

// NOTE: les valeurs \xNN sont des octets à placer dans un fichier source ASCII.
// Ils correspondent aux codes de la page Latin (ISO-8859-1/15) pour les lettres accentuées et autres caractères spéciaux.
static const KeyMapEntry azertyKeyMap[] = {
    // Lettres (HID 0x04..0x1D)
    {0x14, 'a', 'A', 0},
    {0x1A, 'z', 'Z', 0},
    {0x08, 'e', 'E', '\xA4'},	//  e  E  €
    {0x15, 'r', 'R', 0},
    {0x17, 't', 'T', 0},
    {0x1C, 'y', 'Y', 0},
    {0x18, 'u', 'U', 0},
    {0x0C, 'i', 'I', 0},
    {0x12, 'o', 'O', 0},
    {0x13, 'p', 'P', 0},
    {0x2F, '^', '\xEF', 0}, // '^'  ï
    {0x30, '$', '\xA3', '\xC9'}, // '$' ; Shift -> '£' (pound) \xA3    É

    {0x04, 'q', 'Q', '@'},  // AltGr -> '@'
    {0x16, 's', 'S', 0},
    {0x07, 'd', 'D', 0},
    {0x09, 'f', 'F', 0},
    {0x0A, 'g', 'G', 0},
    {0x0B, 'h', 'H', 0},
    {0x0D, 'j', 'J', 0},
    {0x0E, 'k', 'K', 0},
    {0x0F, 'l', 'L', 0},
    {0x33, 'm', 'M', 0},
    {0x34, '\xF9', '%', 0}, // 'ù' = \xF9

    {0x31, '*', '\xB5', 0}, // '*' ; Shift -> micro 'µ' (\xB5)

    {0x1D, 'w', 'W', 0},
    {0x1B, 'x', 'X', 0},
	{0x06, 'c', 'C', 0},
	{0x19, 'v', 'V', 0},
	{0x05, 'b', 'B', 0},
	{0x11, 'n', 'N', 0},
	{0x10, ',', '?', 0},
	{0x36, ';', '.', 0},
	{0x37, ':', '/', 0},
	{0x38, '!', '\xA7', 0},

    // Top row: HID 0x1E .. 0x27 (1..0 keys). Correct mapping AZERTY FR :
    // sans Shift : &  é  "  '  (  -  è  _  ç  à
    // avec Shift  : 1  2  3  4  5  6  7  8  9  0
	{0x35, '\xB2', 0, 0},
    {0x1E, '&', '1', 0},
    {0x1F, '\xE9', '2', '~'},  // 'é' = \xE9
    {0x20, '\"', '3', '#'},
    {0x21, '\'', '4', '{'},
    {0x22, '(', '5', '['},
    {0x23, '-', '6', '|'},
    {0x24, '\xE8', '7', '`'},  // 'è' = \xE8
    {0x25, '_', '8', '\\'},
    {0x26, '\xE7', '9', '^'},  // 'ç' = \xE7
    {0x27, '\xE0', '0', '@'},  // 'à' = \xE0

    // HID 0x28 is Enter (see below)
    // Some punctuation keys (US layout positions, adjusted for AZERTY):
    {0x28, '\n', '\n', '\n'}, // Enter
    {0x2C, ' ', ' ', ' '},    // Space
    {0x2B, '\t', '\t', '\t'}, // Tab (note: HID 0x2B used earlier in some layouts too)

    // Other punctuation / keys (common AZERTY placements)
    {0x2D, ')', '\xB0', ']'},   // ')' ; Shift -> '°' (\xB0)
    {0x2E, '=', '+', '}'},      // '=' ; Shift -> '+'
    {0x64, '<', '>', 0},      // '<' key (non-US)
    // NOTE: layouts vary; adapte si ton clavier diffère.

	{0x52, '(', 0, 0},	// flèche haute redirigée vers (
	{0x51, ')', 0, 0},	// flèche basse redirigée vers )

    // Numeric keypad (HID 0x53 .. 0x64) - NumLock ON typical values
	{0x53,   0,   0, 0},	// Touche Verr Num
	{0x54, '/', '/', 0},
	{0x55, '*', '*', 0},
	{0x56, '-', '-', 0},
	{0x57, '+', '+', 0},
	{0x58, '\n', '\n', 0},
    {0x59, '1', '1', 0},
    {0x5A, '2', '2', 0},
    {0x5B, '3', '3', 0},
    {0x5C, '4', '4', 0},
    {0x5D, '5', '5', 0},
    {0x5E, '6', '6', 0},
    {0x5F, '7', '7', 0},
    {0x60, '8', '8', 0},
    {0x61, '9', '9', 0},
    {0x62, '0', '0', 0},
    {0x63, '.', '.', 0},

};

static const size_t azertyKeyMapSize = sizeof(azertyKeyMap) / sizeof(KeyMapEntry);

// Traduit code HID + modificateurs -> caractère (octet Latin-15 via \xNN si nécessaire)
uint8_t translateHIDtoChar(uint8_t hid_code, bool shift, bool altGr)
{
    for (size_t i = 0; i < azertyKeyMapSize; ++i) {
        if (azertyKeyMap[i].hid_code == hid_code) {
            if (altGr && azertyKeyMap[i].altGr != 0) return azertyKeyMap[i].altGr;
            if (shift) return azertyKeyMap[i].shift;
            return azertyKeyMap[i].normal;
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif // AZERTY_HID_MAP_H
