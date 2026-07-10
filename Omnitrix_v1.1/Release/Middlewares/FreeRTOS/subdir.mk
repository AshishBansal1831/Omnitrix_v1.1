################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/FreeRTOS/croutine.c \
../Middlewares/FreeRTOS/event_groups.c \
../Middlewares/FreeRTOS/list.c \
../Middlewares/FreeRTOS/queue.c \
../Middlewares/FreeRTOS/stream_buffer.c \
../Middlewares/FreeRTOS/tasks.c \
../Middlewares/FreeRTOS/timers.c 

C_DEPS += \
./Middlewares/FreeRTOS/croutine.d \
./Middlewares/FreeRTOS/event_groups.d \
./Middlewares/FreeRTOS/list.d \
./Middlewares/FreeRTOS/queue.d \
./Middlewares/FreeRTOS/stream_buffer.d \
./Middlewares/FreeRTOS/tasks.d \
./Middlewares/FreeRTOS/timers.d 

OBJS += \
./Middlewares/FreeRTOS/croutine.o \
./Middlewares/FreeRTOS/event_groups.o \
./Middlewares/FreeRTOS/list.o \
./Middlewares/FreeRTOS/queue.o \
./Middlewares/FreeRTOS/stream_buffer.o \
./Middlewares/FreeRTOS/tasks.o \
./Middlewares/FreeRTOS/timers.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/FreeRTOS/%.o Middlewares/FreeRTOS/%.su Middlewares/FreeRTOS/%.cyclo: ../Middlewares/FreeRTOS/%.c Middlewares/FreeRTOS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/Config" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/FreeRTOSV11" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/include" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/SEGGER" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-FreeRTOS

clean-Middlewares-2f-FreeRTOS:
	-$(RM) ./Middlewares/FreeRTOS/croutine.cyclo ./Middlewares/FreeRTOS/croutine.d ./Middlewares/FreeRTOS/croutine.o ./Middlewares/FreeRTOS/croutine.su ./Middlewares/FreeRTOS/event_groups.cyclo ./Middlewares/FreeRTOS/event_groups.d ./Middlewares/FreeRTOS/event_groups.o ./Middlewares/FreeRTOS/event_groups.su ./Middlewares/FreeRTOS/list.cyclo ./Middlewares/FreeRTOS/list.d ./Middlewares/FreeRTOS/list.o ./Middlewares/FreeRTOS/list.su ./Middlewares/FreeRTOS/queue.cyclo ./Middlewares/FreeRTOS/queue.d ./Middlewares/FreeRTOS/queue.o ./Middlewares/FreeRTOS/queue.su ./Middlewares/FreeRTOS/stream_buffer.cyclo ./Middlewares/FreeRTOS/stream_buffer.d ./Middlewares/FreeRTOS/stream_buffer.o ./Middlewares/FreeRTOS/stream_buffer.su ./Middlewares/FreeRTOS/tasks.cyclo ./Middlewares/FreeRTOS/tasks.d ./Middlewares/FreeRTOS/tasks.o ./Middlewares/FreeRTOS/tasks.su ./Middlewares/FreeRTOS/timers.cyclo ./Middlewares/FreeRTOS/timers.d ./Middlewares/FreeRTOS/timers.o ./Middlewares/FreeRTOS/timers.su

.PHONY: clean-Middlewares-2f-FreeRTOS

