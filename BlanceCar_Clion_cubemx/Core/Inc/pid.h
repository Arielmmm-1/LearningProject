#ifndef TESE1_PID_H
#define TESE1_PID_H
#include "stm32f1xx_hal.h"
int Vertical(float Med,float Angle,float gyro_Y);
int Velocity(int Target,int encoder_L,int encoder_R);
int Turn(float gyro_Z,int Target_turn);
void Control(void);
#endif //TESE1_PID_H