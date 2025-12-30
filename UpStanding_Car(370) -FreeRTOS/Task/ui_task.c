#include "common.h"

extern uint8_t display_buf[20];
extern float pitch,roll,yaw;
extern int Encoder_Left,Encoder_Right;

extern SemaphoreHandle_t g_ctrl_share_mutex;
extern SemaphoreHandle_t g_oled_mutex;

void ui_task(void const * argument)
{		
	TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xCycle = pdMS_TO_TICKS(100); // 100ms
	
	//本地缓存变量：避免长期持有共享数据互斥量
    int local_Encoder_Left, local_Encoder_Right;
    float local_roll;
	
	for(;;){
		
		// 1. 快速获取共享数据互斥量，仅读取数据后立即释放
        if (xSemaphoreTake(g_ctrl_share_mutex, portMAX_DELAY) == pdPASS)
        {
            // 仅拷贝共享数据到本地变量，不执行其他耗时操作
            local_Encoder_Left = Encoder_Left;
            local_Encoder_Right = Encoder_Right;
            local_roll = roll;
            // 立即释放共享数据互斥量，缩短持有时间
            xSemaphoreGive(g_ctrl_share_mutex);
        }
		

        // 2. 操作OLED前，获取OLED专属互斥量（保护显示屏独占资源）
        if (xSemaphoreTake(g_oled_mutex, portMAX_DELAY) == pdPASS)
        {
           
            sprintf((char *)display_buf,"Encoder_L:%d   ",local_Encoder_Left);
            OLED_ShowString(0,0,display_buf,16);
            sprintf((char *)display_buf,"Encoder_R:%d   ",local_Encoder_Right);
            OLED_ShowString(0,2,display_buf,16);		
            sprintf((char *)display_buf,"roll:%.1f   ",local_roll); 
            OLED_ShowString(0,4,display_buf,16);

            // 释放OLED互斥量
            xSemaphoreGive(g_oled_mutex);
        }

		vTaskDelayUntil(&xLastWakeTime, xCycle);
	}
}
