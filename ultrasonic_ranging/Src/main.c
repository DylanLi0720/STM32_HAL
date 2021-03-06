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
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "oled.h"
#include "stdio.h"
#define CPU_FREQUENCY_MHZ    168

static __IO uint32_t TimingDelay;

uint8_t  	Channel2Edge = 0;
uint32_t 	Channel2Period;
uint32_t  distance = 5100;
uint16_t  Channel2Percent;
uint16_t  Channel2HighTime,count = 0,Channel2Risingstart;
uint16_t 	Channel2RisingTimeLast=0, Channel2RisingTimeNow, Channel2FallingTime;
//bit7:捕获完成标志
//bit6：捕获到高电平的标志
//bit5~0：捕获高电平后定时器溢出的次数
uint8_t   TIM2CH2_CAPTURE_STA=0;    //记录捕获状态                         
uint32_t    TIM2CH2_CAPTURE_VAL;    //记录捕获到下降沿时TIM2_CNT的值
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
void gases_start_signal(void);
void delay_us(__IO uint32_t nTime);

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
//	extern I2C_HandleTypeDef hi2c1;
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
  MX_TIM2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	HAL_I2C_MspInit(&hi2c1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);//开启输入捕获中断
  __HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);   //开启定时器2通道2的中断，即计数到ARR会产生中断
	OLED_Init ();
	OLED_Clear();
  /* USER CODE END 2 */
	uint8_t  string[] = "The distance is ";

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		OLED_ShowString(1,1,string,18);
		OLED_ShowNum(3,3,distance,5,16);
		gases_start_signal(); 
		
		if(TIM2CH2_CAPTURE_STA&0X80)        //一次捕获完成
		{
				HAL_Delay(1000);
				__HAL_TIM_SET_COUNTER(&htim2,0);//将CNT的值设置为0
				__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);//切换捕获极性
				__HAL_TIM_ENABLE(&htim2);//使能计数器
				TIM2CH2_CAPTURE_STA=0; 
		}

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
  /** Initializes the CPU, AHB and APB busses clocks 
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

//CNT到达ARR时进入此中断
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		distance++;
    if((TIM2CH2_CAPTURE_STA&0X80)==0)//一次捕获都未完成
    {
				if(TIM2CH2_CAPTURE_STA&0X40)//如果之前捕获到上升沿
				{
						if((TIM2CH2_CAPTURE_STA&0X3F)==0X3F)//STA的计数值已计满
						{
								TIM2CH2_CAPTURE_STA|=0X80;      //强制使最高位置1，即完成一次捕获
								TIM2CH2_CAPTURE_VAL=0XFFFF;
						}
						else TIM2CH2_CAPTURE_STA++;//溢出一次STA加1
				}    
    }       
}

//检测上升沿或者下降沿
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{			 distance ++;
    if((TIM2CH2_CAPTURE_STA&0X80)==0)//STA的最高位未置零，即一次捕获未结束
    {
			
			if(TIM2CH2_CAPTURE_STA&0X40)        //若之前检测到上升沿    
			{               
					TIM2CH2_CAPTURE_STA|=0X80;      //将STA的最高位置1，表示一次捕获已完成
					TIM2CH2_CAPTURE_VAL=HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_2);//读取此时的CNT

					long long temp=0;
					temp=TIM2CH2_CAPTURE_STA&0X3F;//此时temp表示溢出次数 
					temp*=0XFFFF;               
					temp+=TIM2CH2_CAPTURE_VAL;      //temp表示一次捕获总的计数
//					printf("The time is %lld us\r\n",temp);
					distance = temp*170;
				
					__HAL_TIM_DISABLE(&htim2);        //关闭计数器
			}
			else                                //如果检测到上升沿
			{
					TIM2CH2_CAPTURE_STA=0;         
					TIM2CH2_CAPTURE_VAL=0;
					TIM2CH2_CAPTURE_STA|=0X40;      //检测到上升沿，将STA的次高位置零
					__HAL_TIM_SET_COUNTER(&htim2,0);//将CNT置零
					__HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);//切换捕获极性          
			}       

		}
}



/* USER CODE BEGIN 4 */
void gases_start_signal(void)
{  
 
  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
  //延时20us
  delay_us (10);
// HAL_Delay(10);
  HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
//	distance++;
//	delay_us(250);
//	
//	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
//  //延时20us
//  delay_us (10);
//	
//	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}


/* Private function prototypes -----------------------------------------------*/




void delay_us(__IO uint32_t nTime)
{
    int old_val,new_val,val;

    if(nTime > 900)
    {
        for(old_val = 0; old_val < nTime/900; old_val++)
        {
            delay_us(900);
        }
        nTime = nTime%900;
    }

    old_val = SysTick->VAL;
    new_val = old_val - CPU_FREQUENCY_MHZ*nTime;
    if(new_val >= 0)
    {
        do
        {
            val = SysTick->VAL;
        }
        while((val < old_val)&&(val >= new_val));
    }
    else
    {
        new_val +=CPU_FREQUENCY_MHZ*1000;
        do
        {
            val = SysTick->VAL;
        }
        while((val <= old_val)||(val > new_val));

    }
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
