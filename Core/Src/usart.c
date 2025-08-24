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
#include "retarget.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
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

// ----------------------------
// Configuration des buffers
#define NB_BUFFERS 128   // Nombre de blocs disponibles
#define BUF_SIZE   16  // Taille de chaque bloc en octets

// Structure d’un bloc : données + longueur réelle
typedef struct {
	uint8_t data[BUF_SIZE];  // Données à transmettre
	uint8_t len;             // Nombre d’octets valides dans ce bloc
} tx_block_t;

// Tableau de blocs pour la file d’attente UART
tx_block_t tx_buffers[NB_BUFFERS];

// Index pour écrire/ajouter un bloc
volatile uint8_t tx_write_idx = 0;

// Index pour lire/envoyer un bloc
volatile uint8_t tx_read_idx = 0;

// Nombre de blocs remplis dans la file
volatile uint8_t tx_count = 0;

// Position dans le bloc courant (0..len-1)
volatile uint8_t tx_pos = 0;

// Flag indiquant qu’un bloc est en cours d’envoi
volatile uint8_t sending = 0;

// ----------------------------
// Fonction pour ajouter des données à la file UART
// Peut recevoir un flux de n’importe quelle taille
void uart_queue_data(const uint8_t *data, uint16_t size) {
	uint16_t offset = 0; // Position dans le flux à envoyer

	// Tant qu’il reste des octets à envoyer
	while (offset < size) {
		// Si la file est pleine, on s’arrête
		if (tx_count >= NB_BUFFERS)
			break;

		tx_block_t *blk = (tx_block_t*) &tx_buffers[tx_write_idx];

		// Nombre d’octets à mettre dans ce bloc (max BUF_SIZE)
		uint8_t space = BUF_SIZE;
		blk->len = (size - offset > space) ? space : (size - offset);

		// Copier les octets dans le bloc
		for (int i = 0; i < blk->len; i++)
			blk->data[i] = data[offset + i];

		// Avancer dans le flux et dans la file
		offset += blk->len;
		tx_write_idx = (tx_write_idx + 1) % NB_BUFFERS;
		tx_count++;
	}
}

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
		uart_queue_data((uint8_t*) buf, len);
	}
}

// Appelée par HAL_TIM_PeriodElapsedCallback(...)
void transmit_UART_block(void) {
	if (!sending && tx_count > 0) {
		tx_block_t *blk = &tx_buffers[tx_read_idx];
		sending = 1;
		HAL_UART_Transmit_IT(&huart2, blk->data, blk->len);
	}
}
/*******************************************************************************************************************************/
//void uart2_printf(const char *fmt, ...) {
//
//	va_list args;
//	va_start(args, fmt);
//	int len = vsnprintf(buff, sizeof(buff), fmt, args);
//	va_end(args);
//
//	if (len > 0) {
//		// Attendre que la transmission précédente soit finie
//		while (uart2_tx_busy) {
//			// on peut mettre __NOP(); ici
//		}
//		uart2_tx_busy = 1;
//		if (HAL_UART_Transmit_IT(&huart2, (uint8_t*) buff, (uint16_t) len) != HAL_OK) {
//			uart2_tx_busy = 0; // libère si erreur
//		}
//	}
//	va_list args;
//	va_start(args, fmt);
//	int len = vsnprintf(buff, sizeof(buff), fmt, args);
//	va_end(args);
//	if (len > 0) {
//		for (;;) {
//			HAL_StatusTypeDef s = HAL_UART_Transmit_IT(&huart2, (uint8_t*) buff, (uint16_t) len);
//			if (s == HAL_ERROR)
//				Error_Handler();
//			if (s == HAL_OK)
//				break;
//		}
//	}
//}
void uart_IT_Test(void) {

	//uint8_t test3[] = ">>>  <<< ####*******  test uart_queue_data(...)  azertyuiop\r\n";
	for (int ct = 0; ct < 20; ct++) {
		uart_printf(">>> %d <<< ####*******  test uart_printf  azertyuiop\r\n", ct + 1);
		//uart_queue_data(test3, sizeof(test3) - 1);
	}
}
/**
 * @brief  Tx Transfer completed callbacks.
 * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
 *                the configuration information for the specified UART module.
 * @retval None
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {

		// Bloc terminé → passer au bloc suivant
		tx_read_idx = (tx_read_idx + 1) % NB_BUFFERS;
		tx_count--;
		sending = 0;
	}
}
/* USER CODE END 1 */
