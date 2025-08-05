################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f407vgtx.s 

S_DEPS += \
./Core/Startup/startup_stm32f407vgtx.d 

OBJS += \
./Core/Startup/startup_stm32f407vgtx.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -DAZERTY_KEYBOARD -c -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/STM32F4-Discovery" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/Common" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/cs43l22" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis302dl" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/lis3dsh" -I"D:/STM32CubeMX/SynthF4/Middlewares/ST/STM32_Audio/Addons/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/Drivers/BSP/Components/PDM/Inc" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source" -I"D:/STM32CubeMX/SynthF4/DaisySP_Source/Utility" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f407vgtx.d ./Core/Startup/startup_stm32f407vgtx.o

.PHONY: clean-Core-2f-Startup

