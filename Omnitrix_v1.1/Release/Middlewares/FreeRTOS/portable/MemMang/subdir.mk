################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/FreeRTOS/portable/MemMang/heap_4.c 

C_DEPS += \
./Middlewares/FreeRTOS/portable/MemMang/heap_4.d 

OBJS += \
./Middlewares/FreeRTOS/portable/MemMang/heap_4.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/FreeRTOS/portable/MemMang/%.o Middlewares/FreeRTOS/portable/MemMang/%.su Middlewares/FreeRTOS/portable/MemMang/%.cyclo: ../Middlewares/FreeRTOS/portable/MemMang/%.c Middlewares/FreeRTOS/portable/MemMang/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/Config" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/FreeRTOSV11" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/include" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/SEGGER" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-FreeRTOS-2f-portable-2f-MemMang

clean-Middlewares-2f-FreeRTOS-2f-portable-2f-MemMang:
	-$(RM) ./Middlewares/FreeRTOS/portable/MemMang/heap_4.cyclo ./Middlewares/FreeRTOS/portable/MemMang/heap_4.d ./Middlewares/FreeRTOS/portable/MemMang/heap_4.o ./Middlewares/FreeRTOS/portable/MemMang/heap_4.su

.PHONY: clean-Middlewares-2f-FreeRTOS-2f-portable-2f-MemMang

