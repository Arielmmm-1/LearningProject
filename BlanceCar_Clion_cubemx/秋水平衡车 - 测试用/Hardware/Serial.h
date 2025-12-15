#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"

/************************** 环形缓冲区配置 *******************/
// 增大后的接收缓冲区大小
#define USART3_BUF_SIZE    128
// DMA发送缓冲区大小
#define USART3_DMA_BUF_LEN 128
// DMA发送环形缓冲区大小
#define USART3_TX_BUF_SIZE  64

/************************** 全局变量声明（加extern） *******************/
// 接收缓冲区相关
extern uint8_t usart3_buf[USART3_BUF_SIZE];
extern uint8_t buf_w;
extern uint8_t buf_r;
extern uint8_t buf_len;

// DMA发送相关
extern uint8_t usart3_dma_buf[USART3_DMA_BUF_LEN];
extern uint8_t dma_sending_flag;
extern uint8_t usart3_tx_buf[USART3_TX_BUF_SIZE];
extern uint8_t tx_buf_w;
extern uint8_t tx_buf_r;
extern uint8_t tx_buf_len;

/************************** 函数声明 *******************/
// 串口初始化（含DMA初始化）
void Serial_Init(void);

// 中断服务函数声明
void USART3_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);

// 阻塞式发送函数（原有）
void USART3_SendByte(uint8_t byte);
void USART3_SendString(char* str);

// DMA非阻塞发送函数（新增+修复）
void USART3_SendString_DMA(char* str);
void USART3_Printf_DMA(char *format, ...);

// 接收函数（原有）
uint8_t USART3_ReceiveByte(void);
_Bool USART3_DataAvailable(void);

// 阻塞式格式化打印（原有）
void USART3_Printf(char *format, ...);

#endif
