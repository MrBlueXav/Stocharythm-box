################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_HOST/App/usb_host.c 

C_DEPS += \
./USB_HOST/App/usb_host.d 

OBJS += \
./USB_HOST/App/usb_host.o 


# Each subdirectory must supply rules for building sources it contributes
USB_HOST/App/%.o USB_HOST/App/%.su USB_HOST/App/%.cyclo: ../USB_HOST/App/%.c USB_HOST/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../USB_HOST/App -I../USB_HOST/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/STM32F4-Discovery" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/Common" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/cs43l22" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis302dl" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis3dsh" -I"D:/STM32CubeMX/SynthF4/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_HOST-2f-App

clean-USB_HOST-2f-App:
	-$(RM) ./USB_HOST/App/usb_host.cyclo ./USB_HOST/App/usb_host.d ./USB_HOST/App/usb_host.o ./USB_HOST/App/usb_host.su

.PHONY: clean-USB_HOST-2f-App

