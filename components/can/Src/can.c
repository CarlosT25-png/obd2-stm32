#include "can.h"
#include "cmsis_os2.h"
#include "fdcan.h"
#include "main.h"
#include "stm32g4xx.h"
#include "stm32g4xx_hal_fdcan.h"
#include <stdint.h>

osThreadId_t getCarTelemetryDataHandler;
const osThreadAttr_t getCarTelemetryDataAttr = {
    .name = "getCarTelemetryData",
    .priority = osPriorityNormal,
    .stack_size = 256*4
};

// Semaphore to receive message through CAN
osSemaphoreId_t canRxSemHandler;

const osSemaphoreAttr_t canRxSem_attributes = {
  .name = "canRxSem"
};

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET )
    {
        osSemaphoreRelease(canRxSemHandler);
    }
}

const osThreadAttr_t sendDataToAppAttr = {
    .name = "sendDataToApp",
    .priority = osPriorityNormal,
    .stack_size = 128*4
};

// get car info from CAN
typedef enum {
    STATE_GET_RPM,
    STATE_GET_SPEED,
    STATE_GET_COOLANT,
    STATE_GET_OIL_TEMP,
    STATE_SEND_TO_QUEUE
} TelemetryState_t;

void getCarTelemetryData(void *argument)
{
    // FSM
    TelemetryPacket currentData = {0};
    TelemetryState_t state = STATE_GET_RPM;

    // TX
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8] = {// we send 0x0C which is for rpm
        0x02, 0x01, 0x0C, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC 
    };

    // rx
    FDCAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    // config the headers
    TxHeader.Identifier = 0x7DF;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN; // CAN 2.0 since my car only support that
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    // start the FDCAN
    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    while (1) {
        switch (state) {
            case STATE_GET_RPM:
                TxData[2] = 0x0C; // RPM PID

                // wait for rx fifo level
                while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0) {
                    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                }

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) == HAL_OK) {
                    if (osSemaphoreAcquire(canRxSemHandler, 50) == osOK) {
                        HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                        
                        if (RxData[1] == 0x41 && RxData[2] == 0x0C) { // mode 41 (shifted)
                            currentData.rpm = ((uint16_t)RxData[3] * 256 + RxData[4]) / 4;
                            
                            // Debug
                            // currentData.speed = 13;
                            // currentData.coolant_temp = 20;
                            // currentData.oil_temp = 95;
                            // state = STATE_SEND_TO_QUEUE;
                        }
                    }

                    state = STATE_GET_SPEED;
                }

                // Debug 
                // osDelay(500);
                break;

            case STATE_GET_SPEED:
                TxData[2] = 0x0D; // speed PID

                // wait for rx fifo level
                while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0) {
                    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                }

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) == HAL_OK) {
                    if (osSemaphoreAcquire(canRxSemHandler, 50) == osOK) {
                        HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                        
                        if (RxData[1] == 0x41 && RxData[2] == 0x0D) { 
                            currentData.speed = RxData[3];
                        }
                    }
                    state = STATE_GET_COOLANT; 
                }
                break;

            case STATE_GET_COOLANT:
                TxData[2] = 0x05; // coolant PID

                // wait for rx fifo level
                while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0) {
                    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                }

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) == HAL_OK) {
                    if (osSemaphoreAcquire(canRxSemHandler, 50) == osOK) {
                        HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                        
                        if (RxData[1] == 0x41 && RxData[2] == 0x05) { 
                            currentData.coolant_temp = (int8_t)RxData[3] - 40;
                        }
                    }
                    state = STATE_GET_OIL_TEMP;
                }
                break;

            case STATE_GET_OIL_TEMP:
                // mazda support; they use extended mode for oil temp
                TxData[0] = 0x03; // payload len is 3 bytes now
                TxData[1] = 0x22; // extended mode
                TxData[2] = 0x13; // pid h
                TxData[3] = 0x10; // pid l

                // wait for rx fifo level
                while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0) {
                    HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                }

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) == HAL_OK) {
                    if (osSemaphoreAcquire(canRxSemHandler, 50) == osOK) {
                        HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData);
                        
                        if (RxData[1] == 0x62 && RxData[2] == 0x13 && RxData[3] == 0x10) {
                            // Formula: (((A * 256) + B) / 100) - 40
                            currentData.oil_temp = (int8_t)((((uint16_t)RxData[4] * 256 + RxData[5]) / 100) - 40);
                        }
                    }
                    
                    // reset tx data
                    TxData[0] = 0x02; 
                    TxData[1] = 0x01;
                    TxData[3] = 0xCC; 
                    
                    state = STATE_SEND_TO_QUEUE;
                }
                break;

            case STATE_SEND_TO_QUEUE:
                currentData.timestamp = HAL_GetTick();
                osMessageQueuePut(uartTxQueueHandle, &currentData, 0, 0);
                state = STATE_GET_RPM; 
                osDelay(50); 
                break;
        }
    }
}


// osThreadId_t sendDataToAppHandler; - DEBUG

// send data

// void sendDataToAppTask(void *argument)
// {
//     TelemetryPacket val;
//     while(1)
//     {
//         val.rpm = 2000;
//         val.speed = 75;
//         val.coolant_temp = 100;
//         val.timestamp = HAL_GetTick();
//         printf("message added to queue \r\n");
//         osMessageQueuePut(uartTxQueueHandle, &val, 0, 0);
//         osDelay(2000);
//     }
// }