################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.c 

C_DEPS += \
./Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.d 

OBJS += \
./Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/SEGGER/SEGGER/Syscalls/%.o Middlewares/SEGGER/SEGGER/Syscalls/%.su Middlewares/SEGGER/SEGGER/Syscalls/%.cyclo: ../Middlewares/SEGGER/SEGGER/Syscalls/%.c Middlewares/SEGGER/SEGGER/Syscalls/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/Config" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/FreeRTOSV11" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/include" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/SEGGER" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-SEGGER-2f-SEGGER-2f-Syscalls

clean-Middlewares-2f-SEGGER-2f-SEGGER-2f-Syscalls:
	-$(RM) ./Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.cyclo ./Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.d ./Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.o ./Middlewares/SEGGER/SEGGER/Syscalls/SEGGER_RTT_Syscalls_GCC.su

.PHONY: clean-Middlewares-2f-SEGGER-2f-SEGGER-2f-Syscalls

