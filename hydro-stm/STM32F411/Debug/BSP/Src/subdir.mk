################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/Src/LCD.c \
../BSP/Src/bmp180.c \
../BSP/Src/mq135.c \
../BSP/Src/sensor_hw.c \
../BSP/Src/tds.c 

OBJS += \
./BSP/Src/LCD.o \
./BSP/Src/bmp180.o \
./BSP/Src/mq135.o \
./BSP/Src/sensor_hw.o \
./BSP/Src/tds.o 

C_DEPS += \
./BSP/Src/LCD.d \
./BSP/Src/bmp180.d \
./BSP/Src/mq135.d \
./BSP/Src/sensor_hw.d \
./BSP/Src/tds.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/Src/%.o BSP/Src/%.su BSP/Src/%.cyclo: ../BSP/Src/%.c BSP/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/Inc -I../BSP/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSP-2f-Src

clean-BSP-2f-Src:
	-$(RM) ./BSP/Src/LCD.cyclo ./BSP/Src/LCD.d ./BSP/Src/LCD.o ./BSP/Src/LCD.su ./BSP/Src/bmp180.cyclo ./BSP/Src/bmp180.d ./BSP/Src/bmp180.o ./BSP/Src/bmp180.su ./BSP/Src/mq135.cyclo ./BSP/Src/mq135.d ./BSP/Src/mq135.o ./BSP/Src/mq135.su ./BSP/Src/sensor_hw.cyclo ./BSP/Src/sensor_hw.d ./BSP/Src/sensor_hw.o ./BSP/Src/sensor_hw.su ./BSP/Src/tds.cyclo ./BSP/Src/tds.d ./BSP/Src/tds.o ./BSP/Src/tds.su

.PHONY: clean-BSP-2f-Src

