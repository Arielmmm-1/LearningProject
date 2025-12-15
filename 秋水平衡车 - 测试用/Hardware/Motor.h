#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f10x.h"
#include "PWM.h"

//直接操作GPIO寄存器
#define GPIOB_ODR *(volatile uint32_t *)(0x40010C0C) // GPIOB输出数据寄存器地址
#define Ain1 (GPIOB_ODR |= (1<<13))  // PB13置1
#define Ain1_Clr (GPIOB_ODR &= ~(1<<13)) // PB13清0
#define Ain2 (GPIOB_ODR |= (1<<12))  // PB12置1
#define Ain2_Clr (GPIOB_ODR &= ~(1<<12)) // PB12清0
#define Bin1 (GPIOB_ODR |= (1<<14))  // PB14置1
#define Bin1_Clr (GPIOB_ODR &= ~(1<<14)) // PB14清0
#define Bin2 (GPIOB_ODR |= (1<<15))  // PB15置1
#define Bin2_Clr (GPIOB_ODR &= ~(1<<15)) // PB15清0

void Motor_Init(void);
void Motor_SetPWM(uint8_t n,int16_t PWM);

#endif
