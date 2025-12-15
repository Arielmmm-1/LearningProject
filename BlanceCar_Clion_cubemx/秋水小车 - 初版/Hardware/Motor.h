#ifndef __Motor_H__  
#define  __Motor_H__
#include "stm32f10x.h"              
#include "PWM.h"
void Motor_Init(void);
void Motor_SetSpeed(int8_t LeftSpeed ,int8_t RightSpeed);
void forword(void);
void right(void);
void left(void);
void back(void);
void stop(void);

#endif
