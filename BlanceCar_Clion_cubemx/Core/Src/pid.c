#include "pid.h"
#include "encoder.h"
#include "inv_mpu.h"
#include "mpu6050.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "motor.h"

float Vertical_Kp=-120,Vertical_Kd=-0.75;			//直立环 数量级（Kp：0~1000、Kd：0~10）
float Velocity_Kp=-1.1,Velocity_Ki=-0.0055;		//速度环 数量级（Kp：0~1）
float Turn_Kp=10,Turn_Kd=0.l;											//转向环

//传感器数据变量
int Encoder_Left, Encoder_Right;
float pitch, roll, yaw;
short gyrox,gyroy,gyroz;
short accx,accy,accz;

//闭环控制的中间变量
int Vertical_out,Velocity_out,Turn_out,Target_Speed = 0,Target_turn = 0;
int Motor1,Motor2;
float Med_Angle = 2.5;//平衡时角度值偏移量


extern TIM_HandleTypeDef htim2 ,htim4;
extern float distance;
extern uint8_t Fore,Back,Left,Right;
#define SPEED_Y 8 //俯仰(前后)最大设定速度
#define SPEED_Z 150//偏航(左右)最大设定速度
uint8_t stop;

//直立环PD
//输入参数：期望角度，实际角度，角速度
int Vertical(float Med,float Angle,float gyro_Y)
{
    int temp;
    temp = Vertical_Kp*(Angle-Med)+Vertical_Kd*gyro_Y;
    return temp;
}

//速度环
//输入参数：期望速度，左编码器，右编码器
int Velocity(int Target,int encoder_L,int encoder_R)
{
    static int Err_LowOut_Last,Encoder_S;
    static float a = 0.7;
    Velocity_Ki = Velocity_Kp/200; //书本经验之谈
    int Err,Err_LowOut,temp;
    //1.计算偏差
    Err = (encoder_L + encoder_R) - Target;
    //2.低通滤波(过滤掉速度偏差中的高频噪)
    Err_LowOut = (1 - a)*Err + a*Err_LowOut_Last;
    Err_LowOut_Last = Err_LowOut;
    //3.j积分
    Encoder_S += Err_LowOut;
    //4.积分限幅(类似与ifelse，简易一点，-20000 < Encoder_S < 20000)
    Encoder_S = Encoder_S >20000?20000:(Encoder_S < -20000?-20000:Encoder_S);
    if (stop == 1)Encoder_S = 0 ,stop = 0;
    //5.速度环计算
    temp = Velocity_Kp * Err_LowOut + Velocity_Ki * Encoder_S;
    return temp;
}

//转向环
//输入参数：角速度，角度值
int Turn(float gyro_Z,int Target_turn)
{
    int temp;
    temp = Turn_Kp *Target_turn +Turn_Kd *gyro_Z;
    return temp;
}

//核心控制函数
//c8t6的中断已经用完了，又要10ms执行一次Control函数,用MPU6050的外部中断
//同意在Sr04.c的HAL_GPIO_EXTI_Callback里面了
void Control(void)
{
    int PWM_out;
    //1.读取编码器和陀螺仪值
    Encoder_Left = Read_Speed(&htim2);
    Encoder_Right = -Read_Speed(&htim4);
    mpu_dmp_get_data(&pitch,&roll,&yaw);
    MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);
    MPU_Get_Accelerometer(&accx,&accy,&accz);

    //3.遥控部分
    if((Fore==0)&&(Back==0))Target_Speed=0;//未接受到前进后退指令-->速度清零，稳在原地
    if(Fore==1)
    {
        //采用递增递减的策略，而不是直接赋值，让小车稳定
        if(distance<50)
            Target_Speed++;
        else
            Target_Speed--;
    }
    if(Back==1){Target_Speed++;}
    Target_Speed=Target_Speed>SPEED_Y?SPEED_Y:(Target_Speed<-SPEED_Y?(-SPEED_Y):Target_Speed);//限幅

    /*左右*/
    if((Left==0)&&(Right==0))Target_turn=0;
    if(Left==1)Target_turn+=30;	//左转
    if(Right==1)Target_turn-=30;	//右转
    Target_turn=Target_turn>SPEED_Z?SPEED_Z:(Target_turn<-SPEED_Z?(-SPEED_Z):Target_turn);//限幅( (20*100) * 100   )

    /*转向约束*/
    if((Left==0)&&(Right==0))Turn_Kd=0.6;//若无左右转向指令，则开启转向约束
    else if((Left==1)||(Right==1))Turn_Kd=0;//若左右转向指令接收到，则去掉转向约束


    //2.将数据传入PID控制器，计算输出结果,左右电机的转速值
    Velocity_out = Velocity(Target_Speed,Encoder_Left,Encoder_Right);
    Vertical_out = Vertical(Velocity_out+Med_Angle,roll,gyrox);//gyrox是转轴
    Turn_out = Turn(gyroz,Target_turn);
    PWM_out = Vertical_out;
    Motor1 = PWM_out - Turn_out;
    Motor2 = PWM_out + Turn_out;
    limit(&Motor1,&Motor2);
    load(Motor1,Motor2);
    Stop(&Med_Angle,&roll);//安全检测
}








