#include "encoder.h"

//指定哪个定时器测速
int Read_Speed(TIM_HandleTypeDef *htim)
{
    int temp = 0;
    temp = (short)__HAL_TIM_GetCounter(htim);  //short才有正负(-32768~32768)
    __HAL_TIM_SetCounter(htim,0);
    return temp;
}