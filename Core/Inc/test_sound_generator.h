/*
 * test_sound_generator.h
 *
 *  Created on: Nov 9, 2023
 *      Author: XavSab
 */

#ifndef APPLICATION_USER_TEST_SOUND_GENERATOR_H_
#define APPLICATION_USER_TEST_SOUND_GENERATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------------*/

#include <stdint.h>

/*-----------------------------------------------------------------*/
void SoundGeneratorInit(void);
void MakeSound(uint16_t *buf, uint16_t length);

/*-------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_TEST_SOUND_GENERATOR_H_ */
