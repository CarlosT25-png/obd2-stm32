#pragma once

#include "cmsis_os2.h"
#include "bridge.h"
#include "fdcan.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_fdcan.h"
#include "stm32g4xx_hal_gpio.h"
#include <stdint.h>
#include <stdio.h>

extern osThreadId_t getCarTelemetryDataHandler;
extern const osThreadAttr_t getCarTelemetryDataAttr;

extern osSemaphoreId_t canRxSemHandler;
extern const osSemaphoreAttr_t canRxSem_attributes;


typedef struct __attribute__((packed)) {
    uint16_t rpm;
    uint8_t  speed;
    int8_t   coolant_temp;
    int8_t   oil_temp;
    uint32_t timestamp;
} TelemetryPacket;

void getCarTelemetryData(void *argument);

// DEBUG
// void sendDataToAppTask(void *argument);

// extern osThreadId_t sendDataToAppHandler;
// extern const osThreadAttr_t sendDataToAppAttr;
