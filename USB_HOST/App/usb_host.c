/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file            : usb_host.c
 * @version         : v1.0_Cube
 * @brief           : This file implements the USB Host
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

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_hid.h"

/* USER CODE BEGIN Includes */
#include "stm32f4_discovery.h"
#include "bruitenkor.h"
#include "azerty_hid_map.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */
void USBH_HID_EventCallback(USBH_HandleTypeDef *phost) {

	HID_KEYBD_Info_TypeDef *keybd_info;

	keybd_info = USBH_HID_GetKeybdInfo(phost);
	//uint8_t key = USBH_HID_GetASCIICode(keybd_info);
	bool shift = (keybd_info->lshift || keybd_info->rshift);
	bool altGr = (keybd_info->ralt && !shift); // AltGr only if Right Alt and not shift
	uint8_t key_code = keybd_info->keys[0];
	char ext_key = translateHIDtoChar(key_code, shift, altGr);

//	printf("| state = %d | ", keybd_info->state);
//	printf("lctrl = %d | ", keybd_info->lctrl);
//	printf("lshift = %d | ", keybd_info->lshift);
//	printf("lalt = %d | ", keybd_info->lalt);
//	printf("lgui = %d || ", keybd_info->lgui);
//	printf("rctrl = %d | ", keybd_info->rctrl);
//	printf("rshift = %d | ", keybd_info->rshift);
//	printf("ralt = %d | ", keybd_info->ralt);
//	printf("rgui = %d ||\r\n ", keybd_info->rgui);
//	printf("keys = [ 0x%.2X, 0x%.2X, 0x%.2X, 0x%.2X, 0x%.2X 0x%.2X, ]\r\n", keybd_info->keys[0], keybd_info->keys[1],keybd_info->keys[2],keybd_info->keys[3],keybd_info->keys[4],keybd_info->keys[5]);

	if (keybd_info->keys[0] != 0) {
		//printf("||   key code = 0x%.2X   ||  keys_fr = %c ||\r\n", key_code, ext_key);
		InterpretKey(ext_key, key_code);
	}


	//printf("----------------------------------------------------\r\n");
}
/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_HID_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostFS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
	switch (id) {
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_DISCONNECTION:
		Appli_state = APPLICATION_DISCONNECT;
		BSP_LED_Off(LED6);
		break;

	case HOST_USER_CLASS_ACTIVE:
		Appli_state = APPLICATION_READY;
		BSP_LED_On(LED6);
		break;

	case HOST_USER_CONNECTION:
		Appli_state = APPLICATION_START;
		BSP_LED_Off(LED6);
		break;

	default:
		break;
	}
  /* USER CODE END CALL_BACK_1 */
}

/**
  * @}
  */

/**
  * @}
  */

