/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PTC_Pin GPIO_PIN_0
#define PTC_GPIO_Port GPIOA
#define ASSE_X_JOYSTICK_Pin GPIO_PIN_4
#define ASSE_X_JOYSTICK_GPIO_Port GPIOA
#define ASSE_Y_JOYSTICK_Pin GPIO_PIN_5
#define ASSE_Y_JOYSTICK_GPIO_Port GPIOA
#define PELTIER_COLD_FAN_Pin GPIO_PIN_6
#define PELTIER_COLD_FAN_GPIO_Port GPIOA
#define PTC_FAN_Pin GPIO_PIN_7
#define PTC_FAN_GPIO_Port GPIOA
#define TASTO_JOYSTICK_Pin GPIO_PIN_4
#define TASTO_JOYSTICK_GPIO_Port GPIOC
#define DISPLAY_SCL_Pin GPIO_PIN_10
#define DISPLAY_SCL_GPIO_Port GPIOB
#define VENTOLINA_Pin GPIO_PIN_6
#define VENTOLINA_GPIO_Port GPIOC
#define SENSORE_TEMP_INT_Pin GPIO_PIN_8
#define SENSORE_TEMP_INT_GPIO_Port GPIOA
#define SENSORE_TEMP_EXT_Pin GPIO_PIN_9
#define SENSORE_TEMP_EXT_GPIO_Port GPIOA
#define PELTIER_Pin GPIO_PIN_15
#define PELTIER_GPIO_Port GPIOA
#define DISPLAY_SDA_Pin GPIO_PIN_12
#define DISPLAY_SDA_GPIO_Port GPIOC
#define PELTIER_HOT_FAN_Pin GPIO_PIN_3
#define PELTIER_HOT_FAN_GPIO_Port GPIOB
#define STATO_PORTA_Pin GPIO_PIN_4
#define STATO_PORTA_GPIO_Port GPIOB
#define SERVO_1_Pin GPIO_PIN_6
#define SERVO_1_GPIO_Port GPIOB
#define SERVO_2_Pin GPIO_PIN_7
#define SERVO_2_GPIO_Port GPIOB
#define ESP32_SCL_Pin GPIO_PIN_8
#define ESP32_SCL_GPIO_Port GPIOB
#define ESP32_SDA_Pin GPIO_PIN_9
#define ESP32_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
