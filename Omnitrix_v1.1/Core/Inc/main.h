/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define DataCmd_Pin GPIO_PIN_4
#define DataCmd_GPIO_Port GPIOC
#define LCD_RESET_Pin GPIO_PIN_5
#define LCD_RESET_GPIO_Port GPIOC
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LEFT_ROT_BUTTON_Pin GPIO_PIN_1
#define LEFT_ROT_BUTTON_GPIO_Port GPIOA
#define LEFT_ROT_BUTTON_EXTI_IRQn EXTI1_IRQn
#define LCD_CHIP_SELECT_Pin GPIO_PIN_4
#define LCD_CHIP_SELECT_GPIO_Port GPIOA
#define DataCmd_Pin GPIO_PIN_4
#define DataCmd_GPIO_Port GPIOC
#define LCD_RESET_Pin GPIO_PIN_5
#define LCD_RESET_GPIO_Port GPIOC
#define RIGHT_ROT_BUTTON_Pin GPIO_PIN_13
#define RIGHT_ROT_BUTTON_GPIO_Port GPIOE
#define RIGHT_ROT_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define ESP_READY_Pin GPIO_PIN_15
#define ESP_READY_GPIO_Port GPIOE
#define ESP_CS_Pin GPIO_PIN_9
#define ESP_CS_GPIO_Port GPIOD
#define ESP_CS_EXTI_IRQn EXTI9_5_IRQn
#define ACTIVITY_BUTTON_Pin GPIO_PIN_8
#define ACTIVITY_BUTTON_GPIO_Port GPIOB
#define ACTIVITY_BUTTON_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */
enum SpecialChar_t : uint8_t
{
	BACK_SPACE  = 0x7F,
	ENTER       = 0x0A,
	CAR_RET     = '\r',
	NULL_CHAR	= '\0',
};

struct __attribute__((packed, aligned(4))) Time {
    uint16_t Year;     // e.g., 2026
    uint8_t  Month;    // 1 - 12
    uint8_t  Day;      // 1 - 31
    uint8_t  Hour;    // 0 - 23
    uint8_t  Min;  // 0 - 59
    uint8_t  Sec;  // 0 - 59
    uint32_t epoch;    // Unix Timestamp (4 bytes)
};
void set_time(const uint32_t Time); // as got from the wifi module
void set_date(const uint32_t Date); // ddmmyyyy
void read_time(char *read_time_buf, size_t buf_size);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
