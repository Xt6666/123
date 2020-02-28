#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "max30102.h"
#include "lcd.h"
#include <stdio.h>
#include "stm32l4xx_hal.h"
//#include "algorithm.h"
void SystemClock_Config(void);
float temperature, altitude;
int32_t pressure;
static const char *I2C_BUS="/dev/i2c-1";
static const int I2C_ADDR = 0x60;
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
	LCD_Init();
	
  BMP280_Init(&hi2c1, BMP280_TEMPERATURE_16BIT, BMP280_STANDARD, BMP280_FORCEDMODE);
  while (1)
  {
		BMP280_ReadTemperatureAndPressure(&temperature, &pressure);
		altitude = BMP280_ReadAltitude(103125);
		
		printf("altitude = %.2f\r\n",altitude);
		printf("temperature = %.2f\r\n",temperature);
		printf("pressure = %d\r\n",pressure);		
		printf("\r\n");
		HAL_Delay(1000);
  }
//	  uint16_t vis_data;
//    uint16_t ir_data;
//    uint16_t ps1_data;
//    uint16_t ps2_data;
//    uint16_t ps3_data;
//    uint16_t uv_data;

//			if (si1145_init(I2C_BUS, I2C_ADDR, SI1145_CONFIG_BIT_ALS | 
//																					 SI1145_CONFIG_BIT_UV | 
//																					 SI1145_CONFIG_BIT_MEAS_RATE_SLOW |
//																					 SI1145_CONFIG_BIT_INDOORS) != SI1145_OK)
//				{
//						return 1;
//				}

//				if (si1145_measurement_auto(SI1145_MEASUREMENT_ALS) != SI1145_OK)
//				{
//						return 1;
//				}

//				int i = 0;
//				while (i < 5)
//				{
//						HAL_Delay(2);
//						if (si1145_get_vis_data(&vis_data) != SI1145_OK)
//						{
//								return 1; 
//						}
//						printf("VIS_DATA: 0x%x\n", vis_data);
//						i++;
//				}

//				if (si1145_measurement_pause(SI1145_MEASUREMENT_ALS) != SI1145_OK)
//				{
//						return 1;
//				}

//				if (si1145_get_vis_data(&vis_data) != SI1145_OK ||
//						si1145_get_ir_data(&ir_data) != SI1145_OK ||
//						si1145_get_ps_data(&ps1_data, &ps2_data, &ps3_data) != SI1145_OK ||
//						si1145_get_uv_data(&uv_data) != SI1145_OK)
//				{
//						return 1; 
//				}
//				printf("VIS_DATA: 0x%x\n", vis_data);
//				printf("IR_DATA: 0x%x\n", ir_data);
//				printf("PS1_DATA: 0x%x\n", ps1_data);
//				printf("PS2_DATA: 0x%x\n", ps2_data);
//				printf("PS3_DATA: 0x%x\n", ps3_data);
//				printf("UV_DATA: 0x%x\n", uv_data);
//				
//				si1145_close();
	}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable MSI Auto calibration 
  */
  HAL_RCCEx_EnableMSIPLLMode();
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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
