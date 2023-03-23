/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
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
	HAL_StatusTypeDef ret1;
	HAL_StatusTypeDef ret2;
	HAL_StatusTypeDef ret3;
	HAL_StatusTypeDef ret4;
	uint8_t end = 0x64;
	uint32_t saida;
	uint32_t saida1;
	uint32_t saida2;
	uint32_t saida3;
	uint32_t saida4;
	uint8_t saida5;
	//uint8_t reply[20];
	//uint8_t reply1[20];
	uint8_t buf[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t buf1[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

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
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    // wake-up
	    uint8_t data = 0;
	    ret1 = HAL_I2C_Master_Receive(&hi2c1, 0xFE, &data, sizeof(data), 1000); // Ver onde fala do 0XFE
	    HAL_Delay(10); // 2.5 ms para acordar; 45 ms para entrar em sleep

	    // first read: 0 byte read - should receive an ACK
	    ret2 = HAL_I2C_Master_Receive(&hi2c1, 0xC8, &data, 1, 1000);

	    // command packet: 8.5.1
	    // 6.2.1: Word Address Value: COMMAND == 0x03
	    // read command: 8.5.15
	    // read configuration zone: {COMMAND, COUNT, OPCODE, ZONE, ADDRESS_1, ADDRESS_2, CRC_LSB, CRC_MSB}
	    // read configuration zone: {  0x03,    0x07, 0x02, 0x00,      0x00,      0x00,      0xB2,    0x7E}
	    // CRC-16 Polinomial: 0x8005: includes COUNT, OPCODE, ZONE, ADDRESS_1, ADDRESS_2, CRC_LSB, CRC_MSB (does not include COMMAND)
	    // CRC https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
	    // Zone encoding table 8-5

	    //CRC
	   // uint8_t readCommand[10] = {0x03, 0x07, 0x02, 0x00, 0x00, 0x00, 0x1E, 0x2D}; // inverter os últimos 2 bytes de CRC?
	    uint8_t readCommand[10] = {0x03, 0x07, 0x02, 0x80, 0x00, 0x00, 0x09, 0xAD};
	/*
	    uint8_t reply[20];
	    //ret3 = HAL_I2C_Master_Transmit(&hi2c1, 0xC8, &data, sizeof(data), 1000);
	    ret3 = HAL_I2C_Master_Transmit(&hi2c1, 0xC8, readCommand, 8, 1000);
	   // HAL_Delay(5);
	    ret4 = HAL_I2C_Master_Receive(&hi2c1, 0xC8, reply, 20, 1000);
	    HAL_Delay(10);
	*/
	    uint8_t reply[4];
	    HAL_Delay(5);
	    ret3 = HAL_I2C_Master_Transmit(&hi2c1, 0xC8, &data, sizeof(data), 1000); // Tem que enviar 1 byte
	    HAL_Delay(5);
	    saida1 = HAL_I2C_Master_Receive(&hi2c1, 0xC8, reply, 4, 1000); // tem que receber 0x04 0x11 0x33 0x43
	    HAL_Delay(5);
	    uint8_t reply1[32];
	    ret4 = HAL_I2C_Master_Transmit(&hi2c1, 0xC8, readCommand, 8, 1000); // enviar o comando de leitura
	    HAL_Delay(5);
	    saida2 = HAL_I2C_Master_Receive(&hi2c1, 0xC8, reply1, 16, 1000); // tem que receber (byte de tamanho, 35 em decimal) .. 0x01 0x23 ...
	    HAL_Delay(5);
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

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

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
