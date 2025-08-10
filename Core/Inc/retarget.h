/*****************************************************************************************************************
*	retarget.h 
*	STM32 : retarget stdin and stdout to UART
*
*	// All credit to Carmine Noviello for this code
* // https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-f030R8/system/src/retarget/retarget.c
*
*****************************************************************************************************************/


/*--------------------------------------------------------------*/
#ifndef INC_RETARGET_H_
#define INC_RETARGET_H_

/*--------------------------------------------------------------*/
#include "main.h"
#include <sys/stat.h>
#include "usart.h"

/*--------------------------------------------------------------*/
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif

/*--------------------------------------------------------------*/
void retargetInit(UART_HandleTypeDef *huart);
int _write(int fd, char* ptr, int len);
int _read(int fd, char* ptr, int len);

/*--------------------------------------------------------------*/
#endif /* INC_RETARGET_H_ */
