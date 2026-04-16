/**
 ******************************************************************************
 * @file    audio_play.c
 * @author  Xavier Halgand
 * @brief   for STM32F4 Discovery kit
 * july 2025
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include "audio_play.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery_audio.h"
#include "constants.h"
#include "bruitenkor.h"

char string_message[100];

/* Private typedef -----------------------------------------------------------*/
typedef enum {
	AUDIO_ERROR_NONE = 0, AUDIO_ERROR_NOTREADY, AUDIO_ERROR_IO, AUDIO_ERROR_EOF,

} AUDIO_ErrorTypeDef;

typedef enum {
	AUDIO_STATE_IDLE = 0,
	AUDIO_STATE_INIT,
	AUDIO_STATE_PLAYING,
	AUDIO_STATE_PAUSE

} AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL,

} BUFFER_StateTypeDef;

/* Private variables ---------------------------------------------------------*/
static uint16_t audio_buffer[AUDIO_BUFFER_SIZE]; // AUDIO_BUFFER_SIZE is defined in constants.h
static volatile _DTCMRAM_ BUFFER_StateTypeDef state;
static _DTCMRAM_ AUDIO_PLAYBACK_StateTypeDef audio_state;
static bool sound = true;
static volatile uint32_t uwVolume;

/*---------Functions ---------------------------------------------------------*/

/*************************************************************************************
 * @brief  MODIFIED AUDIO OUT I2S MSP Init function for Circular DMA Mode !
 * @param  hi2s: might be required to set audio peripheral predivider if any.
 * @param  Params : pointer on additional configuration parameters, can be NULL.
 ************************************************************************************/
void BSP_AUDIO_OUT_MspInit(I2S_HandleTypeDef *hi2s, void *Params) {
	static DMA_HandleTypeDef hdma_i2sTx;
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable I2S3 clock */
	I2S3_CLK_ENABLE();

	/*** Configure the GPIOs ***/
	/* Enable I2S GPIO clocks */
	I2S3_SCK_SD_CLK_ENABLE();
	I2S3_WS_CLK_ENABLE();

	/* I2S3 pins configuration: WS, SCK and SD pins ----------------------------*/
	GPIO_InitStruct.Pin = I2S3_SCK_PIN | I2S3_SD_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = I2S3_SCK_SD_WS_AF;
	HAL_GPIO_Init(I2S3_SCK_SD_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = I2S3_WS_PIN;
	HAL_GPIO_Init(I2S3_WS_GPIO_PORT, &GPIO_InitStruct);

	/* I2S3 pins configuration: MCK pin */
	I2S3_MCK_CLK_ENABLE();
	GPIO_InitStruct.Pin = I2S3_MCK_PIN;
	HAL_GPIO_Init(I2S3_MCK_GPIO_PORT, &GPIO_InitStruct);

	/* Enable the I2S DMA clock */
	I2S3_DMAx_CLK_ENABLE();

	if (hi2s->Instance == I2S3) {
		/* Configure the hdma_i2sTx handle parameters */
		hdma_i2sTx.Init.Channel = I2S3_DMAx_CHANNEL;
		hdma_i2sTx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_i2sTx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_i2sTx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_i2sTx.Init.PeriphDataAlignment = I2S3_DMAx_PERIPH_DATA_SIZE;
		hdma_i2sTx.Init.MemDataAlignment = I2S3_DMAx_MEM_DATA_SIZE;
		/*=======================================================================*/
		hdma_i2sTx.Init.Mode = DMA_CIRCULAR;
		/*=======================================================================*/
		hdma_i2sTx.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_i2sTx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
		hdma_i2sTx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
		hdma_i2sTx.Init.MemBurst = DMA_MBURST_SINGLE;
		hdma_i2sTx.Init.PeriphBurst = DMA_PBURST_SINGLE;

		hdma_i2sTx.Instance = I2S3_DMAx_STREAM;

		/* Associate the DMA handle */
		__HAL_LINKDMA(hi2s, hdmatx, hdma_i2sTx);

		/* Deinitialize the Stream for new transfer */
		HAL_DMA_DeInit(&hdma_i2sTx);

		/* Configure the DMA Stream */
		HAL_DMA_Init(&hdma_i2sTx);
	}

	/* I2S DMA IRQ Channel configuration */
	HAL_NVIC_SetPriority(I2S3_DMAx_IRQ, AUDIO_OUT_IRQ_PREPRIO, 0);
	HAL_NVIC_EnableIRQ(I2S3_DMAx_IRQ);
}

void AudioInit(void) {

	uwVolume = VOL;

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, uwVolume, SAMPLERATE) != AUDIO_OK) {
		Error_Handler();
	}
	HAL_Delay(500);

	/*
	 Start playing the file from a circular buffer, once the DMA is enabled, it is
	 always in running state. Application has to fill the buffer with the audio data
	 using Transfer complete and/or half transfer complete interrupts callbacks
	 (AUDIO_TransferComplete_CallBack() or AUDIO_HalfTransfer_CallBack()...
	 */
	state = BUFFER_OFFSET_NONE;
	BSP_AUDIO_OUT_Play(&audio_buffer[0], 2 * AUDIO_BUFFER_SIZE);
	audio_state = AUDIO_STATE_PLAYING;
}

/*----------------------------------------------------------------------------------------------------*/
uint8_t _ITCMRAM_ Process_audio(void) {
	AUDIO_ErrorTypeDef error_state = AUDIO_ERROR_NONE;

	switch (audio_state) {
	case AUDIO_STATE_PLAYING:

		/* 1st half buffer played; so fill it and continue playing from bottom*/
		if (state == BUFFER_OFFSET_HALF) {

			BSP_LED_Off(LED3); // CPU load indicator
			MakeSound(&audio_buffer[0], AUDIO_BUFFER_SIZE / 4);
			state = BUFFER_OFFSET_NONE;
			BSP_LED_On(LED3);
		}

		/* 2nd half buffer played; so fill it and continue playing from top */
		if (state == BUFFER_OFFSET_FULL) {

			BSP_LED_Off(LED3); // CPU load indicator
			MakeSound(&audio_buffer[AUDIO_BUFFER_SIZE / 2],
			AUDIO_BUFFER_SIZE / 4);
			state = BUFFER_OFFSET_NONE;
			BSP_LED_On(LED3);
		}

		break;

	default:
		error_state = AUDIO_ERROR_NOTREADY;
		break;
	}

	return (uint8_t) error_state;
}

/*------------------------------------------------------------------------------
 Callbacks implementation:
 the callbacks API are defined __weak in the stm32F4_discovery_audio.c file
 and their implementation should be done the user code if they are needed.
 ----------------------------------------------------------------------------*/
/**
 * @brief  Manages the full Transfer complete event.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	if (audio_state == AUDIO_STATE_PLAYING) {
		/* allows Process_audio() to refill 2nd part of the buffer  */
		state = BUFFER_OFFSET_FULL;
	}
}
/*----------------------------------------------------------------------------------------------------*/
/**
 * @brief  Manages the DMA Half Transfer complete event.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	if (audio_state == AUDIO_STATE_PLAYING) {
		/* allows Process_audio() to refill 1st part of the buffer  */
		state = BUFFER_OFFSET_HALF;
	}
}
/*----------------------------------------------------------------------------------------------------*/
/**
 * @brief  Manages the DMA FIFO error event.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_Error_CallBack(void) {
	BSP_LED_On(LED5);
}

//--------------------------------- toggle ON/OFF volume ------------------------------------------
void toggleSound(void) {
	if (!sound) {
		//pitchGenResetPhase();
		BSP_AUDIO_OUT_SetVolume(uwVolume);
		sound = true;
	} else {
		BSP_AUDIO_OUT_SetVolume(0);
		sound = false;
	}
}
//------------------------------- increase output DAC volume --------------------------------------------
void incVol(void) {
	if (uwVolume < MAXVOL) {
		uwVolume++;
		BSP_AUDIO_OUT_SetVolume(uwVolume);
	}
	printf("Volume is now : %lu\r\n", uwVolume);
	//send_string_to_CM4(string_message);
}

//-------------------------------- decrease output DAC volume ------------------------------------------
void decVol(void) {
	if (uwVolume > 0) {
		uwVolume--;
		BSP_AUDIO_OUT_SetVolume(uwVolume);
	}
	printf("Volume is now : %lu\r\n", uwVolume);
	//send_string_to_CM4(string_message);
}

//------------------------------------------------------------------------------------------------------
void Volume_set(uint8_t val) {
	uwVolume = (uint8_t) (MAXVOL / MIDI_MAX * val);
	BSP_AUDIO_OUT_SetVolume(uwVolume);
}

/*-------------------------------- END OF FILE -------------------------------------------------------------------------*/
