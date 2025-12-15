#include "stm32f10x.h"                  // Device header

uint8_t Date = 0;  // 初始值为0，接收成功后更新

void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
		
	GPIO_InitTypeDef GPIO_InitStructure;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate= 9600;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None; //硬件流控制，不需要
	USART_InitStructure.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;  //即接收也输出
	USART_InitStructure.USART_Parity=USART_Parity_No; 				//奇偶验证不需要
	USART_InitStructure.USART_StopBits=USART_StopBits_1;			//停止位
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;		//字长
	USART_Init(USART1,&USART_InitStructure);

	USART_Cmd(USART1, ENABLE);	
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);	//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//0是没发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

void Serial_ReceiveByte(void)
{
	// 检测 USART_FLAG_RXNE 标志位（接收缓冲区非空，即有数据接收）
	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
	{
		Date = USART_ReceiveData(USART1);  // 读取接收数据，存入 Date
		/* 接收数据后，USART_FLAG_RXNE 标志位会自动清除，无需手动操作 */
	}
}
