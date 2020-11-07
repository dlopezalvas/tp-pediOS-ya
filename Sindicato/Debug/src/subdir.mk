################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Sindicato.c \
../src/SindicatoApi.c \
../src/SindicatoConsole.c \
../src/SindicatoServer.c \
../src/SindicatoUtils.c 

OBJS += \
./src/Sindicato.o \
./src/SindicatoApi.o \
./src/SindicatoConsole.o \
./src/SindicatoServer.o \
./src/SindicatoUtils.o 

C_DEPS += \
./src/Sindicato.d \
./src/SindicatoApi.d \
./src/SindicatoConsole.d \
./src/SindicatoServer.d \
./src/SindicatoUtils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


