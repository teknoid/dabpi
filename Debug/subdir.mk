################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../dabpi_ctl.o \
../si46xx.o 

C_SRCS += \
../dab.c \
../dabpi_ctl.c \
../gpio.c \
../si46xx.c \
../spidev_test.c \
../spitest.c \
../utils.c 

OBJS += \
./dab.o \
./dabpi_ctl.o \
./gpio.o \
./si46xx.o \
./spidev_test.o \
./spitest.o \
./utils.o 

C_DEPS += \
./dab.d \
./dabpi_ctl.d \
./gpio.d \
./si46xx.d \
./spidev_test.d \
./spitest.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


