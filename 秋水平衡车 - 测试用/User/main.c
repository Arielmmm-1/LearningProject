
/***
 *_______________#########_______________________
 *______________############_____________________
 *______________#############____________________
 *_____________##__###########___________________
 *____________###__######_#####__________________
 *____________###_#######___####_________________
 *___________###__##########_####________________
 *__________####__###########_####_______________
 *________#####___###########__#####_____________
 *_______######___###_########___#####___________
 *_______#####___###___########___######_________
 *______######___###__###########___######_______
 *_____######___####_##############__######______
 *____#######__#####################_#######_____
 *____#######__##############################____
 *___#######__######_#################_#######___
 *___#######__######_######_#########___######___
 *___#######____##__######___######_____######___
 *___#######________######____#####_____#####____
 *____######________#####_____#####_____####_____
 *_____#####________####______#####_____###______
 *______#####______;###________###______#________
 *________##_______####________####______________
 
										----林秋水
 */
 
 
#include "stm32f10x.h"                 
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "PWM.h"
#include "BlueSerial.h"
#include "MPU6050.h"
#include "TIMER.h"
#include "MOTOR.h"
#include "Encoder.h"
#include "PID.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>


 int16_t AX , AY , AZ , GX , GY , GZ;
 uint8_t Timer3ErrorFlag = 0;//检验中断是否卡死
 uint8_t TimerCount = 0;
 uint8_t RunFlag;  //是否执行PID的标志位
 
 int16_t LeftPWM ,RightPWM;
 int16_t  AvePWM ,DifPWM;//平均PWM和差分PWM
 
 float LeftSpeed , RightSpeed;
 float AveSpeed , DifSpeed;
 
 //角度环
 PID_t AnglePID ={
	.Kp = 0,
	.Ki = 0,
	.Kd = 0,
	 
	 .OutMax = 100,
	 .OutMin = -100, 
 };
 
 //速度环
  PID_t SpeedPID ={
	.Kp = 0,
	.Ki = 0,
	.Kd = 0,
	 
	 .OutMax = 20,
	 .OutMin = -20, 
 };
  //转向环
    PID_t TurnPID ={
	.Kp = 0,
	.Ki = 0,
	.Kd = 0,
	 
	 .OutMax = 50,
	 .OutMin = -50, 
 };
 
 float AngleAcc;
 float AngleGyro;
 float Angle;
 

int main(void)
{
	Delay_ms(500);
	MPU6050_Init();		
	OLED_Init();
	Motor_Init();
	Timer_Init();
	LED_Init();
	Encoder_Init();
	BlueSerial_Init();
	
	while(1)
	{
		if ( RunFlag ) { LED_ON();} else { LED_OFF(); }
		
		//用发送1来控制开关
		if(BlueSerial_RxFlag == 1) // 检测到有新的数据包
		{
			
			if (BlueSerial_RxPacket[0] == '1' && BlueSerial_RxPacket[1] == '\0')
			{
				
				RunFlag = !RunFlag;	
				PID_Init(&AnglePID);	 
				PID_Init(&SpeedPID);
				PID_Init(&TurnPID);
			}
		/*------------------------蓝牙串口调试数据-------------------------*/
			char *Tag = strtok(BlueSerial_RxPacket, ",");
			if (strcmp(Tag, "key") == 0)
			{
				char *Name = strtok(NULL, ",");
				char *Action = strtok(NULL, ",");
				
			}
			else if (strcmp(Tag, "slider") == 0)
			{
				char *Name = strtok(NULL, ",");
				char *Value = strtok(NULL, ",");
				
				//滑杆调参
				if (strcmp(Name, "AngleKp") == 0)
				{
					AnglePID.Kp = atof(Value);
				}
				else if (strcmp(Name, "AngleKi") == 0)
				{
					AnglePID.Ki = atof(Value);
				}
				else if (strcmp(Name, "AngleKd") == 0)
				{
					AnglePID.Kd = atof(Value);
				}
				
				
				if (strcmp(Name, "SpeedKp") == 0)
				{
					SpeedPID.Kp = atof(Value);
				}
				else if (strcmp(Name, "SpeedKi") == 0)
				{
					SpeedPID.Ki = atof(Value);
				}
				else if (strcmp(Name, "SpeedKd") == 0)
				{
					SpeedPID.Kd = atof(Value);
				}
				
				
					if (strcmp(Name, "TurnKp") == 0)
				{
					TurnPID.Kp = atof(Value);
				}
				else if (strcmp(Name, "TurnKi") == 0)
				{
					TurnPID.Ki = atof(Value);
				}
				else if (strcmp(Name, "TurnKd") == 0)
				{
					TurnPID.Kd = atof(Value);
				}
				
			}
			//遥感控制
			else if (strcmp(Tag, "joystick") == 0)
			{
				int8_t LH = atoi(strtok(NULL, ","));
				int8_t LV = atoi(strtok(NULL, ","));
				int8_t RH = atoi(strtok(NULL, ","));
				int8_t RV = atoi(strtok(NULL, ","));
				
				SpeedPID.Target = LV / 25.0;
				TurnPID.Target = RH / 25.0;
			}
			
	/*------------------------------------------------------------------*/
			// 处理完成后，清空标志位，等待下一次接收
			BlueSerial_RxFlag = 0;
		}
		
		OLED_Clear();
		OLED_Printf(0, 0, OLED_6X8, "  Angle");
		OLED_Printf(0, 8, OLED_6X8, "P:%05.2f", AnglePID.Kp);
		OLED_Printf(0, 16, OLED_6X8, "I:%05.2f", AnglePID.Ki);
		OLED_Printf(0, 24, OLED_6X8, "D:%05.2f", AnglePID.Kd);
		OLED_Printf(0, 32, OLED_6X8, "T:%+05.1f", AnglePID.Target);
		OLED_Printf(0, 40, OLED_6X8, "A:%+05.1f", AngleAcc);
		OLED_Printf(0, 48, OLED_6X8, "O:%+05.0f", AnglePID.Out);
		OLED_Printf(0, 56, OLED_6X8, "GX:%+05d", GX);
		OLED_Printf(50, 0, OLED_6X8, "  Speed");
		OLED_Printf(50, 8, OLED_6X8, "%05.2f", SpeedPID.Kp);
		OLED_Printf(50, 16, OLED_6X8, "%05.2f", SpeedPID.Ki);
		OLED_Printf(50, 24, OLED_6X8, "%05.2f", SpeedPID.Kd);
		OLED_Printf(50, 32, OLED_6X8, "%+05.1f", SpeedPID.Target);
		OLED_Printf(50, 40, OLED_6X8, "%+05.1f", AveSpeed);
		OLED_Printf(50, 48, OLED_6X8, "%+05.0f", SpeedPID.Out);
		OLED_Update();
		BlueSerial_Printf("[plot,%f,%f,%f]", AngleAcc, AngleGyro, Angle);
		Delay_ms(5);                     
	}
}

//1ms中断
void TIM3_IRQHandler(void)
{
	static uint16_t Count0 , Count1;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
		Count0++;
		
		if(Count0 >= 10)
		{
			Count0 = 0;
			
			//MPU6050获得原始数据
			MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
			
			if(GX < 30 && GX >-120){ GX = 0; }/*传感器本身存在零偏误差（陀螺仪静止时读数不为 0），		
												这里是手动的零偏校准减少后续积分漂移。*/
			
			//弧度转角度
			float ax_square = (float)AX * AX;    // AX的平方
			float az_square = (float)AZ * AZ;    // AZ的平方
			float sqrt_ax_az = sqrt(ax_square + az_square); 
			AngleAcc = -atan2((float)AY, sqrt_ax_az) * 180.0f / 3.1415926535f;
			
			//角度偏移校准：补偿安装或重力的偏移
			AngleAcc += 3;
			
			//互补滤波
			AngleGyro = Angle + GX /32768.0 * 2000 * 0.01;/*用陀螺仪积分计算角度:当前角度 + 角速度 * 时间;
															Angle + 角速度 * 时间：陀螺仪积分（陀螺仪预测姿态）*/	
			//简单互补滤波融合角度
			float Alpha = 0.18;
			Angle = Alpha * AngleAcc + (1 - Alpha) * AngleGyro;
			if( Angle > 100 ||  Angle < -100 )
			{
				RunFlag = 0;
			}	
			
			//接收到1才开始PID调控
			if(RunFlag) 
			{
				//角度环
				AnglePID.Actual = Angle;
				PID_Update(&AnglePID);
				AvePWM = -AnglePID.Out; 
				
				LeftPWM = AvePWM + DifPWM / 2;
				RightPWM = AvePWM - DifPWM / 2;
				
				
				//限幅
				if(LeftPWM >= 99) { LeftPWM = 99;} else if(LeftPWM < -99) { LeftPWM = -99;}
				if(RightPWM >= 99) { RightPWM = 99;} else if(RightPWM < -99) { RightPWM = -99;}
				
				Motor_SetPWM(1,LeftPWM);
				Motor_SetPWM(2,RightPWM);
				
			
			}
			else
			{
				Motor_SetPWM(1,0);
				Motor_SetPWM(2,0);
			}
		}
		
		//速度环
		Count1++;
		if(Count1 >= 50)
		{
			Count1 = 0;
			LeftSpeed = -Encoder_Get(1) / 44.0 /0.05 /10.71;
			RightSpeed = -Encoder_Get(2) / 44.0 /0.05 /10.71;
			
			AveSpeed = (LeftSpeed + RightSpeed) / 2;
			DifSpeed = LeftSpeed - RightSpeed;
			
			if(RunFlag)
			{
				SpeedPID.Actual = AveSpeed;
				PID_Update(&SpeedPID);
				AnglePID.Target = SpeedPID.Out;
				
				//转向环（也用50ms直接写了）
				TurnPID.Actual = DifSpeed;
				PID_Update(&TurnPID);
				DifPWM = TurnPID.Out;
			}
		}
		
		
		
		//不超时标志位为0，进不去
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
		{
			Timer3ErrorFlag = 1; 
            TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 

		}
		TimerCount = TIM_GetCounter(TIM3);
	}
}
