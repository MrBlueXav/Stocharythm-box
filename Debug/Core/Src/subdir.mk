################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/bruitenkor.cpp 

C_SRCS += \
../Core/Src/audio_play.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/usart.c 

C_DEPS += \
./Core/Src/audio_play.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/usart.d 

OBJS += \
./Core/Src/audio_play.o \
./Core/Src/bruitenkor.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/usart.o 

CPP_DEPS += \
./Core/Src/bruitenkor.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DAZERTY_KEYBOARD -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/STM32F4-Discovery" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/Common" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/cs43l22" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis302dl" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis3dsh" -I"D:/STM32CubeMX/SynthF4/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source/Utility" -I../PDM2PCM/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.cpp Core/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DAZERTY_KEYBOARD -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/STM32F4-Discovery" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/Common" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/cs43l22" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis302dl" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis3dsh" -I"D:/STM32CubeMX/SynthF4/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source/Utility" -I../PDM2PCM/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/audio_play.cyclo ./Core/Src/audio_play.d ./Core/Src/audio_play.o ./Core/Src/audio_play.su ./Core/Src/bruitenkor.cyclo ./Core/Src/bruitenkor.d ./Core/Src/bruitenkor.o ./Core/Src/bruitenkor.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

