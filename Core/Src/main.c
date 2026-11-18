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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "eeprom_24c256.h"
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

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t driver_tx_buffer[16];
uint8_t driver_rx_buffer[16];

volatile HAL_StatusTypeDef driver_init_status = HAL_ERROR;
volatile HAL_StatusTypeDef driver_write_status = HAL_ERROR;
volatile HAL_StatusTypeDef driver_read_status = HAL_ERROR;

volatile uint32_t driver_mismatch_count = 0;
volatile uint8_t driver_verify_ok = 0;
volatile uint8_t driver_test_done = 0;

volatile HAL_StatusTypeDef test_zero_length = HAL_ERROR;
volatile HAL_StatusTypeDef test_null_pointer = HAL_OK;
volatile HAL_StatusTypeDef test_out_of_range = HAL_OK;

volatile uint8_t boundary_test_done = 0;

#define EEPROM_LONG_TEST_SIZE 128U

uint8_t long_tx_buffer[EEPROM_LONG_TEST_SIZE];
uint8_t long_rx_buffer[EEPROM_LONG_TEST_SIZE];

volatile HAL_StatusTypeDef long_write_status = HAL_ERROR;
volatile HAL_StatusTypeDef long_read_status = HAL_ERROR;

volatile uint32_t long_mismatch_count = 0;
volatile uint8_t long_verify_ok = 0;
volatile uint8_t long_test_done = 0;

uint8_t end_tx_data = 0xA5;
uint8_t end_rx_data = 0x00;

volatile HAL_StatusTypeDef end_write_status = HAL_ERROR;
volatile HAL_StatusTypeDef end_invalid_status = HAL_OK;
volatile HAL_StatusTypeDef end_read_status = HAL_ERROR;

volatile uint8_t end_verify_ok = 0;
volatile uint8_t end_test_done = 0;

volatile HAL_StatusTypeDef missing_status = HAL_OK;
volatile uint32_t missing_error = 0;
volatile uint32_t missing_elapsed_ms = 0;
volatile uint8_t missing_test_done = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  /*eeprom_multi_write_status = HAL_ERROR;
  eeprom_multi_ready_status = HAL_ERROR;
  eeprom_multi_read_status = HAL_ERROR;
  eeprom_mismatch_count = 0;
  eeprom_multi_verify_ok = 0;
  eeprom_multi_test_done = 0;

  for(uint8_t i = 0; i < EEPROM_TEST_SIZE; i++){
	  eeprom_tx_buffer[i] = (uint8_t) (0xA0U + i);
	  eeprom_rx_buffer[i] = 0x00;
  }

  eeprom_multi_write_status = HAL_I2C_Mem_Write(&hi2c1,EEPROM_DEV_ADDR,EEPROM_TEST_ADDR,I2C_MEMADD_SIZE_16BIT, eeprom_tx_buffer,EEPROM_TEST_SIZE,100);
  if(eeprom_multi_write_status == HAL_OK){
	  eeprom_multi_ready_status = EEPROM_WaitReady(20);
	  if (eeprom_multi_ready_status == HAL_OK){
		  eeprom_multi_read_status = HAL_I2C_Mem_Read(&hi2c1,EEPROM_DEV_ADDR,EEPROM_TEST_ADDR,I2C_MEMADD_SIZE_16BIT, eeprom_rx_buffer,EEPROM_TEST_SIZE,100);
		  if (eeprom_multi_read_status == HAL_OK){
			  for (uint8_t i = 0; i < EEPROM_TEST_SIZE; i++)
			  {
			       if (eeprom_tx_buffer[i] != eeprom_rx_buffer[i])
			       {
			           eeprom_mismatch_count++;
			       }
			   }
			  if(eeprom_mismatch_count == 0)
				  eeprom_multi_verify_ok = 1;
		  }
	  }
  }
  eeprom_multi_test_done = 1;

  page_offset = page_test_addr % EEPROM_PAGE_SIZE;
  page_remaining = EEPROM_PAGE_SIZE - page_offset;
  chunk_size = (page_test_size < page_remaining)? page_test_size : page_remaining;

  eeprom_page_write_count = 0;
  eeprom_cross_mismatch_count = 0;
  eeprom_cross_verify_ok = 0;
  eeprom_cross_test_done = 0;

  for (uint8_t i = 0; i < 16U; i++)
  {
      cross_tx_buffer[i] = (uint8_t)(0xC0U + i);
      cross_rx_buffer[i] = 0;
  }

  eeprom_cross_write_status = EEPROM_WriteBuffer(
      0x0078,
      cross_tx_buffer,
      16
  );

  if (eeprom_cross_write_status == HAL_OK)
  {
      eeprom_cross_read_status = HAL_I2C_Mem_Read(
          &hi2c1,
          EEPROM_DEV_ADDR,
          0x0078,
          I2C_MEMADD_SIZE_16BIT,
          cross_rx_buffer,
          16,
          100
      );

      if (eeprom_cross_read_status == HAL_OK)
      {
          for (uint8_t i = 0; i < 16U; i++)
          {
              if (cross_tx_buffer[i] != cross_rx_buffer[i])
              {
                  eeprom_cross_mismatch_count++;
              }
          }

          if (eeprom_cross_mismatch_count == 0)
          {
              eeprom_cross_verify_ok = 1;
          }
      }
  }

  eeprom_cross_test_done = 1;*/

 /* i2c_status = HAL_I2C_IsDeviceReady(&hi2c1, (0x50U << 1), 3, 100);

  i2c_error_code = HAL_I2C_GetError(&hi2c1);

  if(i2c_status == HAL_OK)
  {
	  eeprom_write_status = HAL_I2C_Mem_Write(&hi2c1, (0x50U << 1), 0x0010, I2C_MEMADD_SIZE_16BIT, &eeprom_tx_data, 1, 100);

	  if(eeprom_write_status == HAL_OK){
		  uint32_t wait_start = HAL_GetTick();

		  eeprom_ready_status = EEPROM_WaitReady(20);

		  eeprom_wait_elapsed_ms = HAL_GetTick() - wait_start;
		  if(eeprom_ready_status == HAL_OK){
			  eeprom_read_status = HAL_I2C_Mem_Read(&hi2c1, (0x50U << 1), 0x0010, I2C_MEMADD_SIZE_16BIT, &eeprom_rx_data, 1, 100);

			  if (eeprom_read_status == HAL_OK && eeprom_rx_data == eeprom_tx_data)
			  {
			        eeprom_verify_ok = 1;
			  }
		  }
	  }
  }

  i2c_test_done = 1; */

  /* Lab 07-9: Modular EEPROM driver test */

  driver_init_status = EEPROM24C256_Init(&hi2c1);

  if (driver_init_status == HAL_OK)
  {
      for (uint8_t i = 0; i < 16U; i++)
      {
          driver_tx_buffer[i] = (uint8_t)(0xD0U + i);
          driver_rx_buffer[i] = 0;
      }

      driver_write_status = EEPROM24C256_Write(
          0x0078,
          driver_tx_buffer,
          16
      );

      if (driver_write_status == HAL_OK)
      {
          driver_read_status = EEPROM24C256_Read(
              0x0078,
              driver_rx_buffer,
              16
          );

          if (driver_read_status == HAL_OK)
          {
              for (uint8_t i = 0; i < 16U; i++)
              {
                  if (driver_tx_buffer[i] != driver_rx_buffer[i])
                  {
                      driver_mismatch_count++;
                  }
              }

              if (driver_mismatch_count == 0U)
              {
                  driver_verify_ok = 1;
              }
          }
      }
  }

  driver_test_done = 1;

  if (driver_init_status == HAL_OK)
  {
      test_zero_length = EEPROM24C256_Write(
          0x0100,
          driver_tx_buffer,
          0
      );

      test_null_pointer = EEPROM24C256_Write(
          0x0100,
          NULL,
          8
      );

      test_out_of_range = EEPROM24C256_Write(
          0x7FFC,
          driver_tx_buffer,
          8
      );

      boundary_test_done = 1;
  }


  /* Lab 07-9E: 128-byte multi-page test */

  if (driver_init_status == HAL_OK)
  {
      long_mismatch_count = 0;
      long_verify_ok = 0;
      long_test_done = 0;

      for (uint16_t i = 0; i < EEPROM_LONG_TEST_SIZE; i++)
      {
          long_tx_buffer[i] = (uint8_t)(0x30U + i);
          long_rx_buffer[i] = 0;
      }

      long_write_status = EEPROM24C256_Write(
          0x0030,
          long_tx_buffer,
          EEPROM_LONG_TEST_SIZE
      );

      if (long_write_status == HAL_OK)
      {
          long_read_status = EEPROM24C256_Read(
              0x0030,
              long_rx_buffer,
              EEPROM_LONG_TEST_SIZE
          );

          if (long_read_status == HAL_OK)
          {
              for (uint16_t i = 0; i < EEPROM_LONG_TEST_SIZE; i++)
              {
                  if (long_tx_buffer[i] != long_rx_buffer[i])
                  {
                      long_mismatch_count++;
                  }
              }

              if (long_mismatch_count == 0)
              {
                  long_verify_ok = 1;
              }
          }
      }

      long_test_done = 1;
  }
  end_write_status = HAL_ERROR;
  end_invalid_status = HAL_OK;
  end_read_status = HAL_ERROR;
  end_rx_data = 0;
  end_verify_ok = 0;
  end_test_done = 0;

  if (driver_init_status == HAL_OK)
  {
      /* A: Write last valid byte */
      end_write_status = EEPROM24C256_Write(
          0x7FFF,
          &end_tx_data,
          1
      );

      /* B: Reject write beyond EEPROM capacity */
      end_invalid_status = EEPROM24C256_Write(
          0x7FFF,
          driver_tx_buffer,
          2
      );

      /* C: Read last valid byte */
      if (end_write_status == HAL_OK)
      {
          end_read_status = EEPROM24C256_Read(
              0x7FFF,
              &end_rx_data,
              1
          );

          if (end_read_status == HAL_OK &&
              end_rx_data == end_tx_data)
          {
              end_verify_ok = 1;
          }
      }

      end_test_done = 1;
  }


  /* Lab 07-9G: Missing I2C Device Test */

  missing_test_done = 0;

  uint32_t start_tick = HAL_GetTick();

  missing_status = HAL_I2C_IsDeviceReady(
      &hi2c1,
      (0x51U << 1),
      3,
      100
  );

  missing_elapsed_ms = HAL_GetTick() - start_tick;
  missing_error = HAL_I2C_GetError(&hi2c1);

  missing_test_done = 1;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
