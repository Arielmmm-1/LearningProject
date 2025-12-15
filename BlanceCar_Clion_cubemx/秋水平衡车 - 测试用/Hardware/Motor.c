#include "Motor.h"

#include "Motor.h"

void Motor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	// 初始电平清零（直接操作寄存器）
	GPIOB_ODR &= ~((1<<12)|(1<<13)|(1<<14)|(1<<15));
	
	PWM_Init();
}

void Motor_SetPWM(uint8_t n,int16_t PWM)
{
	// 限幅（避免PWM溢出）
	if(PWM>99) PWM=99;
	if(PWM<-99) PWM=-99;
	
	// 左电机（AIN1=PB13、AIN2=PB12）
	if(n == 1)
	{
		if(PWM >= 0)
		{
			Ain1_Clr;// PB13=0
			Ain2;    // PB12=1
			PWM_SetCompare4(PWM);
		}
		else 
		{
			Ain1;    // PB13=1
			Ain2_Clr;// PB12=0
			PWM_SetCompare4(-PWM);
		}
	}
	// 右电机（BIN1=PB14、BIN2=PB15）
	else if(n == 2)
	{
		if(PWM >= 0)
		{
			Bin1_Clr;// PB14=0
			Bin2;    // PB15=1
			PWM_SetCompare1(PWM);
		}
		else
		{
			Bin1;		// PB14=0
			Bin2_Clr;   // PB15=1
			PWM_SetCompare1(-PWM);
		}
	}
}
