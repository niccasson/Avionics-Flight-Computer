//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS/Avionics-2019
//
// File Description:
//  Source file for the start up task.
//
// History
// 2019-04-19 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "startupTask.h"

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
// Description:
//  This task will check if the memory is empty and erase it if it is not.
//
//	Right now the function assumes that if the first page of memory is empty,
//	then the whole memory is empty.
//
// Returns:
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void eraseFlash(startParams * params);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void eraseFlash(startParams * params){

	  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);

	  FlashStatus_t stat;

	  FlashStruct_t * flash = params->flash_ptr;
	  UART_HandleTypeDef * huart = params->huart_ptr;
	  configData_t * config = params->flightCompConfig;

	  uint8_t dataRX[256];
	  transmit_line(huart,"Checking flash memory...");
	 // Read the first page of memory. If its empty, assume the whole memory is empty.
	  stat = read_page(flash,config->values.start_data_address,dataRX,256);

	  uint16_t good= 0xFFFF;

	  int i;
	  for(i=0;i<256;i++){

		  if(dataRX[i] != 0xFF){
			  good --;
		  }
	  }

	  if(good == 0xFFFF){
		  		  transmit_line(huart,"flash empty.");
		  		  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);

	  }else{


		  transmit_line(huart,"flash not empty.");
		  //Erase the whole flash. This could take up to 2 minutes.
	  	  //stat = erase_device(flash);
		  uint32_t address = FLASH_START_ADDRESS;

		  while(address < config->values.end_data_address){

			  if(address>FLASH_PARAM_END_ADDRESS){
			  stat = erase_sector(flash,address);
			  address += FLASH_SECTOR_SIZE;
			  }
			  else{
				  stat = erase_param_sector(flash,address);
				  address += FLASH_PARAM_SECTOR_SIZE;
			  }
			  //Wait for erase to finish
			  while(IS_DEVICE_BUSY(stat)){

				  stat = get_status_reg(flash);

				  vTaskDelay(pdMS_TO_TICKS(1));
			  }


		  }

		  read_page(flash,FLASH_START_ADDRESS,dataRX,256);
		  uint16_t empty = 0xFFFF;

		  for(i=0;i<256;i++){

			  if(dataRX[i] != 0xFF){
				 empty --;
			  }
		  }

		  if(empty == 0xFFFF){

			  transmit_line(huart,"Flash Erased Success!");
			  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
			  HAL_Delay(1000);
			  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
		  }

	  }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_starter(void * pvParams){

	  startParams * sp = (startParams *) pvParams;

	  TaskHandle_t dataLoggingTask_h = sp->loggingTask_h;
	  TaskHandle_t bmpTask_h = sp->bmpTask_h;
	  TaskHandle_t imuTask_h = sp->imuTask_h;
	  TaskHandle_t xtractTask_h = sp->xtractTask_h;
	  FlashStruct_t * flash = sp->flash_ptr;
	  UART_HandleTypeDef * huart = sp->huart_ptr;
	  configData_t * config = sp->flightCompConfig;

	  vTaskSuspend(xtractTask_h);
	  vTaskSuspend(imuTask_h);
	  vTaskSuspend(bmpTask_h);
	  vTaskSuspend(dataLoggingTask_h);

	  while(1){

		  if((!HAL_GPIO_ReadPin(USR_PB_PORT,USR_PB_PIN))||(config->values.state==STATE_XTRACT)){

			  HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
			  config->values.state = STATE_XTRACT;
			  vTaskResume(xtractTask_h);
			  vTaskSuspend(NULL);

		  }else{

			  if(!IS_IN_FLIGHT(config->values.flags)){

				  uint32_t count = 0;
				  uint32_t delay = 2000;
				  uint8_t state = 0;

				  uint32_t time = config->values.initial_time_to_wait;

				  while(count<time){

					  vTaskDelay(pdMS_TO_TICKS(delay));
					  HAL_GPIO_TogglePin(USR_LED_PORT,USR_LED_PIN);
					  count += delay;

					  if(count > (time/2) && state ==0){

						  delay = 1000;
						  state = 1;
					  }
					  else if( count > (3*(time/4)) && state == 1){

						  delay = 500;
						  state = 2;
					  }

				  }

			  	  eraseFlash(sp);
			  }

			  vTaskResume(dataLoggingTask_h);
			  vTaskResume(imuTask_h);
			  vTaskResume(bmpTask_h);
			  vTaskSuspend(NULL);
		  }
	  }

}
