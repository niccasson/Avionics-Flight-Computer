//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS/Avionics-2019
//
// File Description:
//  Source file for the data logging module.
//
// History
// 2019-04-10 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "dataLogging.h"


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

//Checks if a measurement is empty. returns 0 if it is empty.
uint8_t isMeasurementEmpty(Measurement_t * measurement){

	uint8_t result = 0;
	int i;

	for(i=0;i<sizeof(Measurement_t);i++){

		if(measurement->bytes[i] != 0){
			result ++;
		}
	}
	return result;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void loggingTask(void * params){

	LoggingStruct_t * logStruct = (LoggingStruct_t *)params;
	FlashStruct_t * flash_ptr = logStruct->flash_ptr;

	uint8_t data_buffer[DATA_BUFFER_SIZE];	//This stores the data until we have enough to write to flash.
	uint8_t buffer_index_curr = 0;		//The current index in the buffer.

	Measurement_t measurement;
	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	int i;

	for(i=0;i<sizeof(Measurement_t);i++){

		measurement.bytes[i] = 0;
	}
	imu_data_struct imu_reading;

	while(1){

		imu_reading.data_acc.x=0;
		imu_reading.data_acc.y=0;
		imu_reading.data_acc.z=0;
		imu_reading.data_gyro.x=0;
		imu_reading.data_gyro.y=0;
		imu_reading.data_gyro.z=0;
		imu_reading.time_ticks=0;

		//get data from imu queue until empty.

		BaseType_t stat = xQueueReceive(logStruct->IMU_data_queue,&imu_reading,TIME_RESOLUTION);

		if(stat == pdPASS){
			//We read data from imu.
			buffer_index_curr++;
			HAL_GPIO_TogglePin(USR_LED_PORT,USR_LED_PIN);
		}

		//check pres/temp queue

		//Check if we got any data
		uint8_t is_data = isMeasurementEmpty(&measurement);
		if(is_data){
			data_buffer[buffer_index_curr] = 1;
			//put data into buffer.
		}

	};

}
