#include "SR04.h"  //另一种写法，把库的头函数放在程序头函数里
#include "pid.h"

extern TIM_HandleTypeDef htim3;
uint16_t count;
float distance;

void delay_us(uint32_t us) //空指令延时函数us
{
	uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
	while (delay--)
	{
		;
	}
}

void GET_Distance(void)
{
	//测距模块触发信号
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
	delay_us(12);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
	delay_us(12);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_2)
	{
		//发送超声波上升沿
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2) == GPIO_PIN_SET)
		{
			//清除计数器后开始计时
			__HAL_TIM_SetCounter(&htim3,0);
			HAL_TIM_Base_Start (&htim3);
		}
		//接收超声波下降沿
		else
		{
			//停止计时
			HAL_TIM_Base_Stop(&htim3);
			count = __HAL_TIM_GetCounter(&htim3);
			distance = count*0.017f;
		}
	}
	//判断中断引脚
	if (GPIO_Pin == GPIO_PIN_5)
	{
		Control();
	}
}


