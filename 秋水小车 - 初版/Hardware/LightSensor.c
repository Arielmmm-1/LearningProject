#include "stm32f10x.h"                  // Device header
void LightSensor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);  //开启时钟
	GPIO_InitTypeDef GPIO_InitStructure;				 //宏定义
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//设置通用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);  				//初始化GPIO,GPIO_InitStructure在上面配置
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
}

uint8_t LightSensor_GET()
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);
}
