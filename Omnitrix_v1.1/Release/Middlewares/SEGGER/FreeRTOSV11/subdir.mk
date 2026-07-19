################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.c 

C_DEPS += \
./Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.d 

OBJS += \
./Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/SEGGER/FreeRTOSV11/%.o Middlewares/SEGGER/FreeRTOSV11/%.su Middlewares/SEGGER/FreeRTOSV11/%.cyclo: ../Middlewares/SEGGER/FreeRTOSV11/%.c Middlewares/SEGGER/FreeRTOSV11/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/Config" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/FreeRTOSV11" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/include" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/SEGGER" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Oz -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-SEGGER-2f-FreeRTOSV11

clean-Middlewares-2f-SEGGER-2f-FreeRTOSV11:
	-$(RM) ./Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.cyclo ./Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.d ./Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.o ./Middlewares/SEGGER/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.su

.PHONY: clean-Middlewares-2f-SEGGER-2f-FreeRTOSV11

