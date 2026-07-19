################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Aliens/cannon_bolt.c \
../Core/Aliens/fourarms.c \
../Core/Aliens/grey_matter_bmp.c \
../Core/Aliens/heatblast.c \
../Core/Aliens/omnitrix.c \
../Core/Aliens/xlr8_bitmap.c 

C_DEPS += \
./Core/Aliens/cannon_bolt.d \
./Core/Aliens/fourarms.d \
./Core/Aliens/grey_matter_bmp.d \
./Core/Aliens/heatblast.d \
./Core/Aliens/omnitrix.d \
./Core/Aliens/xlr8_bitmap.d 

OBJS += \
./Core/Aliens/cannon_bolt.o \
./Core/Aliens/fourarms.o \
./Core/Aliens/grey_matter_bmp.o \
./Core/Aliens/heatblast.o \
./Core/Aliens/omnitrix.o \
./Core/Aliens/xlr8_bitmap.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Aliens/%.o Core/Aliens/%.su Core/Aliens/%.cyclo: ../Core/Aliens/%.c Core/Aliens/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/Config" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/FreeRTOSV11" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/include" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/SEGGER/SEGGER" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS" -I"/home/ashish/Personal_Projects/OmniWatchv1.1/Omnitrix_v1.1/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Oz -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Aliens

clean-Core-2f-Aliens:
	-$(RM) ./Core/Aliens/cannon_bolt.cyclo ./Core/Aliens/cannon_bolt.d ./Core/Aliens/cannon_bolt.o ./Core/Aliens/cannon_bolt.su ./Core/Aliens/fourarms.cyclo ./Core/Aliens/fourarms.d ./Core/Aliens/fourarms.o ./Core/Aliens/fourarms.su ./Core/Aliens/grey_matter_bmp.cyclo ./Core/Aliens/grey_matter_bmp.d ./Core/Aliens/grey_matter_bmp.o ./Core/Aliens/grey_matter_bmp.su ./Core/Aliens/heatblast.cyclo ./Core/Aliens/heatblast.d ./Core/Aliens/heatblast.o ./Core/Aliens/heatblast.su ./Core/Aliens/omnitrix.cyclo ./Core/Aliens/omnitrix.d ./Core/Aliens/omnitrix.o ./Core/Aliens/omnitrix.su ./Core/Aliens/xlr8_bitmap.cyclo ./Core/Aliens/xlr8_bitmap.d ./Core/Aliens/xlr8_bitmap.o ./Core/Aliens/xlr8_bitmap.su

.PHONY: clean-Core-2f-Aliens

