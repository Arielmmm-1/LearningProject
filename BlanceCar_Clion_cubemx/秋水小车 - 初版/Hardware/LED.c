#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  //开启时钟
	GPIO_InitTypeDef GPIO_InitStructure;				 //宏定义
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置通用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  				//初始化GPIO,GPIO_InitStructure在上面配置

	GPIO_SetBits(GPIOA,GPIO_Pin_1 | GPIO_Pin_2);
	
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}
void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
}
void LED1_Turn()
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1) == 0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_1); 
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	}
}


void LED2_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
}
void LED2_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_2);
}
void LED2_Turn()
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_2) == 0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_2); 
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
	}
}

/*
  GPIO 引脚的电平状态是 “一旦设置，除非被其他代码修改，否则会一直保持”

	1.  第一次按按键：触发LED2_Turn()
	  此时GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2)读取到高电平（1）（因为初始是灭）；
	  满足else条件，执行GPIO_ResetBits(GPIOA, GPIO_Pin_2) → PA2 变为低电平 → LED 亮；

	2.  第二次按按键：再次触发LED2_Turn()
	  此时GPIO_ReadOutputDataBit读取到低电平（0）（因为第一次翻转后 LED 亮）；
	  满足if条件，执行GPIO_SetBits → PA2 变为高电平 → LED 灭；
*/


