/*
 * bruitenkor.h
 *
 *  Created on: Aug 5, 2025
 *      Author: Xavier Halgand
 */

#ifndef INC_BRUITENKOR_H_
#define INC_BRUITENKOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>


/*-----------------------------------------------------------------------------------------------------------*/
void InterpretKey(uint8_t key);
void SoundGeneratorInit(void);
void MakeSound(uint16_t *buf, uint16_t length);
void PrintALine(void);

/*-----------------------------------------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif /* INC_BRUITENKOR_H_ */
