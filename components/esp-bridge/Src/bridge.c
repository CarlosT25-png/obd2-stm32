#include "bridge.h"
#include "cmsis_os2.h"


osMessageQueueId_t uartTxQueueHandle;
osSemaphoreId_t uartTxSemHandle;
osThreadId_t sendToDeviceUartTaskHandle;

const osSemaphoreAttr_t uartTxSem_attributes = {
  .name = "uartTxSem"
};

const osThreadAttr_t sendToDeviceUartAttr = {
    .name = "sendDataToDeviceUart",
    .priority = osPriorityAboveNormal,
    .stack_size = 128 *4
};

void sendToDeviceTask(void *argument)
{
    TelemetryPacket msg;
    while (1)
    {
        if(osMessageQueueGet(uartTxQueueHandle, &msg, 0, osWaitForever) == osOK)
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)&msg, sizeof(TelemetryPacket), 100);
        }
    }
}