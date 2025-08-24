/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

#include "stm32f4_discovery.h"
#include "retarget.h"
#include "ring_buffer.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/*############ Retarget printf and scanf to USART2 */
	retargetInit(&huart2);
	/*################################################ */

	/* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle) {

	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (uartHandle->Instance == USART2) {
		/* USER CODE BEGIN USART2_MspInit 0 */

		/* USER CODE END USART2_MspInit 0 */
		/* USART2 clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART2 GPIO Configuration
		 PA2     ------> USART2_TX
		 PA3     ------> USART2_RX
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USART2 interrupt Init */
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		/* USER CODE BEGIN USART2_MspInit 1 */

		/* USER CODE END USART2_MspInit 1 */
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle) {

	if (uartHandle->Instance == USART2) {
		/* USER CODE BEGIN USART2_MspDeInit 0 */

		/* USER CODE END USART2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART2_CLK_DISABLE();

		/**USART2 GPIO Configuration
		 PA2     ------> USART2_TX
		 PA3     ------> USART2_RX
		 */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

		/* USART2 interrupt Deinit */
		HAL_NVIC_DisableIRQ(USART2_IRQn);
		/* USER CODE BEGIN USART2_MspDeInit 1 */

		/* USER CODE END USART2_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

// Buffers RX et TX ********************************************
//static char rx_storage[128];
static char tx_storage[2048];
//static ring_buffer_t rx_rb;
static ring_buffer_t tx_rb;

// Variable temporaire pour RX
//static uint8_t rx_byte;

/*-----------------------------------------------------------------------------------------------*/
void uart_app_init(void) {
	//rb_init(&rx_rb, rx_storage, sizeof(rx_storage));
	rb_init(&tx_rb, tx_storage, sizeof(tx_storage));

	// Lancer la réception d’un octet
	//HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

/*-----------------------------------------------------------------------------------------------*/
bool uart_send(const char *s) {
	bool kickstart = rb_is_empty(&tx_rb);  // faut-il lancer l’IT ?

	// Mettre la chaîne dans le buffer
	while (*s) {
		while (!rb_push(&tx_rb, *s)) { 		// Tant que le buffer est plein...
			HAL_Delay(1);					// on attend.
			//return false; // buffer plein
		}
		s++;
	}

	// Si l’UART était inactif, on démarre la première émission
	if (kickstart) {
		char c;
		if (rb_pop(&tx_rb, &c)) {
			HAL_UART_Transmit_IT(&huart2, (uint8_t*) &c, 1);
		}
	}

	return true;
}

/*-----------------------------------------------------------------------------------------------*/
void uart_printf(const char *fmt, ...) {
	char buf[128]; // buffer temporaire pour la chaîne formatée
	va_list args;
	va_start(args, fmt);

	// Génère la chaîne formatée
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	if (len > 0) {
		// S’assurer de ne pas dépasser la taille du buffer
		if (len > sizeof(buf))
			len = sizeof(buf);

		// Envoie via les blocs UART
		//uart_queue_data((uint8_t*) buf, len);
		bool ok = uart_send(buf);
		if (!ok)
			BSP_LED_On(LED5);		// red
	}
}

/*******************************************************************************************************************************/
void uart_IT_Test(void) {

	//uint8_t test3[] = ">>>  <<< ####*******  test uart_queue_data(...)  azertyuiop\r\n";
	for (int ct = 0; ct < 20; ct++) {
		uart_printf(">>> %d <<< ####*******  test uart_printf  azertyuiop\r\n", ct + 1);
		//uart_queue_data(test3, sizeof(test3) - 1);
	}
}

/*-----------------------------------------------------------------------------------------------*/

/**
 * @brief  Tx Transfer completed callbacks.
 * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
 *                the configuration information for the specified UART module.
 * @retval None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {

		char c;
		if (rb_pop(&tx_rb, &c)) {
			// Envoyer le suivant
			HAL_UART_Transmit_IT(huart, (uint8_t*) &c, 1);
		}
	}
}


// Event : position = 302 || type = 0X59 || data1 = 117 || data2 = 108 || data3 = 100 ||
/* USER CODE END 1 */
