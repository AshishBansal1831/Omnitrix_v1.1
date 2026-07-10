################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../Middlewares/SEGGER/SEGGER/SEGGER_RTT_ASM_ARMv7M.S 

C_SRCS += \
../Middlewares/SEGGER/SEGGER/SEGGER_RTT.c \
../Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.c \
../Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.c 

C_DEPS += \
./Middlewares/SEGGER/SEGGER/SEGGER_RTT.d \
./Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.d \
./Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.d 

OBJS += \
./Middlewares/SEGGER/SEGGER/SEGGER_RTT.o \
./Middlewares/SEGGER/SEGGER/SEGGER_RTT_ASM_ARMv7M.o \
./Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.o \
./Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.o 

S_UPPER_DEPS += \
./Middlewares/SEGGER/SEGGER/SEGGER_RTT_ASM_ARMv7M.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/SEGGER/SEGGER/%.o Middlewares/SEGGER/SEGGER/%.su Middlewares/SEGGER/SEGGER/%.cyclo: ../Middlewares/SEGGER/SEGGER/%.c Middlewares/SEGGER/SEGGER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/Config" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/FreeRTOSV11" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/include" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/SEGGER" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/SEGGER/SEGGER/%.o: ../Middlewares/SEGGER/SEGGER/%.S Middlewares/SEGGER/SEGGER/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Middlewares-2f-SEGGER-2f-SEGGER

clean-Middlewares-2f-SEGGER-2f-SEGGER:
	-$(RM) ./Middlewares/SEGGER/SEGGER/SEGGER_RTT.cyclo ./Middlewares/SEGGER/SEGGER/SEGGER_RTT.d ./Middlewares/SEGGER/SEGGER/SEGGER_RTT.o ./Middlewares/SEGGER/SEGGER/SEGGER_RTT.su ./Middlewares/SEGGER/SEGGER/SEGGER_RTT_ASM_ARMv7M.d ./Middlewares/SEGGER/SEGGER/SEGGER_RTT_ASM_ARMv7M.o ./Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.cyclo ./Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.d ./Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.o ./Middlewares/SEGGER/SEGGER/SEGGER_RTT_printf.su ./Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.cyclo ./Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.d ./Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.o ./Middlewares/SEGGER/SEGGER/SEGGER_SYSVIEW.su

.PHONY: clean-Middlewares-2f-SEGGER-2f-SEGGER

