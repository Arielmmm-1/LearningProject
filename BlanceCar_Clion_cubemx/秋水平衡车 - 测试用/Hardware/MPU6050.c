#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "MPU6050_Reg.h"


#define MPU6050_ADDRESS		0xD0 //数据位包括写  ， 0xD0 | 0x01(0xD1)就是数据位和读


void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data)
{
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(Data);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	Data = MyI2C_ReceiveByte();
	MyI2C_SendAck(1);
	MyI2C_Stop();
	return Data;
}

//连续读取一片地址
void MPU6050_ReadRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count)
{
	uint8_t i;
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
	MyI2C_ReceiveAck();
	for (i = 0; i < Count; i ++)
	{
		DataArray[i] = MyI2C_ReceiveByte();
		if (i < Count - 1)
		{
			MyI2C_SendAck(0);
		}
		else
		{
			MyI2C_SendAck(1);
		}
	}
	MyI2C_Stop();
}

void MPU6050_Init(void)
{
	MyI2C_Init();
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1,0x01);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2,0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV,0x07);//分频
	MPU6050_WriteReg(MPU6050_CONFIG,0x00);//滤波
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG,0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG,0x18);
}

//void MPU6050_GetData(int16_t *AccX,int16_t *AccY,int16_t *AccZ,
//					 int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ)
//{

	/*MPU6050 的加速度计、陀螺仪输出都是16 位有符号整数（范围：-32768 ~ +32767,
											      ,对应物理范围-2000°/s ~ +2000°/s）
	但它的 I2C 寄存器是 8 位的;
	所以会把 16 位数据拆成「高 8 位（H）」和「低 8 位（L）」两个寄存器存储*/

////这个方案太慢了，放不进1ms中断
//	uint8_t DataH,DataL;
//	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
//	*AccX = (int16_t)(DataH << 8) | DataL;  //DataH高八位左移，本身的值并不会改变
//	
//	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
//    DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
//    *AccY = (int16_t)(DataH << 8) | DataL; 


//	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
//    DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
//    *AccZ = (int16_t)(DataH << 8) | DataL; 

//	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
//    DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
//    *GyroX = (int16_t)(DataH << 8) | DataL; 
//	
//	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
//	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
//	*GyroY = (int16_t)(DataH << 8) | DataL; 
//	
//	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
//    DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
//    *GyroZ = (int16_t)(DataH << 8) | DataL;  
//}
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t Data[14];
	
	MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, Data, 14);
	
	*AccX = (Data[0] << 8) | Data[1];
	*AccY = (Data[2] << 8) | Data[3];
	*AccZ = (Data[4] << 8) | Data[5];
	
	*GyroX = (Data[8] << 8) | Data[9];
	*GyroY = (Data[10] << 8) | Data[11];
	*GyroZ = (Data[12] << 8) | Data[13];
}
