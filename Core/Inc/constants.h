/**
 ******************************************************************************
 * File Name          : constants.h
 * Author			  : Xavier Halgand
 * Date               :	2024
 * Description        : Global defines for Dekrispator
 ******************************************************************************
 */
#ifndef __DEKR_CONSTANTS_H__
#define __DEKR_CONSTANTS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************************************************/
#define AUDIO_BUFFER_SIZE       1024 /* AUDIO_BUFFER_SIZE (in bytes) must be a multiple of 8   */

#define SAMPLERATE              48000 // Don't modify !

#define Ts						(1.f/SAMPLERATE)  // sample period
#define _2PI                    6.283185307f
#define _PI                    	3.14159265f

#define VOL                     60 // initial output DAC volume
#define MAXVOL                  100 // maximal output DAC volume

#define MIDI_MAX				127.f 	// floating max value
#define MIDI_MAXi				127		// integer max value
#define MIDI_MID_i				64		// integer mid value

#define USE_THE_LCD				1

/*****************************************************************************************************************/
//#define _DTCMRAM_				__attribute__((section(".DTCMRAM_section_bss")))
//#define _SDRAM_					__attribute__((section(".sdram_bss")))

//#define _ITCMRAM_				__attribute__((section(".itcm_text")))
#define	_ITCMRAM_
#define _DTCMRAM_

/******************************************************************************************************************/
/* Align X to 4 bytes */
#define MEM_ALIGN(x)			(((x) + 0x00000003) & ~(0x00000003))

/******************************************************************************************************************/
#if defined   (__GNUC__)        /* GNU Compiler */
    #define __ALIGN    __attribute__ ((aligned (4)))
#endif /* __GNUC__ */

/******************************************************************************************************************/

#ifdef __cplusplus
}
#endif

/************************************************************************************/
#endif  /*__DEKR_CONSTANTS_H__ */
