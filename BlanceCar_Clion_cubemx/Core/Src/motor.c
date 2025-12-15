#include "Motor.h"
#include "tim.h"
#define PWM_MAX 7200
#define PWM_MIN  -7200

extern int stop;
//绝对值函数
int abs(int p)
{
    if (p>0)
        return p;
    else
        return -p;
}

//电机1和电机2转速分配
void load(int motor1,int motor2)  //-7200 ~ 7200占空比
{
    //左电机方向，及PWM输入
    if (motor1<0)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
    }
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_4,abs(motor1));

    //右电机方向，及PWM输入
    if (motor2<0)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);
    }
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_1,abs(motor2));
}
void limit(int *motorA,int *motorB)
{
    if (*motorA>PWM_MAX)*motorA=PWM_MAX;
    if (*motorA<PWM_MIN)*motorA=PWM_MIN;
    if (*motorB>PWM_MAX)*motorB=PWM_MAX;
    if (*motorB<PWM_MIN)*motorB=PWM_MIN;
}
void Stop(float *Med_Jiaodu,float *Jiaodu)
{
    if(abs((int)(*Jiaodu-*Med_Jiaodu))>60)
    {
        load(0,0);
        stop=1;
    }
}
