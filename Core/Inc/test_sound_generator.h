/*
 * test_sound_generator.h
 *
 *  Created on: Nov 9, 2023
 *	Modified : 26/07/2025
 *  Author: Xavier Halgand
 *---------------------------------------------------------------------*/

#ifndef APPLICATION_USER_TEST_SOUND_GENERATOR_H_
#define APPLICATION_USER_TEST_SOUND_GENERATOR_H_

/*-----------------------------------------------------------------------------------*/

#include <stdint.h>

/*-----------------------------------------------------------------*/
void SoundGeneratorInit(void);
void make_test_sound(uint16_t *buf, uint16_t length);

/*-------------------------------------------------------------------------------------*/

#endif /* APPLICATION_USER_TEST_SOUND_GENERATOR_H_ */
