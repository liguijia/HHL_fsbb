/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern void my_pid_init(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USR_LED_Pin GPIO_PIN_13
#define USR_LED_GPIO_Port GPIOC
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOF
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOF
#define UART_TX_Pin GPIO_PIN_2
#define UART_TX_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_3
#define UART_RX_GPIO_Port GPIOA
#define ADC_V_CAP_Pin GPIO_PIN_5
#define ADC_V_CAP_GPIO_Port GPIOA
#define ADC_I_CAP_Pin GPIO_PIN_2
#define ADC_I_CAP_GPIO_Port GPIOB
#define ADC_I_CHASSIS_Pin GPIO_PIN_12
#define ADC_I_CHASSIS_GPIO_Port GPIOB
#define ADC_V_MOTOR_Pin GPIO_PIN_13
#define ADC_V_MOTOR_GPIO_Port GPIOB
#define CAP_L_Pin GPIO_PIN_14
#define CAP_L_GPIO_Port GPIOB
#define CAP_H_Pin GPIO_PIN_15
#define CAP_H_GPIO_Port GPIOB
#define MOTOR_L_Pin GPIO_PIN_8
#define MOTOR_L_GPIO_Port GPIOA
#define MOTOR_H_Pin GPIO_PIN_9
#define MOTOR_H_GPIO_Port GPIOA
#define CAN_RX_Pin GPIO_PIN_11
#define CAN_RX_GPIO_Port GPIOA
#define CAN_TX_Pin GPIO_PIN_12
#define CAN_TX_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
