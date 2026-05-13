/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "arm_math.h"
#include "math.h"
#include "AD9833.h"
#include "My_Math.h"
#include "PID.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define	Sample_Size	1024
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
arm_cfft_instance_f32 cfft_handler;
arm_cfft_instance_f32 cfft_handler_512;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t adc_done = 0;	//ADC转换完成标志位
volatile uint8_t adc3_done = 0;

uint16_t Count;		//测试变量
uint8_t Error;

/*
	各频率分量结构体
*/
SignalPara_t FirstSignal = {0};		//最大频率信号结构体
SignalPara_t First3rdSiganl = {0};	//最大频率3次谐波结构体
SignalPara_t First5thSiganl = {0};	////最大频率5次谐波结构体
SignalRatio_t FirstRatio = {0};

SignalPara_t SecondSignal = {0};		//次最大频率信号对应结构体
SignalPara_t Second3rdSiganl = {0};	////次最大频率3次谐波结构体
SignalPara_t Second5thSiganl = {0};	////次最大频率5次谐波结构体
SignalRatio_t SecondRatio = {0};

SignalPara_t Feedback_First = {0};
SignalPara_t Feedback_Second = {0};

float Phase_Diff = 0.0f;

PID_t Phase_PID = {
	.Kp = 0.004,
	.Ki = 0,
	.Kd = 0,

	.OutMax = 50,
	.OutMin = -50,
};

uint16_t FFT_Buff[Sample_Size] = {0};		//ADC采样原始数据
float FFT_IN[Sample_Size * 2] = {0};	
float FFT_Complex[Sample_Size * 2] = {0};	//备份FFT后的复数数组，用于计算相位
float FFT_OUT[Sample_Size / 2] = {0};

uint16_t FeedbackBuff_First[Sample_Size] = {0};
uint16_t FeedbackBuff_Second[Sample_Size] = {0};

uint16_t Feedback_MixBuff[Sample_Size] = {0};
float Feedback_FFTINFirst[Sample_Size] = {0};
float Feedback_FFTOUTFirst[Sample_Size / 4] = {0};
float Feedback_FFTINSecond[Sample_Size] = {0};
float Feedback_FFTOUTSecond[Sample_Size / 4] = {0};


float LastRatio_First3rdval = 0.0f;
float LastRatio_First5thval = 0.0f;
float LastRatio_Second3rdval = 0.0f;
float LastRatio_Second5thval = 0.0f;

float PhaseDiff = 0.0f;

float Beta = 0.99f;

float last_valphase =0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);

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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  MX_TIM4_Init();
  MX_SPI3_Init();
  MX_ADC2_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
  arm_cfft_init_f32(&cfft_handler, Sample_Size);
  arm_cfft_init_f32(&cfft_handler_512, Sample_Size / 2);
  
  AD98331_Init();
  AD98332_Init();
  
  PID_Init(&Phase_PID);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_Base_Start(&htim3);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)FFT_Buff, Sample_Size);
//  HAL_TIM_Base_Start(&htim4);
//  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)Feedback_MixBuff, Sample_Size);

//  AD98331_SetFrequencyQuick(1000, Sine);
//  AD98332_SetFrequencyQuick(10000, Sine);

  while (1)
  {

	  if (adc_done == 1)
	  {
		  HAL_TIM_Base_Stop(&htim3);
		  HAL_ADC_Stop_DMA(&hadc1);
		  adc_done = 0;

		  Math_FFT(FFT_Buff, FFT_IN, FFT_OUT, FFT_Complex, Sample_Size);
		  
		  Math_FindMaxAndSecondMaxFreqAndAmp(FFT_OUT, &FirstSignal, &SecondSignal, Sample_Size);
		  
		  First3rdSiganl = Math_Get3rdHarmonicPara(FFT_OUT, &FirstSignal, Sample_Size);
		  First5thSiganl = Math_Get5thHarmonicPara(FFT_OUT, &FirstSignal, Sample_Size);
		  Second3rdSiganl = Math_Get3rdHarmonicPara(FFT_OUT, &SecondSignal, Sample_Size);
		  Second5thSiganl = Math_Get5thHarmonicPara(FFT_OUT, &SecondSignal, Sample_Size);
		 
		  FirstRatio = Math_CaculateRatio(&FirstSignal, &First3rdSiganl, &First5thSiganl);
		  SecondRatio = Math_CaculateRatio(&SecondSignal, &Second3rdSiganl, &Second5thSiganl);
		  
		  FirstRatio.Ratio3rd = Math_LowPassFilter(LastRatio_First3rdval, FirstRatio.Ratio3rd);
		  FirstRatio.Ratio5th = Math_LowPassFilter(LastRatio_First5thval, FirstRatio.Ratio5th);
		  SecondRatio.Ratio3rd = Math_LowPassFilter(LastRatio_Second3rdval, SecondRatio.Ratio3rd);
		  SecondRatio.Ratio5th = Math_LowPassFilter(LastRatio_Second5thval, SecondRatio.Ratio5th);
		  
		  LastRatio_First3rdval= FirstRatio.Ratio3rd;
		  LastRatio_First5thval = FirstRatio.Ratio5th;
		  LastRatio_Second3rdval = SecondRatio.Ratio3rd;
		  LastRatio_Second5thval = SecondRatio.Ratio5th;
		  
		  SecondSignal.WaveType = Math_WaveformJudegeSecond(&FirstSignal, &FirstRatio, &SecondSignal, &SecondRatio);
		  FirstSignal.WaveType = Math_Math_WaveformJudegeFirst(&FirstRatio);		  		  
		  
		  AD98331_SetFrequencyQuick(FirstSignal.Freq, FirstSignal.WaveType);
		  AD98332_SetFrequencyQuick(SecondSignal.Freq, SecondSignal.WaveType);
		  		  
//		  HAL_TIM_Base_Start(&htim3);
//		  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)FFT_Buff, Sample_Size);
		  HAL_TIM_Base_Start(&htim4);
		  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)Feedback_MixBuff, Sample_Size);

	  }
	  if (adc3_done == 1)
	  {
		  adc3_done = 0;
		  HAL_TIM_Base_Stop(&htim4);
		  HAL_ADC_Stop_DMA(&hadc2);
		  
		  Math_Feedback_SeparateChannel(Feedback_MixBuff,
		  							  Feedback_FFTINFirst,
		  							  Feedback_FFTINSecond,
		  							  Sample_Size);
		  
		  
		  Math_Feedback_FFTAndExtractPhase(&Feedback_First,
		  								 Feedback_FFTINFirst,
		  								 Feedback_FFTOUTFirst,
		  								 Sample_Size);
		  
		  Math_Feedback_FFTAndExtractPhase(&Feedback_Second,
		  								 Feedback_FFTINSecond,
		  								 Feedback_FFTOUTSecond,
		  								 Sample_Size);
		  

		  
		  Phase_Diff = Feedback_First.Phase - Feedback_Second.Phase;
		  
		  if (Phase_Diff >  180.0f) Phase_Diff -= 360.0f;
		  if (Phase_Diff < -180.0f) Phase_Diff += 360.0f;
		  
		  Phase_PID.Actual = Phase_Diff;
		  PID_Update(&Phase_PID);
		  AD98331_SetFrequencyQuick(FirstSignal.Freq - Phase_PID.Out, FirstSignal.WaveType);
		  
		  HAL_TIM_Base_Start(&htim4);
		  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)Feedback_MixBuff, Sample_Size);
	  }
	   

    /* USER CODE END WHILE */

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SPI3
                              |RCC_PERIPHCLK_SPI2;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim3){
		
	}
	if (htim == &htim5){
		  
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc == &hadc1){	
		adc_done ++;
	}
	if (hadc == &hadc2){
		adc3_done = 1;
	}
}


int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
