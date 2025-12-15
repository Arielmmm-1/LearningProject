#include "stm32f10x.h"                  // Device header

#ifndef  __SERIAL_H_
#define  __SERIAL_H_

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_ReceiveByte(void);
#endif
