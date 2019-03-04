//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS Google Drive: UMSATS/Guides and HowTos.../Command and Data Handling (CDH)/Coding Standards
//
// File Description:
//  Template source file for C / C++ projects. Unused sections can be deleted.
//
// History
// 2019-01-13 by Tamkin Rahman
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buzzer.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void SystemClock_Config_b(void);
static void Initialization(void);
void buzz(int seconds);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Enter description for static function here.
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
/**
  * @brief System Clock Configuration
  * @retval None
  */

int main_b(void)
{
	buzz(1);

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void buzz(int seconds)
{
	/* Configure the system clock */
	SystemClock_Config_b();

	/* Initialize all configured peripherals & timer */
	Initialization();

	int count = seconds * SECOND;
	  while (count != 0)
	  {
		  HAL_GPIO_WritePin (GPIOA, GPIO_PIN_4, GPIO_PIN_SET); //sets pin 4 as high
		  HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // sets pin 5 as low
		    //count to 10,502.56 for proper delay of 0.125 ms
		    TIM2->CNT = 0; //Sets timer count to 0
		    TIM2->CR1 |= 1; //Enables Timer
		    while((TIM2->SR & 1) != 1){} //Waits for timer to reach specified value
		    TIM2->CR1 &= ~1; //Disables Timer
		    TIM2->SR &= ~1; //Resets UIF pin
		  	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // sets pin 4 as low
		  	HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, GPIO_PIN_SET); //sets pin 5 as high
		  	TIM2->CNT = 0;
		  	TIM2->CR1 |= 1;
		  	while((TIM2->SR & 1) != 1){}
		  	TIM2->CR1 &= ~1;
		  	TIM2->SR &= ~1;
		  	count -= 1;
	  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void SystemClock_Config_b(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /**Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /**Initializes the CPU, AHB and APB busses clocks 
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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void Initialization(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  	 //set up PA4 as output.
  	 GPIO_InitTypeDef GPIOInit;
     GPIOInit.Pin       = GPIO_PIN_4 | GPIO_PIN_5;
     GPIOInit.Mode      = GPIO_MODE_OUTPUT_PP;

     HAL_GPIO_Init(GPIOA,&GPIOInit);
     
	/* Enables clock for timer */
	__HAL_RCC_TIM2_CLK_ENABLE();

	/* set value to count to */
	TIM2->ARR = COUNTER;
}
