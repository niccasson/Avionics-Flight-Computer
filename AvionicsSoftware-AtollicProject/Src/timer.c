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
// 2019-03-13 by Benjamin Zacharias
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------


#include "timer.h"
#include "cmsis_os.h"
#include "buzzer.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static UART_HandleTypeDef* uart;
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
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Enter description for static function here.
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void timerOn(void); //turns the timer on
void timerOff(void); //turns the timer off
int getTimer(void); //gets the current value of the timer
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void Timer_GPIO_Init(void){

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct;

  //set up OUTPUT1 as output.
  GPIO_InitStruct.Pin       = OUTPUT1_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(OUTPUT1_PORT,&GPIO_InitStruct);

  //set up OUTPUT2 as output
  //TODO add this back in when OUTPUT2 is available
  //GPIO_InitStruct.Pin       = OUTPUT2_PIN;
  //GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  //HAL_GPIO_Init(OUTPUT2_PORT,&GPIO_InitStruct);


  //set up the push button as input
  GPIO_InitStruct.Pin		= INPUT_PIN;
  GPIO_InitStruct.Mode 		= GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull		= GPIO_PULLDOWN;
  HAL_GPIO_Init(INPUT_PORT, &GPIO_InitStruct);
}

void vTask_timer(void *param){

	//taskENABLE_INTERRUPTS();
	configData_t * configParams = (configData_t *)param;

	//Set both of the outputs to 0 for setup
	HAL_GPIO_WritePin(OUTPUT1_PORT,OUTPUT1_PIN, GPIO_PIN_RESET); //turn Output 1 off
	HAL_GPIO_WritePin(OUTPUT2_PORT,OUTPUT2_PIN, GPIO_PIN_RESET); //turn Output 2 off
	TickType_t prevTime;
	prevTime = xTaskGetTickCount();
	/* As per most FreeRTOS tasks, this task is implemented in an infinite loop. */


				//TODO check how long outputs should be on for
				//TODO check in init that outputs are off before connecting to e-matches

				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(TIME_INTERVAL1)); //wait for the first time interval
				recoverySelect_t event = DROGUE;
				enable_mosfet(event);
				activate_mosfet(event);

				configParams->values.state = STATE_IN_FLIGHT_POST_APOGEE;

				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));

				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(TIME_INTERVAL2)); //wait for the first time interval
				event = MAIN;
				enable_mosfet(event);
				activate_mosfet(event);
				configParams->values.state = STATE_IN_FLIGHT_POST_MAIN;
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));
				buzz(250);
				vTaskDelayUntil(&prevTime,pdMS_TO_TICKS(250));

				vTaskDelete(NULL);

//				vTaskDelay(pdMS_TO_TICKS(TIME_INTERVAL2)); //keep Output on for the designated time
//				HAL_GPIO_WritePin(OUTPUT1_PORT,OUTPUT1_PIN, GPIO_PIN_RESET); //turn Output 1 off
//
//
//				vTaskDelay(pdMS_TO_TICKS(TIME_INTERVAL3)); //wait for the second time interval
//				HAL_GPIO_WritePin(OUTPUT2_PORT,OUTPUT2_PIN, GPIO_PIN_SET); //turn Output 2 on
//				vTaskDelay(pdMS_TO_TICKS(TIME_INTERVAL4)); //keep Output 2 on for the desired time
//				HAL_GPIO_WritePin(OUTPUT2_PORT,OUTPUT2_PIN, GPIO_PIN_RESET); //turn Output 2 off


}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
