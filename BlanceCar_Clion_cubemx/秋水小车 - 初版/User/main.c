#include "stm32f10x.h"
#include "PWM.h"
#include "motor.h"
#include "usart.h"
#include "OLED.h"
#include "Delay.h"
#include "Trace.h"

// 模式标志：0=蓝牙模式，1=循迹模式（初始蓝牙模式）
uint8_t mode = 02
;

int main(void)
{
    // 初始化所有模块（只初始化一次）
    OLED_Init();
    Motor_Init();
    Trace_Init();
    USART1_Init(9600); 

    // 开机提示
    USART1_SendString("Car Ready!\r\n");
    USART1_SendString("Send '1'=蓝牙, '2'=循迹\r\n");
    OLED_ShowString(1,1,"Mode: BT");  // 初始显示蓝牙模式

    while(1) 
    {
        /************************** 第一步：检查模式切换指令 **************************/
        // 只要串口有数据，优先处理模式切换（不区分当前模式）
        if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
        {
            char recv = USART_ReceiveData(USART1);  // 直接读取串口数据
            
            // 切换到蓝牙模式（发送'1'）
            if(recv == '1')
            {
                mode = 0;
                USART1_SendString("Mode: Bluetooth\r\n");
                OLED_ShowString(1,1,"Mode: BT");
            }
            // 切换到循迹模式（发送'2'）
            else if(recv == '2')
            {
                mode = 1;
                USART1_SendString("Mode: Trace\r\n");
                OLED_ShowString(1,1,"Mode: TR");
                stop();  // 切换模式时先停车，避免乱动作
            }
            // 蓝牙模式下的控制指令（只在mode=0时生效）
            else if(mode == 0)
            {
                // 执行电机控制
                switch(recv)
                {
                    case 'F': forword(); USART1_SendString("Go Forward\r\n"); break;
                    case 'B': back();    USART1_SendString("Go Back\r\n");    break;
                    case 'L': left();    USART1_SendString("Turn Left\r\n");  break;
                    case 'R': right();   USART1_SendString("Turn Right\r\n"); break;
                    case 'S': stop();    USART1_SendString("Stop\r\n");       break;
                    default: USART1_SendString("Invalid Cmd! F/B/L/R/S\r\n"); break;
                }
            }
            // 循迹模式下忽略控制指令（可选提示）
            else if(mode == 1)
            {
                USART1_SendString("Trace Mode: Cmd Ignored\r\n");
            }
        }

        /************************** 第二步：执行当前模式逻辑 **************************/
        // 循迹模式：持续执行循迹任务
        if(mode == 1)
        {
            Trace_task();  // 你的循迹逻辑
            Delay_ms(20);  // 调节循迹响应速度
        }
        // 蓝牙模式：无需额外操作，指令已在上面处理
        else
        {
            Delay_ms(50);  // 降低CPU占用
        }
    }
}
