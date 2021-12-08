/**
	*串口发送数据就用printf函数打印，但是要包括stdio.h头文件，同时在 stm32f4xx_hal.c 中重写fget和fput函数：
	*****************************************************************************************************
  * 函数功能: 重定向c库函数printf到DEBUG_USARTx
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  *
	*int fputc(int ch, FILE *f)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
		return ch;
	}
	 
	*
	*	函数功能: 重定向c库函数getchar,scanf到DEBUG_USARTx
	*	输入参数: 无
	*	返 回 值: 无
	*	说    明：无
	*	
	int fgetc(FILE *f)
	{
		uint8_t ch = 0;
		HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
		return ch;
	}
	*********************************************************************************************************
	*UART接收中断
	*因为中断接收函数只能触发一次接收中断，所以我们需要在中断回调函数中再调用一次中断接收函数
	*
	*具体流程：
	*1、初始化串口
	*
	*2、在main中第一次调用接收中断函数
	*
	*3、进入接收中断，接收完数据? 进入中断回调函数
	*
	*4、修改HAL_UART_RxCpltCallback中断回调函数，处理接收的数据，
	*
	*5、回调函数中要调用一次HAL_UART_Receive_IT函数，使得程序可以重新触发接收中断
	*
	*函数流程图：
	*
	*HAL_UART_Receive_IT(中断接收函数(开关))?? ?->??USART1_IRQHandler(void)(中断服务函数)? ? ->??? HAL_UART_IRQHandler(UART_HandleTypeDef *huart)(中断处理函数)? ? ->??? UART_Receive_IT(UART_HandleTypeDef *huart) (接收函数)? ?->??? HAL_UART_RxCpltCallback(huart);(中断回调函数)
	*
	*HAL_UART_RxCpltCallback函数就是用户要重写在main.c里的回调函数。
	*/


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
//uint8_t RxByte;
uint8_t RxBuff[100];
//uint16_t Rx_Count;

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart1,RxBuff,10);//打开接收中断开关，接受10个字符，注意区别 UART_Receive_IT 、HAL_UART_Receive_IT、HAL_UART_Receive
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
//		printf("Please enter your strings\n\t");
		
		
    /* USER CODE BEGIN 3 */
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
  
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file
   */
//	unsigned char get[] = "I get";
		printf("I get\t");
		printf("%s\t",RxBuff);
		HAL_UART_Transmit(&huart1,RxBuff,10,0xff);//发送十个字符，不能超过10个字符，否则数组会越界
//	RxBuff[Rx_Count++]=RxByte;//在主函数的初始化的时候已经将RxByte的地址赋值给pRxBuffPtr，这里已经的RxByte代表了字符串的第一个字符
//	
//	if(RxByte==0x0A)
//	{
//		while(HAL_UART_Transmit_IT(&huart1,RxBuff,Rx_Count)==HAL_OK);
//		
//		Rx_Count=0;
//	}
//	if(Rx_Count>=254)
//	{
//		Rx_Count=0;
//	}
	while(HAL_UART_Receive_IT(&huart1,RxBuff,10)==HAL_OK);//打开串口中断
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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
