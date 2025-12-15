#ifndef __Trace_H_
#define __Trace_H_

#include "stm32f10x.h"                  // Device header

#define X1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)
#define X2 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)
#define X3 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)
#define X4 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)
void Trace_task(void);
void Trace_Init(void);

#endif

