/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* USER CODE BEGIN PV */
volatile uint16_t adc[2]={0,0}; // значения с АЦП с двух каналов
volatile float voltage[2]={0,0}; //расчитываемое напряжение
volatile uint8_t flag = 0; // флаг для колбэка
char buffer[12]; // буффер для числа
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) // Колбэк по прерыванию от работы ADC
{
    if(hadc->Instance == ADC1) //проверка если прерывание от ADC1
    {
    	 flag = 1;
    }
}
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
  MX_DMA_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  Lcd_PortType ports[]={GPIOB,GPIOB,GPIOB,GPIOB};
  Lcd_PinType pins[]={GPIO_PIN_3,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_6};
  Lcd_HandleTypeDef lcd;
  lcd=Lcd_create(ports,pins,GPIOB,GPIO_PIN_7,GPIOB,GPIO_PIN_8, LCD_4_BIT_MODE); //инициализация ЛСД

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2); //запуск АЦП в двух каналах с помощью прямого доступа к памяти
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  if(flag)
	  {
		  flag = 0; //
		  voltage[0]=adc[0]/4096.0*500.0; //делим полученное измерение на кол-во измерений (2^12) и умножаем на макс. значение вольтметра, получаем вольты
		  voltage[1] = adc[1]/4096.0*500.0; // тоже самое для второго канала (нужен только для режима измерения падения напряжения)


	  if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==GPIO_PIN_SET){ //если выбран режим измерения падения напряжения
		  Lcd_cursor(&lcd,0,0);
		  Lcd_string(&lcd, "Mode: Vdrop"); //вывод на lcd
		  voltage[1]=voltage[1]-voltage[0]; // считаем разницу

	  }
	  if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==GPIO_PIN_SET){ // если выбран режим измерения напряжения
		  Lcd_cursor(&lcd,0,0);
		  Lcd_string(&lcd, "Mode: V    ");//вывод на lcd

	  }

	  if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2)==GPIO_PIN_SET){ // если выбран диапазон 100-500В
		  if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==GPIO_PIN_SET){ // проверка если стоит режим выключенного прибора
			  Lcd_clear(&lcd); // значит держим экран чистым
		  }
		  else{
			  Lcd_cursor(&lcd,0,11);
			  Lcd_string(&lcd, "500 V   "); // вывод выбранного диапазона
			  if (voltage[1]>=499.8){ // если напряжение выше возможного измерить
				  Lcd_string(&lcd, "Voltage limit"); // пишем, что достигли лимита вольтметра
			  }
			  if (voltage[1]>10.0 & voltage[1]<100.0){ // если напряжение ниже выбранного диапазона
				  Lcd_cursor(&lcd,1,0);
				  snprintf(buffer, sizeof(buffer), "%.1f", voltage[1]); //из float в string
				  Lcd_string(&lcd, buffer); // выводим напряжение
				  Lcd_cursor(&lcd,1,4);
				  Lcd_string(&lcd, " V             "); // обозначение + чистим экран от значов из других режимов
			  }
			  if (voltage[1]<10.0){ // если напряжение ниже выбранного диапазона
				  Lcd_cursor(&lcd,1,0);
				  snprintf(buffer, sizeof(buffer), "%.1f", voltage[1]); //из float в string
				  Lcd_string(&lcd, buffer); // выводим напряжение
				  Lcd_cursor(&lcd,1,3);
				  Lcd_string(&lcd, " V              "); // обозначение + чистим экран от значов из других режимов
			  }
			  if (voltage[1]>=100.0){
				  snprintf(buffer, sizeof(buffer), "%.1f", voltage[1]); //из float в string
				  Lcd_cursor(&lcd,1,0);
				  Lcd_string(&lcd, buffer); // выводим напряжение
				  Lcd_cursor(&lcd,1,5);
				  Lcd_string(&lcd, " V            "); // обозначение + чистим экран от значов из других режимов
			  }
		  }
	  }
	  if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3)==GPIO_PIN_SET){ // если выбран диапазон 10-100В
		  if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==GPIO_PIN_SET){ // проверка если стоит режим выключенного прибора
			  Lcd_clear(&lcd); // значит держим экран чистым
		  }
		  else{
			  Lcd_cursor(&lcd,0,11);
			  Lcd_string(&lcd, "100 V   "); // вывод выбранного диапазона
			  if (voltage[1]>=100.0){ // если напряжение выше выбранного диапазона
				  Lcd_cursor(&lcd,1,0);
				  Lcd_string(&lcd, "Voltage limit"); // пишем, что достигли лимита диапазона
		  	  }
			  if (voltage[1]<10.0){ // если напряжение ниже выбранного диапазона
				  Lcd_cursor(&lcd,1,0);
				  snprintf(buffer, sizeof(buffer), "%.2f", voltage[1]); //из float в string
				  Lcd_cursor(&lcd,1,0);
				  Lcd_string(&lcd, buffer); // выводим напряжение
				  Lcd_cursor(&lcd,1,4);
				  Lcd_string(&lcd, " V              "); // обозначение + чистим экран от значов из других режимов
			  }
			  if (voltage[1]>10.0 & voltage[1]<100.0){ // если напряжение в нашем выбранном диапазоне
				  snprintf(buffer, sizeof(buffer), "%.2f", voltage[1]); //из float в string
				  Lcd_cursor(&lcd,1,0);
				  Lcd_string(&lcd, buffer); // выводим напряжение
				  Lcd_cursor(&lcd,1,5);
				  Lcd_string(&lcd, " V             "); // обозначение + чистим экран от значов из других режимов
		  	  }
		  }
	  }
	  if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==GPIO_PIN_SET){ // если выбран диапазон 0-10В
		  if (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==GPIO_PIN_SET){ // проверка если стоит режим выключенного прибора
			  Lcd_clear(&lcd); // значит держим экран чистым
		  }
		  else{
			  Lcd_cursor(&lcd,0,11);
			  Lcd_string(&lcd, "10000 mV"); // вывод выбранного диапазона
			  if (voltage[1]>=10.0){ // если напряжение выше выбранного диапазона
				  Lcd_cursor(&lcd,1,0);
				  Lcd_string(&lcd, "Voltage limit"); // пишем, что достигли лимита диапазона
	  		  }
	  		  else{
	  			  voltage[1]=voltage[1]*1000.0; //переводим в мВ
	  	  		  snprintf(buffer, sizeof(buffer), "%4.0f", voltage[1]); //из float в string
	  	  		  Lcd_cursor(&lcd,1,0);
	  	  		  Lcd_string(&lcd, buffer);// выводим напряжение
	  	  		  Lcd_cursor(&lcd,1,4);
	  	  		  Lcd_string(&lcd, " mV           "); // обозначение + чистим экран от значов из других режимов
	  		  }
		  }
	  }
	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2); //по новой измеряем значение АЦП
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA2 PA3 PA4 PA5
                           PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
                          |GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 PB6
                           PB7 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
