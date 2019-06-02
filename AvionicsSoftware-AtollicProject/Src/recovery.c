//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS Rocketry 2019
//
// Repository:
//  UMSATS/Avionics
//
// File Description:
//  Source file for functions related to recovery ignition circuit.
//
// History
// 2019-05-29 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "recovery.h"
#include "hardwareDefs.h"
#include "stm32f4xx_hal.h"

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

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void recovery_init(){

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;

	//Setup outputs.
	GPIO_InitStruct.Pin       = RECOV_DROGUE_ACTIVATE_PIN|RECOV_DROGUE_ENABLE_PIN|RECOV_MAIN_ACTIVATE_PIN|RECOV_MAIN_ENABLE_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed	  = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(RECOV_DROGUE_ACTIVATE_PORT,&GPIO_InitStruct);

	//Set default state drogue.
	HAL_GPIO_WritePin(RECOV_DROGUE_ACTIVATE_PORT,RECOV_DROGUE_ACTIVATE_PIN,GPIO_PIN_RESET);	//Active high so set to low.
	HAL_GPIO_WritePin(RECOV_DROGUE_ENABLE_PORT,RECOV_DROGUE_ENABLE_PIN,GPIO_PIN_SET);			//Active low so set to high.

	//Set default state main.
	HAL_GPIO_WritePin(RECOV_MAIN_ACTIVATE_PORT,RECOV_MAIN_ACTIVATE_PIN,GPIO_PIN_RESET);	//Active high so set to low.
	HAL_GPIO_WritePin(RECOV_MAIN_ENABLE_PORT,RECOV_MAIN_ENABLE_PIN,GPIO_PIN_SET);			//Active low so set to high.


	//Setup inputs.
	GPIO_InitTypeDef GPIO_InitStruct2;

	GPIO_InitStruct2.Pin = RECOV_DROGUE_OVERCURRENT_PIN|RECOV_DROGUE_CONTINUITY_PIN|RECOV_MAIN_OVERCURRENT_PIN;
	GPIO_InitStruct2.Mode = GPIO_MODE_INPUT;
	HAL_GPIO_Init(RECOV_DROGUE_OVERCURRENT_PORT,&GPIO_InitStruct2);

	GPIO_InitTypeDef GPIO_InitStruct3;

	GPIO_InitStruct3.Pin = RECOV_MAIN_CONTINUITY_PIN;
	GPIO_InitStruct3.Mode= GPIO_MODE_INPUT;
	HAL_GPIO_Init(RECOV_MAIN_CONTINUITY_PORT,&GPIO_InitStruct3);


}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void enable_mosfet(recoverySelect_t recov_event){

	if(recov_event == MAIN){
		//Active low.
		HAL_GPIO_WritePin(RECOV_MAIN_ENABLE_PORT,RECOV_MAIN_ENABLE_PIN,GPIO_PIN_RESET);
	}
	else if(recov_event == DROGUE){

		//Active low.
		HAL_GPIO_WritePin(RECOV_DROGUE_ENABLE_PORT,RECOV_DROGUE_ENABLE_PIN,GPIO_PIN_RESET);
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void activate_mosfet(recoverySelect_t recov_event){

	if(recov_event == MAIN){

		//Write pin low in case there was a fault.
		HAL_GPIO_WritePin(RECOV_MAIN_ACTIVATE_PORT,RECOV_MAIN_ACTIVATE_PIN,GPIO_PIN_RESET);

		//Active high.
		HAL_GPIO_WritePin(RECOV_MAIN_ACTIVATE_PORT,RECOV_MAIN_ACTIVATE_PIN,GPIO_PIN_SET);

		vTaskDelay(EMATCH_ON_TIME);

		//De-activate.
		HAL_GPIO_WritePin(RECOV_MAIN_ACTIVATE_PORT,RECOV_MAIN_ACTIVATE_PIN,GPIO_PIN_RESET);

		//Disable driver.
		HAL_GPIO_WritePin(RECOV_MAIN_ENABLE_PORT,RECOV_MAIN_ENABLE_PIN,GPIO_PIN_SET);
	}
	else if(recov_event == DROGUE){

		//Write pin low in case there was a fault.
		HAL_GPIO_WritePin(RECOV_DROGUE_ACTIVATE_PORT,RECOV_DROGUE_ACTIVATE_PIN,GPIO_PIN_RESET);

		//Active high.
		HAL_GPIO_WritePin(RECOV_DROGUE_ACTIVATE_PORT,RECOV_DROGUE_ACTIVATE_PIN,GPIO_PIN_SET);

		vTaskDelay(pdMS_TO_TICKS(EMATCH_ON_TIME));

		//De-activate.
		HAL_GPIO_WritePin(RECOV_DROGUE_ACTIVATE_PORT,RECOV_DROGUE_ACTIVATE_PIN,GPIO_PIN_RESET);

		//Disable driver.
		HAL_GPIO_WritePin(RECOV_DROGUE_ENABLE_PORT,RECOV_DROGUE_ENABLE_PIN,GPIO_PIN_SET);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------


continuityStatus_t check_continuity(recoverySelect_t recov_event){

	GPIO_PinState result = GPIO_PIN_RESET;//Should the default be to read open or closed circuit???
	continuityStatus_t cont;

	if(recov_event == MAIN){
		result = HAL_GPIO_ReadPin(RECOV_MAIN_CONTINUITY_PORT,RECOV_MAIN_CONTINUITY_PIN);
	}
	else if(recov_event == DROGUE){
		result = HAL_GPIO_ReadPin(RECOV_DROGUE_CONTINUITY_PORT,RECOV_DROGUE_CONTINUITY_PIN);
	}

	if(result == GPIO_PIN_SET){
		cont = SHORT_CIRCUIT;
	}
	else{
		cont = OPEN_CIRCUIT;
	}

	return cont;
}


overcurrentStatus_t check_overcurrent(recoverySelect_t recov_event){

	GPIO_PinState result = GPIO_PIN_RESET;//Should the default be to read open or closed circuit???
	overcurrentStatus_t overcurrent;

	if(recov_event == MAIN){
		result = HAL_GPIO_ReadPin(RECOV_MAIN_OVERCURRENT_PORT,RECOV_MAIN_OVERCURRENT_PIN);
	}
	else if(recov_event == DROGUE){
		result = HAL_GPIO_ReadPin(RECOV_DROGUE_OVERCURRENT_PORT,RECOV_DROGUE_OVERCURRENT_PIN);
	}

	if(result == GPIO_PIN_SET){
		overcurrent = NO_OVERCURRENT;
	}
	else{
		overcurrent = OVERCURRENT;
	}

	return overcurrent;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

