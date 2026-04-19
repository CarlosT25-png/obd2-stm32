#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "cmsis_os2.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_uart.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

extern osMessageQueueId_t uartTxQueueHandle;
extern osSemaphoreId_t uartTxSemHandle;
extern osThreadId_t sendToDeviceUartTaskHandle;

extern const osSemaphoreAttr_t uartTxSem_attributes;
extern const osThreadAttr_t sendToDeviceUartAttr;

void sendToDeviceTask(void *argument);