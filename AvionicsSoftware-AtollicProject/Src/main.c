/*	Avionics Software
 *
 *	File Description:
 *		Main file for the avionics software. The tasks are initialized here and the scheduler is started.
 *		Right now the default task is setup to blink the on-board LED.
 *
 *
 *	History:
 *	- 2019-01-22
 *		Created by Joseph Howarth
 *
 *
 */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

osThreadId defaultTaskHandle;
UART_HandleTypeDef huart2_ptr; //global var to be passed to vTask_xtract

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartDefaultTask(void const * argument);
void testFlash();
void testpress();
void testIMU();

int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init(); //GPIO MUST be firstly initialized

  MX_HAL_UART6_Init(&huart6_ptr); //UART uses GPIO pin 2 & 3

  //testIMU();
  //testpress();
  //testFlash();


  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  if(xTaskCreate(	vTask_xtract, 	 /* Pointer to the function that implements the task */
    		  	"xtract uart cli", /* Text name for the task. This is only to facilitate debugging */
    		  	 1000,		 /* Stack depth - small microcontrollers will use much less stack than this */
				 (void*) &huart2_ptr,	/* pointer to the huart object */
				 1,			 /* This task will run at priorirt 1. */
				 NULL		 /* This example does not use the task handle. */
      	  	  ) == -1){
	  Error_Handler();
  }
 

  /* Start scheduler -- comment to not use FreeRTOS */
 // osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1)
  {

  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
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


void testFlash(){

	  HAL_Delay(1000);
	  FlashStruct_t flash;
	  flash.hspi = flash_spi;

	  FlashStatus_t stat = initialize_flash(&flash);

	  if(stat == FLASH_OK){

		  transmit_line(&huart6_ptr,"SPI INIT good!");
		  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	  }
	  else{

		  transmit_line(&huart6_ptr,"SPI INIT FAILED.");
	  }

//	  uint8_t dataTX[1] = {0xAA};
//	  uint8_t dataRX[1] = {0x00};
//
//	  program_page(&flash,0x00000000,dataTX,1);
//
//	  HAL_Delay(10);
//	  read_page(&flash,0x00000000,dataRX,1);
//
//	  if(dataRX[0] == dataTX[0]){
//
//		  transmit_line(&huart6_ptr,"SPI read successful.");
//		  //HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
//	  }
//
//	  erase_sector(&flash,0x00000000);
//	  volatile uint8_t stat_reg = 0xFF;
//	  while(IS_DEVICE_BUSY(stat_reg)){
//		  stat_reg = get_Status_reg(&flash);
//
//		  HAL_Delay(1);
//	  }
//
//
//	  read_page(&flash,0x00000000,dataRX,1);
//	  if(dataRX[0] == 0xFF){
//
//		  transmit_line(&huart6_ptr,"Flash Erased Successfully.");
//		  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
//	  }
}

void testpress(){
SPI_HandleTypeDef spi2;

spi2_init(&spi2);
HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
uint8_t id= 0x50;

uint8_t command[] = {0x80};
uint8_t id_read[] = {0x00,0x00};

spi_receive(spi2,command,1,id_read,2,10);

if(id_read[1] == id){

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
}

}

void testIMU(){
	SPI_HandleTypeDef spi3;

	spi3_init(&spi3);
	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	uint8_t id= 0x1E;

	uint8_t command[] = {0x80};
	uint8_t id_read[] = {0x00,0x00,0x00,0x00};
	uint8_t id_dummy[] = {0x00,0x00};
	spi_receive(spi3,command,1,id_dummy,2,10);
	spi_receive(spi3,command,1,id_read,2,10);

	if(id_read[1] == id){

		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	}

}


static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct;

  //set up PA5 as output.
  GPIO_InitStruct.Pin       = GPIO_PIN_5;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
}

void StartDefaultTask(void const * argument)
{


  for(;;)
  {

	HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
    vTaskDelay(pdMS_TO_TICKS(1000)); //Delay for 1 second.
  }
  /* USER CODE END 5 */ 
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */

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

  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
