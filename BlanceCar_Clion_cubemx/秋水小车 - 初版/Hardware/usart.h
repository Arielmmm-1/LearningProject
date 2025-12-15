#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdio.h>

// 函数声明
void USART1_Init(uint32_t baudrate);
void USART1_SendByte(uint8_t byte);
void USART1_SendString(char* str);
uint8_t USART1_ReceiveByte(void);
_Bool USART1_DataAvailable(void); // 检查是否有数据可读

#endif
