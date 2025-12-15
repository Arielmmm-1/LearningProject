#include "stm32f10x.h"

void PWM_Init(void)
{
	/*GPIO初始化,PWMA PA11,PWMB PA8*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_8;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*TIM配置*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO, ENABLE);
	
	//初始化定时器
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period=100 - 1;//自动重装载值
	TIM_TimeBaseInitStruct.TIM_Prescaler=36 - 1;//预分频
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
	
	//初始化输出比较
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;  
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High; //按需可反转极性
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse=0;//CCR
	TIM_OC1Init(TIM1,&TIM_OCInitStruct); //PA8 TIM1_CH1
	TIM_OC4Init(TIM1,&TIM_OCInitStruct); //PA11 TIM1_CH4
	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//高级定时器专属--MOE主输出使能
	
	TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);//ENABLE//OC1预装载寄存器使能
	TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);//ENABLE//OC4预装载寄存器使能
	TIM_ARRPreloadConfig(TIM1,ENABLE);//TIM1在ARR上预装载寄存器使能
	TIM_Cmd(TIM1,ENABLE);
}

void PWM_SetCompare1(uint16_t Compare)
{
	TIM_SetCompare1(TIM1,Compare);
}
void PWM_SetCompare4(uint16_t Compare)
{
	TIM_SetCompare4(TIM1,Compare);
}
