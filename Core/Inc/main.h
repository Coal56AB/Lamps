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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
typedef struct
{
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
}time_t;
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define DIGIT_HOUR_H_Pin GPIO_PIN_0
#define DIGIT_HOUR_H_GPIO_Port GPIOA
#define DIGIT_HOUR_L_Pin GPIO_PIN_1
#define DIGIT_HOUR_L_GPIO_Port GPIOA
#define DIGIT_MIN_H_Pin GPIO_PIN_2
#define DIGIT_MIN_H_GPIO_Port GPIOA
#define DIGIT_MIN_L_Pin GPIO_PIN_3
#define DIGIT_MIN_L_GPIO_Port GPIOA
#define DIGIT_SEC_H_Pin GPIO_PIN_4
#define DIGIT_SEC_H_GPIO_Port GPIOA
#define DIGIT_SEC_L_Pin GPIO_PIN_5
#define DIGIT_SEC_L_GPIO_Port GPIOA
#define SEGMENT_A_Pin GPIO_PIN_15
#define SEGMENT_A_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOA
#define BUZZER_B_Pin GPIO_PIN_0
#define BUZZER_B_GPIO_Port GPIOB
#define SW_BACK_Pin GPIO_PIN_9
#define SW_BACK_GPIO_Port GPIOA
#define SW_ENTER_Pin GPIO_PIN_10
#define SW_ENTER_GPIO_Port GPIOA
#define SW_DOWN_Pin GPIO_PIN_11
#define SW_DOWN_GPIO_Port GPIOA
#define SW_UP_Pin GPIO_PIN_12
#define SW_UP_GPIO_Port GPIOA
#define SEGMENT_C_Pin GPIO_PIN_3
#define SEGMENT_C_GPIO_Port GPIOB
#define SEGMENT_B_Pin GPIO_PIN_4
#define SEGMENT_B_GPIO_Port GPIOB
#define SEGMENT_D_Pin GPIO_PIN_6
#define SEGMENT_D_GPIO_Port GPIOB
#define SEGMENT_E_Pin GPIO_PIN_7
#define SEGMENT_E_GPIO_Port GPIOB
#define SEGMENT_F_Pin GPIO_PIN_8
#define SEGMENT_F_GPIO_Port GPIOB
#define SEGMENT_G_Pin GPIO_PIN_9
#define SEGMENT_G_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* Keep SWD on PA13/PA14 and disable buzzer B on PB0.
 * PB0 is physically bridged to PA13 on this board. */
#define SWD_DEBUG_MODE

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
