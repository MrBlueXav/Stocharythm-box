/*****************************************************************************************************************
*	retarget.c
*	STM32 : retarget stdin and stdout to UART
*
*	// All credit to Carmine Noviello for this code
*	// https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-f030R8/system/src/retarget/retarget.c
*
*****************************************************************************************************************/

/*---------------------------------------------------------------------------------------------*/
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <limits.h>
#include <signal.h>
#include <../Inc/retarget.h>
#include <stdint.h>
#include <stdio.h>

/*---------------------------------------------------------------------------------------------*/
#if !defined(OS_USE_SEMIHOSTING)
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/*---------------------------------------------------------------------------------------------*/
UART_HandleTypeDef *gHuart;

/*---------------------------------------------------------------------------------------------*/
void retargetInit(UART_HandleTypeDef *huart) {
	gHuart = huart;

	/* Disable I/O buffering for STDOUT stream, so that
	 * chars are sent out as soon as they are printed. */
	setvbuf(stdout, NULL, _IONBF, 0);
}

/*---------------------------------------------------------------------------------------------*/
PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(gHuart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

GETCHAR_PROTOTYPE {
	uint8_t ch = 0;

	/* Clear the Overrun flag just before receiving the first character */
	__HAL_UART_CLEAR_OREFLAG(gHuart);

	/* Wait for reception of a character on the USART RX line and echo this
	 * character on console */
	HAL_UART_Receive(gHuart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	HAL_UART_Transmit(gHuart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

/*---------------------------------------------------------------------------------------------*/
int _write(int fd, char *ptr, int len) {
	HAL_StatusTypeDef hstatus;

	if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
		hstatus = HAL_UART_Transmit(gHuart, (uint8_t*) ptr, len, HAL_MAX_DELAY);
		if (hstatus == HAL_OK)
			return len;
		else
			return EIO;
	}
	errno = EBADF;
	return -1;
}

/*---------------------------------------------------------------------------------------------*/
int _read(int fd, char *ptr, int len) {
	HAL_StatusTypeDef hstatusR, hstatusT;

	if (fd == STDIN_FILENO) {
		hstatusR = HAL_UART_Receive(gHuart, (uint8_t*) ptr, 1, HAL_MAX_DELAY);
		hstatusT = HAL_UART_Transmit(gHuart, (uint8_t*) ptr, 1, HAL_MAX_DELAY);
		if (hstatusR == HAL_OK && hstatusT == HAL_OK)
			return 1;
		else
			return EIO;
	}
	errno = EBADF;
	return -1;
}
/*---------------------------------------------------------------------------------------------*/
#endif //#if !defined(OS_USE_SEMIHOSTING)
