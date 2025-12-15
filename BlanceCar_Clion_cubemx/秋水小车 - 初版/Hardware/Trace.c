#include "stm32f10x.h"
#include "Delay.h"
#include "Trace.h"
#include "motor.h"
void Trace_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;  //循迹
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}	

void Trace_task(void)
{

	if(X1==0&&X3==0) Motor_SetSpeed(30,30);
		
	if(X1==1&&X3==0) Motor_SetSpeed(25,0);   //右转
		
	if(X1==0&&X3==1) Motor_SetSpeed(0,25);
	
	if(X2==1&&X1==1&&X3==1&&X4==0) Motor_SetSpeed(25,0);
		
	if(X2==0&&X1==1&&X3==1&&X4==1) Motor_SetSpeed(0,25);
		
	
	//拐右直角
	if(X3==0&&X4==0)
	{
		Delay_ms(200);
		Motor_SetSpeed(0,0);  //停车一下
		Delay_ms(500);
		do
		{
			Motor_SetSpeed(25,-25);  //原地右转
		}while(X1==0);
	}
	
	//拐左直角
	if(X2==0&&X1==0)
	{
		Delay_ms(200);
		Motor_SetSpeed(0,0);  //停车一下
		Delay_ms(500);
		do
		{
			Motor_SetSpeed(-25,25);  //原地左转
		}while(X3==0);
	}
	
	
	//if(X2==1&&X1==1&&X3==1&&X4==1) Set_Car_Speed(4000,4000);

}


