################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/actuator_manager.c \
../App/Src/comm_manager.c \
../App/Src/comm_protocol.c \
../App/Src/config_store.c \
../App/Src/control_loop.c \
../App/Src/scheduler.c \
../App/Src/sensor_manager.c 

OBJS += \
./App/Src/actuator_manager.o \
./App/Src/comm_manager.o \
./App/Src/comm_protocol.o \
./App/Src/config_store.o \
./App/Src/control_loop.o \
./App/Src/scheduler.o \
./App/Src/sensor_manager.o 

C_DEPS += \
./App/Src/actuator_manager.d \
./App/Src/comm_manager.d \
./App/Src/comm_protocol.d \
./App/Src/config_store.d \
./App/Src/control_loop.d \
./App/Src/scheduler.d \
./App/Src/sensor_manager.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su App/Src/%.cyclo: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../App/Inc -I../BSP/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/actuator_manager.cyclo ./App/Src/actuator_manager.d ./App/Src/actuator_manager.o ./App/Src/actuator_manager.su ./App/Src/comm_manager.cyclo ./App/Src/comm_manager.d ./App/Src/comm_manager.o ./App/Src/comm_manager.su ./App/Src/comm_protocol.cyclo ./App/Src/comm_protocol.d ./App/Src/comm_protocol.o ./App/Src/comm_protocol.su ./App/Src/config_store.cyclo ./App/Src/config_store.d ./App/Src/config_store.o ./App/Src/config_store.su ./App/Src/control_loop.cyclo ./App/Src/control_loop.d ./App/Src/control_loop.o ./App/Src/control_loop.su ./App/Src/scheduler.cyclo ./App/Src/scheduler.d ./App/Src/scheduler.o ./App/Src/scheduler.su ./App/Src/sensor_manager.cyclo ./App/Src/sensor_manager.d ./App/Src/sensor_manager.o ./App/Src/sensor_manager.su

.PHONY: clean-App-2f-Src

