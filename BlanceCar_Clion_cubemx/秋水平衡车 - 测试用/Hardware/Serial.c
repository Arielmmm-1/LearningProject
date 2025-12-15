#include "stm32f10x.h" 
#include <stdarg.h>
#include <stdio.h>
#include <string.h>  // 新增：DMA发送需要字符串长度函数
/*这里用USART3来当串口，收发较简单*/

/************************** 新增：环形缓冲区配置 --豆包--*******************/
#define USART3_BUF_SIZE  128  // 修复：增大接收缓冲区到128字节，减少接收丢包
uint8_t usart3_buf[USART3_BUF_SIZE];  // 存数据的"临时柜子"
uint8_t buf_w = 0;                    // 写指针（中断中用）
uint8_t buf_r = 0;                    // 读指针（查询中用）
uint8_t buf_len = 0;                  // 缓冲区已有数据长度

/************************** 新增：DMA相关配置（简单注释） *******************/
#define USART3_DMA_BUF_LEN 128         // 修复：增大DMA发送缓冲区到128字节
#define USART3_TX_BUF_SIZE  64         // 新增：DMA发送环形缓冲区大小
uint8_t usart3_dma_buf[USART3_DMA_BUF_LEN]; // DMA发送缓冲区
uint8_t dma_sending_flag = 0;          // DMA发送中标志（0=空闲，1=发送中）
// 新增：DMA发送环形缓冲区（解决覆盖问题）
uint8_t usart3_tx_buf[USART3_TX_BUF_SIZE];
uint8_t tx_buf_w = 0;
uint8_t tx_buf_r = 0;
uint8_t tx_buf_len = 0;

/************************** 新增：缓冲区操作函数 **************************/
// 1. 中断中调用：往缓冲区写1个字节
static void USART3_Buf_Write(uint8_t data) {
    uint8_t next_w = (buf_w + 1) % USART3_BUF_SIZE;
    if (next_w == buf_r) return;  // 修复：环形缓冲区满判断更严谨，避免溢出
    usart3_buf[buf_w] = data;                // 数据写入当前位置
    buf_w = next_w;                          // 写指针后移，环形绕回
    buf_len++;                                // 数据长度+1
}

// 2. 查询中调用：从缓冲区读1个字节
static uint8_t USART3_Buf_Read(void) {
    uint8_t data = 0;
    if (buf_len == 0) return 0;              // 缓冲区空，返回0
    data = usart3_buf[buf_r];                // 读取数据
    buf_r = (buf_r + 1) % USART3_BUF_SIZE;   // 读指针后移，环形绕回
    buf_len--;                                // 数据长度-1
    return data;
}

// 新增：DMA发送缓冲区写函数（环形缓存，避免覆盖）
static void USART3_TxBuf_Write(uint8_t *data, uint16_t len) {
    for(uint16_t i=0; i<len; i++) {
        uint8_t next_w = (tx_buf_w + 1) % USART3_TX_BUF_SIZE;
        if(next_w == tx_buf_r) break; // 缓冲区满则停止写入，避免丢包
        usart3_tx_buf[tx_buf_w] = data[i];
        tx_buf_w = next_w;
        tx_buf_len++;
    }
}

// 新增：DMA发送缓冲区读函数
static uint16_t USART3_TxBuf_Read(uint8_t *data, uint16_t max_len) {
    uint16_t read_len = 0;
    while(tx_buf_len > 0 && read_len < max_len) {
        data[read_len] = usart3_tx_buf[tx_buf_r];
        tx_buf_r = (tx_buf_r + 1) % USART3_TX_BUF_SIZE;
        tx_buf_len--;
        read_len++;
    }
    return read_len;
}

/************************** 新增：DMA初始化函数（简单注释） *******************/
static void USART3_DMA_Init(void)
{
    // 开启DMA1时钟（USART3_TX对应DMA1_Channel2）
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    DMA_InitTypeDef DMA_InitStruct;
    // 配置DMA外设地址：USART3数据寄存器
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
    // 配置DMA内存地址：发送缓冲区
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)usart3_dma_buf;
    // 数据方向：内存→外设（发送）
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    // DMA缓冲区长度
    DMA_InitStruct.DMA_BufferSize = USART3_DMA_BUF_LEN;
    // 外设地址不递增
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    // 内存地址递增
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    // 数据宽度：字节
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    // 正常模式（发送完停止）
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    // 修复：提高DMA优先级，避免被抢占
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    // 非内存到内存
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    // 初始化DMA通道2
    DMA_Init(DMA1_Channel2, &DMA_InitStruct);
    
    // 开启DMA发送完成中断
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
    // 开启USART3的DMA发送功能
    USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
    // 初始化后关闭DMA通道（发送时再开启）
    DMA_Cmd(DMA1_Channel2, DISABLE);
    
    // 配置DMA中断优先级
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    // 修复：提高DMA中断优先级，避免被抢占
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

//串口初始化
void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	//USART3_TX(发送)   PB10
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//USART3_RX(接收)   PB11
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//接收直接用浮空输入
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//配置USART3
	USART_InitTypeDef USART_InitStruct;

	USART_InitStruct.USART_BaudRate= 9600;//波特率
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_StopBits=USART_StopBits_1;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART3,&USART_InitStruct);
	
	//配置NVIC
	NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;           // USART3中断通道
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;   // 抢占优先级（平衡车场景设为2，低于MPU6050）
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;          // 子优先级
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;             
    NVIC_Init(&NVIC_InitStruct);
	
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//开启中断，判断条件为：接收数据中断标志位
	USART_Cmd(USART3,ENABLE);
	
    // 新增：初始化DMA（不改动原有串口初始化逻辑）
    USART3_DMA_Init();
}

/************************** 新增：DMA发送完成中断服务函数 *******************/
void DMA1_Channel2_IRQHandler(void)
{
    // 检查DMA发送完成中断标志
    if(DMA_GetITStatus(DMA1_IT_TC2) != RESET)
    {
        dma_sending_flag = 0;                  // 标记发送完成
        DMA_ClearITPendingBit(DMA1_IT_TC2);     // 清除中断标志
        DMA_Cmd(DMA1_Channel2, DISABLE);       // 关闭DMA通道
        
        // 新增：发送完成后，继续读取缓冲区数据发送
        uint16_t read_len = USART3_TxBuf_Read(usart3_dma_buf, USART3_DMA_BUF_LEN);
        if(read_len > 0) {
            dma_sending_flag = 1;
            DMA_SetCurrDataCounter(DMA1_Channel2, read_len);
            DMA_Cmd(DMA1_Channel2, ENABLE);
        }
    }
}
	
//中断接收,存入缓冲区
//接收 --> 标志位置一 --> 将数据存进缓冲区 --> 清除标志位
void USART3_IRQHandler(void) {
    uint8_t data;
    // 检查是否是接收非空中断
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) 
	{
      data = (uint8_t)USART_ReceiveData(USART3);  // 读数据
      USART3_Buf_Write(data);                     // 存入缓冲区
      USART_ClearITPendingBit(USART3, USART_IT_RXNE);  // 清中断标志
	}
}

//发一个字节
void USART3_SendByte(uint8_t byte)
{
    USART_SendData(USART3, byte);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

//发字符串
void USART3_SendString(char* str)
{
    while (*str != '\0')
    {
        USART3_SendByte((uint8_t)*str);
        str++;
    }
}

/************************** 新增：DMA发送字符串函数（简单注释） *******************/
void USART3_SendString_DMA(char* str)
{
    uint16_t send_len = strlen(str);  // 获取字符串长度
    if(send_len == 0) return;
    
    // 第一步：先写入发送环形缓冲区，避免覆盖
    USART3_TxBuf_Write((uint8_t*)str, send_len);
    
    // 第二步：若DMA空闲，立即发送
    if(!dma_sending_flag) {
        uint16_t read_len = USART3_TxBuf_Read(usart3_dma_buf, USART3_DMA_BUF_LEN);
        if(read_len > 0) {
            dma_sending_flag = 1;
            DMA_SetCurrDataCounter(DMA1_Channel2, read_len);
            DMA_Cmd(DMA1_Channel2, ENABLE);
        }
    }
}

/************************** 新增：DMA版本Printf（简单注释） *******************/
void USART3_Printf_DMA(char *format, ...)
{
    char String[100];  // 复用原有缓冲区大小
    va_list arg;
    
    va_start(arg, format);
    // 修复：使用vsnprintf防止缓冲区溢出
    vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);
    
    // 调用DMA发送字符串
    USART3_SendString_DMA(String);
}

// 接收一个字节（从缓冲区读，不再死等硬件）
uint8_t USART3_ReceiveByte(void)
{
    while (buf_len == 0);  // 死等缓冲区有数据（也可改成非阻塞，返回0）
    return USART3_Buf_Read();
}

// 检查是否有数据可供读取（从缓冲区判断，不是硬件标志）
_Bool USART3_DataAvailable(void)
{
    return (buf_len > 0) ? 1 : 0;  // 缓冲区有数据返回1，否则0
}

void USART3_Printf(char *format, ...)
{
    char String[100];  // 缓冲区，可根据需要调整大小（最大100字节）
    va_list arg;       // 可变参数列表
    
    // 1. 初始化可变参数
    va_start(arg, format);
    // 2. 格式化拼接字符串（核心：把%d/%f/%s等转为字符串）
    // 修复：使用vsnprintf防止缓冲区溢出
    vsnprintf(String, sizeof(String), format, arg);
    // 3. 结束可变参数
    va_end(arg);
    // 4. 调用现有发送函数发送拼接后的字符串
    USART3_SendString(String);
}

/*主函数使用

// 检查缓冲区是否有数据
        if (USART3_DataAvailable() == 1)
        {
            // 从缓冲区读1个字节（不会丢数据）
            uint8_t data = USART3_ReceiveByte();
            // 处理数据（比如回显）
            USART3_SendString("收到数据：");
            USART3_SendByte(data);
            USART3_SendString("\r\n");
        }


// 新增DMA使用示例：
// USART3_SendString_DMA("Hello DMA!\r\n");  // DMA发送字符串
// USART3_Printf_DMA("[plot,%f]\r\n",AngleAcc); // DMA发送格式化字符串

        // 其他功能（比如控制LED闪烁）

*/
