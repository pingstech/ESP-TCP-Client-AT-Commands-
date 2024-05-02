################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/tcp_client_firmware/esp/api/api.c 

OBJS += \
./Core/tcp_client_firmware/esp/api/api.o 

C_DEPS += \
./Core/tcp_client_firmware/esp/api/api.d 


# Each subdirectory must supply rules for building sources it contributes
Core/tcp_client_firmware/esp/api/%.o Core/tcp_client_firmware/esp/api/%.su Core/tcp_client_firmware/esp/api/%.cyclo: ../Core/tcp_client_firmware/esp/api/%.c Core/tcp_client_firmware/esp/api/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G0B1xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I../Core/hardware -I../Core/tcp_client_firmware -I../Core/tcp_client_firmware/esp -I../Core/tcp_client_firmware/esp/api -I../Core/tcp_client_firmware/esp/app -I../Core/tcp_client_firmware/queue -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-tcp_client_firmware-2f-esp-2f-api

clean-Core-2f-tcp_client_firmware-2f-esp-2f-api:
	-$(RM) ./Core/tcp_client_firmware/esp/api/api.cyclo ./Core/tcp_client_firmware/esp/api/api.d ./Core/tcp_client_firmware/esp/api/api.o ./Core/tcp_client_firmware/esp/api/api.su

.PHONY: clean-Core-2f-tcp_client_firmware-2f-esp-2f-api

