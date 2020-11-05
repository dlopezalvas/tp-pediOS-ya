################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Comanda

dependents:
	-cd /home/utnso/workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug && $(MAKE) all

# Tool invocations
Comanda: $(OBJS) $(USER_OBJS) /home/utnso/workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug/libcommonsCoronaLinux.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug" -o "Comanda" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Comanda
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/workspace/tp-2020-2c-CoronaLinux/commonsCoronaLinux/Debug/libcommonsCoronaLinux.so:

-include ../makefile.targets
