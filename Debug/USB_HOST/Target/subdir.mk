################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_HOST/Target/usbh_conf.c 

C_DEPS += \
./USB_HOST/Target/usbh_conf.d 

OBJS += \
./USB_HOST/Target/usbh_conf.o 


# Each subdirectory must supply rules for building sources it contributes
USB_HOST/Target/%.o USB_HOST/Target/%.su USB_HOST/Target/%.cyclo: ../USB_HOST/Target/%.c USB_HOST/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DAZERTY_KEYBOARD -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/STM32F4-Discovery" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/Common" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/cs43l22" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis302dl" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis3dsh" -I"D:/STM32CubeMX/SynthF4/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source/Utility" -I../PDM2PCM/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_HOST-2f-Target

clean-USB_HOST-2f-Target:
	-$(RM) ./USB_HOST/Target/usbh_conf.cyclo ./USB_HOST/Target/usbh_conf.d ./USB_HOST/Target/usbh_conf.o ./USB_HOST/Target/usbh_conf.su

.PHONY: clean-USB_HOST-2f-Target

