//
// Created by MECHREVO on 2025/12/14.
//

#ifndef TESE1_MOTOR_H
#define TESE1_MOTOR_H
#include "stm32f1xx_hal.h"

void load(int motor1,int motor2);
void limit(int *motorA,int *motorB);
void Stop(float *Med_Jiaodu,float *Jiaodu);
#endif //TESE1_MOTOR_H