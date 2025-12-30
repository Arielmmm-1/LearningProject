#include "common.h"

// 外部变量：与中断共享/供控制任务使用
extern UART_HandleTypeDef huart3;
extern uint8_t rx_buf[1];
extern uint8_t Bluetooth_data;
extern uint8_t Fore, Back, Left, Right;
// 外部互斥量：保护蓝牙指令全局变量（Fore/Back/Left/Right）
extern SemaphoreHandle_t g_ble_data_mutex;

// 蓝牙任务句柄
//TaskHandle_t bleTaskHandle = NULL;

void ble_task(void const * argument)
{
  
  for(;;)
  {
    // 1. 无需主动解析指令（中断已完成），仅按需检查蓝牙中断状态，确保接收不中断
    if(huart3.RxState != HAL_UART_STATE_BUSY_RX)
    {
      HAL_UART_Receive_IT(&huart3, rx_buf, 1);
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
/*
在it中
void USART3_IRQHandler(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  HAL_UART_IRQHandler(&huart3); 

  // 直接修改变量（单字节uint8_t是原子操作，短期不会数据错乱）
  Bluetooth_data = rx_buf[0];
  if(Bluetooth_data == 0x00)      Fore=0,Back=0,Left=0,Right=0;
  else if(Bluetooth_data == 0x01) Fore=1,Back=0,Left=0,Right=0;
  else if(Bluetooth_data == 0x05) Fore=0,Back=1,Left=0,Right=0;
  else if(Bluetooth_data == 0x03) Fore=0,Back=0,Left=0,Right=1;
  else if(Bluetooth_data == 0x07) Fore=0,Back=0,Left=1,Right=0;
  else                            Fore=0,Back=0,Left=0,Right=0;

  // 重新使能串口中断接收，保证连续接收蓝牙数据（必需）
  HAL_UART_Receive_IT(&huart3, rx_buf, 1);

  // 任务切换标记
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
*/