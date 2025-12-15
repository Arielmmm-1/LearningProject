#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						
	
	PWM_Init();													
}

/**
  * 函    数：直流电机设置速度
  * 参    数：Speed 要设置的速度，范围：-100~100
  * 返 回 值：无
  */
void Motor_SetSpeed(int8_t LeftSpeed ,int8_t RightSpeed)
{
	if (LeftSpeed >= 0)							
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_6);	
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);	
		PWM_SetCompare1(LeftSpeed);			
	}
	else									
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);	
		GPIO_SetBits(GPIOB, GPIO_Pin_5);	
		PWM_SetCompare1(-LeftSpeed);		
	}
	if (RightSpeed >= 0)					
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_7);	
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);	
		PWM_SetCompare2(RightSpeed);		
	}
	else									
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_7);	
		GPIO_SetBits(GPIOB, GPIO_Pin_8);	
		PWM_SetCompare2(-RightSpeed);		
	}
}
void forword()
{
	Motor_SetSpeed(50,50);
}

void right()
{
	Motor_SetSpeed(-50,50);
}

void left()
{
	Motor_SetSpeed(50,-50);
}

void back()
{
	Motor_SetSpeed(-50,-50);
}

void stop()
{
	Motor_SetSpeed(0,0);
}
