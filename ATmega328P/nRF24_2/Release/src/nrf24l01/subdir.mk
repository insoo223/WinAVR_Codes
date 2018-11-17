################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/nrf24l01/nrf24l01.c 

OBJS += \
./src/nrf24l01/nrf24l01.o 

C_DEPS += \
./src/nrf24l01/nrf24l01.d 


# Each subdirectory must supply rules for building sources it contributes
src/nrf24l01/%.o: ../src/nrf24l01/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega8 -DF_CPU=1000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


