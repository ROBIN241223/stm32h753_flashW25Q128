/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdbool.h>
#include "gpio.h"
extern void vCLITask(void *pvParameters);
extern void vRegisterSampleCLICommands(void);
extern void vRegisterCustomCLICommands(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for uartPrintTask */
osThreadId_t uartPrintTaskHandle;
const osThreadAttr_t uartPrintTask_attributes = {
  .name = "uartPrintTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartUartPrintTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartUartPrintTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  /* creation of uartPrintTask */
  uartPrintTaskHandle = osThreadNew(StartUartPrintTask, NULL, &uartPrintTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  /* Register CLI commands */
  vRegisterSampleCLICommands();
  vRegisterCustomCLICommands();
  
  /* Create CLI task */
  xTaskCreate(vCLITask, 
              "CLI", 
              512,           /* Stack size in words */
              NULL, 
              tskIDLE_PRIORITY + 2, 
              NULL);
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  
  /* Infinite loop - LED nháy liên tục */
  for(;;)
  {
    // Đỏ ON
    LED_SetRGB(true, false, false);
    osDelay(300);
    
    // Xanh lá ON
    LED_SetRGB(false, true, false);
    osDelay(300);
    
    // Xanh dương ON
    LED_SetRGB(false, false, true);
    osDelay(300);
    
    // Tất cả ON (trắng)
    LED_SetRGB(true, true, true);
    osDelay(300);
    
    // Tất cả OFF
    LED_SetRGB(false, false, false);
    osDelay(300);
  }
  /* USER CODE END StartDefaultTask */
}
/* USER CODE BEGIN Header_StartUartPrintTask */
/**
  * @brief  Function implementing the uartPrintTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartUartPrintTask */
void StartUartPrintTask(void *argument)
{
  /* USER CODE BEGIN StartUartPrintTask */
  for(;;)
  {
    printf("Hello from FreeRTOS!\r\n");
    osDelay(1000);
  }
  /* USER CODE END StartUartPrintTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* Runtime stats timer configuration for FreeRTOS+CLI */
static uint32_t ulRunTimeStatsCounter = 0;

void vConfigureTimerForRunTimeStats(void)
{
    /* Initialize runtime stats counter */
    ulRunTimeStatsCounter = 0;
}

uint32_t ulGetRunTimeCounterValue(void)
{
    /* Return runtime counter value (incremented in SysTick) */
    return ulRunTimeStatsCounter;
}

/* This should be called from SysTick_Handler or a high-frequency timer */
void vApplicationTickHook(void)
{
    ulRunTimeStatsCounter++;
}

/* USER CODE END Application */
