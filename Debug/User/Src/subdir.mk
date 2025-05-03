################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Src/analog_signal.c \
../User/Src/fsbb_pwm.c 

OBJS += \
./User/Src/analog_signal.o \
./User/Src/fsbb_pwm.o 

C_DEPS += \
./User/Src/analog_signal.d \
./User/Src/fsbb_pwm.d 


# Each subdirectory must supply rules for building sources it contributes
User/Src/%.o User/Src/%.su User/Src/%.cyclo: ../User/Src/%.c User/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"I:/RM files/SuperCap/FSBB_code/fsbb/User/Inc" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-User-2f-Src

clean-User-2f-Src:
	-$(RM) ./User/Src/analog_signal.cyclo ./User/Src/analog_signal.d ./User/Src/analog_signal.o ./User/Src/analog_signal.su ./User/Src/fsbb_pwm.cyclo ./User/Src/fsbb_pwm.d ./User/Src/fsbb_pwm.o ./User/Src/fsbb_pwm.su

.PHONY: clean-User-2f-Src

