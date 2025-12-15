/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "IIC.h"
#include "inv_mpu.h"
#include "mpu6050.h"
#include "stdio.h"
#include "Sr04.h"
#include <math.h>
#include "motor.h"
#include "encoder.h"
#include "string.h"
#include "pid.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char display_buf[32];
char temp_buf[8];
extern uint8_t rx_buf[2];
extern int Encoder_Left, Encoder_Right;
extern float pitch, roll, yaw;
extern short gyrox;
uint32_t sys_tick;

extern float distance;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// 声明自定义函数，避免编译警告
void float2str(float num, char *buf, int n);
void int2str(int num, char *buf, int len);
void Read(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 把自定义函数放在这里                            

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  HAL_Delay(50);
  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  //初始化顺序是有讲究的
  //OLED初始化
  OLED_Init();
  OLED_Clear();
  //开启测速必要
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
  //MPU6050初始化
  MPU_Init();
  mpu_dmp_init();
  //用MPU6050的IN整个10ms中断
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  //开启PWM必要
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  //开启串口接收必要(串口中断)
  HAL_UART_Receive_IT(&huart3, rx_buf, 1);

  OLED_ShowString(0,00,"Init Sucesse",16);
  OLED_Clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


    //以下为基础代码学习使用，并没有优化逻辑，只能最基础的使用
    // int Encoder_Left, Encoder_Right;
    // Read();//编码器测速，370电机在40左右
    //左电机测速显示(pid.c里面重新写了读取速度)
     int2str(Encoder_Left, temp_buf, 5);
     OLED_ShowString(0, 0, (uint8_t*)"Encoder_L:", 16);
     OLED_ShowString(80, 0, (uint8_t*)temp_buf, 16);
    //右电机测速显示
     int2str(Encoder_Right, temp_buf, 5);
     OLED_ShowString(0, 2, (uint8_t*)"Encoder_R:", 16);
     OLED_ShowString(80, 2, (uint8_t*)temp_buf,  16);

     //角度获取显示
     //float pitch, roll, yaw;
     mpu_dmp_get_data(&pitch, &roll, &yaw);
     float2str(-roll, temp_buf, 2);
     sprintf(display_buf, "roll:%s   ", temp_buf);
     OLED_ShowString(0, 4,(uint8_t*) display_buf, 16);

    //非中断发送角度
     // uint16_t send_len = strlen(display_buf);
     // HAL_UART_Transmit(&huart3, (uint8_t*)display_buf, send_len, 100);

    //距离获取显示
    GET_Distance();
    float2str(distance, temp_buf, 2);
    sprintf(display_buf, "distance:%s   ", temp_buf);
    OLED_ShowString(0, 6, (uint8_t*)display_buf, 12);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//*1确保Read函数的执行频率不会超过每 10 毫秒一次
// void  Read()
// {
//   if (uwTick-sys_tick<10)
//   return;
//   sys_tick =uwTick;
//   Encoder_Left = -Read_Speed(&htim2);
//   Encoder_Right = Read_Speed(&htim4);
// }

//*2用来处理浮点数
void float2str(float num, char *buf, int n)
{
  // 处理符号
  int sign = 0;
  if (num < 0.0f)
  {
    sign = 1;
    num = fabsf(num); // 用fabsf更适合浮点数，避免负数取绝对值的问题
  }

  // 提取整数部分
  int integer_part = (int)num;
  // 提取小数部分（乘以10^n后取整）
  float decimal_part = num - (float)integer_part;

  // 解决pow精度问题：手动计算10的n次方，不用pow函数
  long decimal_int = 0;
  long scale = 1; // 存储10^n
  for (int i = 0; i < n; i++)
  {
    decimal_part *= 10.0f;
    scale *= 10;
  }
  decimal_int = (long)(decimal_part + 0.5f); // 四舍五入

  // 转换整数部分为字符串
  char int_buf[10] = {0};
  int i = 0;
  if (integer_part == 0)
  {
    int_buf[i++] = '0';
  }
  else
  {
    while (integer_part > 0)
    {
      int_buf[i++] = (integer_part % 10) + '0';
      integer_part /= 10;
    }
  }

  // 反转整数部分并拼接结果
  int j = 0;
  if (sign)
  {
    buf[j++] = '-';
  }
  // 反转整数部分
  for (int k = i - 1; k >= 0; k--)
  {
    buf[j++] = int_buf[k];
  }
  // 添加小数点
  buf[j++] = '.';
  // 转换小数部分为字符串（用scale代替pow，避免精度问题）
  long temp_scale = scale;
  for (int k = 0; k < n; k++)
  {
    temp_scale /= 10;
    int digit = (decimal_int / temp_scale) % 10;
    buf[j++] = digit + '0';
  }
  // 字符串结束符
  buf[j] = '\0';
}
//*3用来处理正负数
void int2str(int num, char *buf, int len)
{
    // 1. 处理符号（和float2str的符号处理逻辑完全一致）
    int sign = 0;
    int abs_num = num;
    if (num < 0)
    {
        sign = 1;
        abs_num = -num; // 取整数的绝对值
    }

    // 2. 提取整数的每一位（逆序存储到临时缓冲区，和float2str整数部分逻辑一致）
    char int_buf[16] = {0}; // 临时存储逆序的数字，足够容纳int最大位数
    int i = 0;
    if (abs_num == 0)
    {
        // 特殊情况：数字为0，直接存'0'
        int_buf[i++] = '0';
    }
    else
    {
        // 循环提取每一位数字（逆序）
        while (abs_num > 0)
        {
            int_buf[i++] = (abs_num % 10) + '0';
            abs_num /= 10;
        }
    }

    // 3. 拼接结果：处理符号+补位+反转数字（和float2str的拼接逻辑对齐）
    int j = 0;
    int digit_count = i; // 实际提取的数字位数
    int total_digit_len = len; // 指定的显示位数

    if (sign)
    {
        // 负数：先添加负号，负号占一位
        buf[j++] = '-';
    }
    else
    {
        // 正数：可以留一个空格位（也可以去掉，根据需求）
        // buf[j++] = ' ';
    }
  // 不足指定位数，补空格（想要补0的话，把' '换成'0'即可）
  int space_count = total_digit_len - digit_count;
  for (int k = 0; k < space_count; k++)
  {
    buf[j++] = ' ';
  }

  // 反转临时缓冲区的数字，拼接至结果
  for (int k = digit_count - 1; k >= 0; k--)
  {
    buf[j++] = int_buf[k];
  }

  // 4. 添加字符串结束符（必须，避免乱码）
  buf[j] = '\0';
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
