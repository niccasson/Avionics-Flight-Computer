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
#include "configuration.h"
#include "cmsis_os.h"
#include "flash.h"
#include "sensorAG.h"
#include "pressure_sensor_bmp3.h"
#include "dataLogging.h"
#include "xtract.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart_io.h"
#include "startupTask.h"
#include "main.h"
#include "buzzer.h"
#include "timer.h"
#include "recovery.h"

osThreadId defaultTaskHandle;
UART_HandleTypeDef huart6_ptr; //global var to be passed to vTask_xtract

SPI_HandleTypeDef flash_spi;
FlashStruct_t flash;
ImuTaskStruct imuTaskParams ;
LoggingStruct_t logParams;
PressureTaskParams bmp388Params;
xtractParams xtractParameters;
configData_t flightCompConfig;


startParams tasks;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void StartDefaultTask(void const * argument);
void testFlash(FlashStruct_t * flash);
char testpress();
char testIMU();
void MX_GPIO_Init();
void vTask_starter(void * pvParams);

int main(void)


{

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init(); //GPIO MUST be firstly initialized



	MX_HAL_UART6_Init(&huart6_ptr); //UART uses GPIO pin 2 & 3
	transmit_line(&huart6_ptr,"UMSATS ROCKETRY FLIGHT COMPUTER");

	buzzerInit();
	//buzz(500);

	QueueHandle_t imuQueue_h = xQueueCreate(10,sizeof(imu_data_struct));
	if(imuQueue_h == NULL){
	  while(1);
	}

	QueueHandle_t bmpQueue_h = xQueueCreate(10,sizeof(bmp_data_struct));
	if(bmpQueue_h == NULL){
	  while(1);
	}

	//For debugging in Atollic.
	//  vQueueAddToRegistry(imuQueue_h,"imu");
	//  vQueueAddToRegistry(bmpQueue_h,"bmp");

	flash.hspi = flash_spi;

	FlashStatus_t flash_stat =initialize_flash(&flash);
	if(flash_stat != FLASH_OK){
	  while(1);
	}
	transmit_line(&huart6_ptr,"Flash ID read successful\n");
	//Initialize and get the flight computer parameters.

	flightCompConfig.values.flash = &flash;

	read_config(&flightCompConfig);

	char lines[50];
	sprintf(lines,"ID :%d \n",flightCompConfig.values.id);
	transmit_line(&huart6_ptr,lines);

	if(flightCompConfig.values.id != ID){
		transmit_line(&huart6_ptr,"No config found in flash, reseting to default.\n");
		init_config(&flightCompConfig);
		write_config(&flightCompConfig);
	}

	read_config(&flightCompConfig);

	if(IS_POST_MAIN(flightCompConfig.values.flags)){

		flightCompConfig.values.state = STATE_IN_FLIGHT_POST_MAIN;
	}
	else if(IS_POST_DROGUE(flightCompConfig.values.flags)){

		flightCompConfig.values.state = STATE_IN_FLIGHT_POST_APOGEE;
	}
	else if(IS_PRE_DROGUE(flightCompConfig.values.flags)){

		flightCompConfig.values.state = STATE_IN_FLIGHT_PRE_APOGEE;
	}

	uint32_t end_Address = scan_flash(&flash);
	sprintf(lines,"end address :%ld \n",end_Address);
	transmit_line(&huart6_ptr,lines);
	flightCompConfig.values.end_data_address = end_Address;

	recovery_init();
	transmit_line(&huart6_ptr,"Recovery GPIO pins setup.");



	logParams.flash_ptr = &flash;
	logParams.IMU_data_queue = imuQueue_h;
	logParams.PRES_data_queue= bmpQueue_h;
	logParams.uart = &huart6_ptr;
	logParams.flightCompConfig = &flightCompConfig;

	bmp388Params.huart = &huart6_ptr;
	bmp388Params.bmp388_queue =bmpQueue_h;
	bmp388Params.flightCompConfig = &flightCompConfig;

	imuTaskParams.huart = &huart6_ptr;
	imuTaskParams.imu_queue = imuQueue_h;
	imuTaskParams.flightCompConfig = &flightCompConfig;

	//xtractParams xtractParameters;
	xtractParameters.flash = &flash;
	xtractParameters.huart = &huart6_ptr;
	xtractParameters.flightCompConfig = &flightCompConfig;

	tasks.loggingTask_h = NULL;
	tasks.bmpTask_h = NULL;
	tasks.imuTask_h = NULL;
	tasks.xtractTask_h = NULL;
	tasks.timerTask_h = NULL;
	xtractParameters.startupTaskHandle = NULL;

	logParams.timerTask_h = &tasks.timerTask_h;
	tasks.flash_ptr = &flash;
	tasks.huart_ptr = &huart6_ptr;
	tasks.flightCompConfig = &flightCompConfig;

	if(!IS_IN_FLIGHT(flightCompConfig.values.flags)){


	flightCompConfig.values.state = STATE_LAUNCHPAD; // CHANGE TO STATE_LAUNCHPAD !!!!!
	//init_bmp(&flightCompConfig);
	char imu_good = testIMU();
	char bmp_good = testpress();

	if(imu_good == 1 && bmp_good==1){
		HAL_Delay(1000);

		buzz(500); //CHANGE TO 2 SECONDS !!!!!
	}
	else{

		int i;
		for(i=0;i<20;i++){
			buzz(500);
			HAL_Delay(500);
			flightCompConfig.values.state = STATE_XTRACT;
		}
	}

	}
	// testFlash(&flash);

	//Timer_GPIO_Init(); //GPIO MUST be firstly initialized
	// MX_HAL_UART2_Init(&huart2_ptr); //UART uses GPIO pin 2 & 3


	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);


	if(xTaskCreate(	vTask_timer, 	 /* Pointer to the function that implements the task */
	      		  	"timer", /* Text name for the task. This is only to facilitate debugging */
	      		  	 1000,		 /* Stack depth - small microcontrollers will use much less stack than this */
	  				 NULL,	/* pointer to the huart object */
	  				 1,			 /* This task will run at priorirt 2. */
	  				 &tasks.timerTask_h		 /* This example does not use the task handle. */
	        	  	  ) == -1){
	  	  Error_Handler();
	    }

	if( xTaskCreate(	vTask_sensorAG, 	 /* Pointer to the function that implements the task */
			"acc and gyro sensor", /* Text name for the task. This is only to facilitate debugging */
			 1000,		 /* Stack depth - small microcontrollers will use much less stack than this */
			 (void*) &imuTaskParams,	/* pointer to the huart object */
			 2,			 /* This task will run at priorirt 2. */
			 &tasks.imuTask_h		 /* This example does not use the task handle. */
			  ) !=1){

		Error_Handler();
	}


	if(xTaskCreate(	loggingTask, 	 /* Pointer to the function that implements the task */
			"Logging task", /* Text name for the task. This is only to facilitate debugging */
			 10000,		 /* Stack depth - small microcontrollers will use much less stack than this */
			 (void*) &logParams,	/* pointer to the huart object */
			 2,			 /* This task will run at priorirt 2. */
			 &tasks.loggingTask_h	 /* This example does not use the task handle. */
			  ) != 1){
		Error_Handler();
	}
	if(xTaskCreate(	vTask_xtract, 	 /* Pointer to the function that implements the task */
		"xtract uart cli", /* Text name for the task. This is only to facilitate debugging */
		 1000,		 /* Stack depth - small microcontrollers will use much less stack than this */
		 (void*) &xtractParameters,	/* pointer to the huart object */
		 1,			 /* This task will run at priorirt 1. */
		 &tasks.xtractTask_h		 /* This example does not use the task handle. */
	  ) == -1){
		Error_Handler();
	}



	if(xTaskCreate(	vTask_pressure_sensor_bmp3, 	 /* Pointer to the function that implements the task */
		"bmp388 pressure sensor", /* Text name for the task. This is only to facilitate debugging */
		 1000,		 /* Stack depth - small microcontrollers will use much less stack than this */
		 (void*) &bmp388Params,	/* function arguments */
		 2,			 /* This task will run at priority 1. */
		 &tasks.bmpTask_h		 /* This example does not use the task handle. */
		  ) != 1){
		Error_Handler();
	}

	if(xTaskCreate(	vTask_starter, 	 /* Pointer to the function that implements the task */
		"starter task", /* Text name for the task. This is only to facilitate debugging */
		 1000,		 /* Stack depth - small microcontrollers will use much less stack than this */
		 (void*)&tasks,	/* function arguments */
		 1,			 /* This task will run at priority 1. */
		 &xtractParameters.startupTaskHandle		 /* This example does not use the task handle. */
		  ) == -1){
		Error_Handler();
	}

	//Start with all tasks suspended except starter task.
	vTaskSuspend(tasks.xtractTask_h);
	vTaskSuspend(tasks.imuTask_h);
	vTaskSuspend(tasks.bmpTask_h);
	vTaskSuspend(tasks.loggingTask_h);
	vTaskSuspend(tasks.timerTask_h);
	/* Start scheduler -- comment to not use FreeRTOS */







	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	while (1)
	{

		if(HAL_GPIO_ReadPin(USR_PB_PORT,USR_PB_PIN)){

			HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
		}
		else{

			HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
		}


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






char testpress(){
SPI_HandleTypeDef spi2;
char result = 0;
spi2_init(&spi2);
HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
uint8_t id= 0x50;

uint8_t command[] = {0x80};
uint8_t id_read[] = {0x00,0x00};

spi_receive(spi2,command,1,id_read,2,10);

if(id_read[1] == id){

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	result = 1;
}
return result;
}

char testIMU(){

//    GPIO_InitTypeDef GPIO_InitStruct3 = {0};
//    GPIO_InitStruct3.Pin = SPI3_CS2_PIN;
//    GPIO_InitStruct3.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct3.Pull = GPIO_NOPULL;
//    GPIO_InitStruct3.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStruct3.Alternate = 0;
//
//    HAL_GPIO_Init(SPI3_CS2_PORT,&GPIO_InitStruct3);

	char result = 0;
	char res = 0;
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
		res += 1;
	}

	spi_receive(spi3,command,1,id_read,2,11);

	if(id_read[0] == 0x0F){

		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
		res += 1;
	}

	if(res == 2){
		result = 1;
	}
	return result;
}





void MX_GPIO_Init(void)
{
		  /* GPIO Ports Clock Enable */
		  __HAL_RCC_GPIOB_CLK_ENABLE();

		  GPIO_InitTypeDef GPIO_InitStruct;

		  //set up PA5 as output.
		  GPIO_InitStruct.Pin       = USR_LED_PIN;
		  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
		  HAL_GPIO_Init(USR_LED_PORT,&GPIO_InitStruct);

		  GPIO_InitTypeDef GPIO_InitStruct2;

		  GPIO_InitStruct2.Pin = USR_PB_PIN;
		  GPIO_InitStruct2.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct2.Pull = GPIO_PULLUP;
		  HAL_GPIO_Init(USR_PB_PORT,&GPIO_InitStruct2);

}


void StartDefaultTask(void const * argument)
{
  for(;;)
  {

	//HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);
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
while(1);
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
