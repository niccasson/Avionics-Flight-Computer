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
	uint32_t flash_address = FLASH_START_ADDRESS;

	uint8_t data_bufferA[DATA_BUFFER_SIZE];	//This stores the data until we have enough to write to flash.

	uint8_t	data_bufferB[DATA_BUFFER_SIZE];

	uint8_t buffer_selection =0;	//0 for buffer A, 1 for buffer B.

	uint8_t buffer_index_curr = 0;		//The current index in the buffer.
	uint8_t is_there_data; 				//Used to keep track of if the current measurement has data.
	Measurement_t measurement;
	uint8_t	measurement_data_index=0;
	uint8_t measurement_length = 0;
	uint32_t prev_time_ticks = 0;	//Holds the previous time to caluculate the change in time.

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	int i;

	for(i=0;i<sizeof(Measurement_t);i++){

		measurement.bytes[i] = 0;
	}
	imu_data_struct  imu_reading;

	while(1){

		//get data from imu queue until empty.

		BaseType_t stat = xQueueReceive(logStruct->IMU_data_queue,&imu_reading,TIME_RESOLUTION);

		if(stat == pdPASS){
			//We read data from imu.

			//Check if the current measurement has data.
			is_there_data = isMeasurementEmpty(&measurement);

			if(!is_there_data){


				uint16_t delta_t = imu_reading.time_ticks-prev_time_ticks;
				measurement.format.header = (ACC_TYPE | GYRO_TYPE) + (delta_t & 0x0FFF);// Make sure time doesn't overwrite type bits.

				measurement_length = ACC_LENGTH + GYRO_LENGTH;

				prev_time_ticks = imu_reading.time_ticks;

				measurement.format.data[0] = imu_reading.data_acc.x >>8;
				measurement.format.data[1] = imu_reading.data_acc.x & 0xFF;

				measurement.format.data[2] = imu_reading.data_acc.y >>8;
				measurement.format.data[3] = imu_reading.data_acc.y & 0xFF;

				measurement.format.data[4] = imu_reading.data_acc.z >>8;
				measurement.format.data[5] = imu_reading.data_acc.z & 0xFF;

				measurement.format.data[6] = imu_reading.data_gyro.x >>8;
				measurement.format.data[7] = imu_reading.data_gyro.x & 0xFF;

				measurement.format.data[8] = imu_reading.data_gyro.y >>8;
				measurement.format.data[9] = imu_reading.data_gyro.y & 0xFF;

				measurement.format.data[10] = imu_reading.data_gyro.z >>8;
				measurement.format.data[11] = imu_reading.data_gyro.z & 0xFF;

				measurement_data_index = ACC_LENGTH+GYRO_LENGTH;
			}


			HAL_GPIO_TogglePin(USR_LED_PORT,USR_LED_PIN);
		}

		//check pres/temp queue
		if(imu_reading.data_acc.x==1){
			HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,data_bufferA[buffer_index_curr]);
		}


		is_there_data = isMeasurementEmpty(&measurement);
		if(buffer_index_curr+measurement_data_index+2 < DATA_BUFFER_SIZE && is_there_data){
			//There is room in the buffer for the full measurement.
			if(buffer_selection == 0){
				memcpy(&data_bufferA[buffer_index_curr],&(measurement.bytes),measurement_length+2);
			}
			else if(buffer_selection == 1){
				memcpy(&data_bufferB[buffer_index_curr],&(measurement.bytes),measurement_length+2);
			}
			buffer_index_curr +=measurement_length+2;

			//reset the measurement.
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.bytes[i] = 0;
			}
			measurement_data_index=0;
			measurement_length=0;

		}else if(is_there_data){

			//Split measurement across flash pages, and write to flash.
			uint8_t bytesInPrevBuffer = BUFFER_SIZE- buffer_index_curr;
			uint8_t bytesLeft = (measurement_data_index+2)-bytesInPrevBuffer;


			//Put as much data as will fit into the almost full buffer.
			if(buffer_selection == 0){
				memcpy(&data_bufferA[buffer_index_curr],&(measurement.bytes),bytesInPrevBuffer);
				buffer_selection =1;
				buffer_index_curr = 0;
			}
			else if(buffer_selection == 1){
				memcpy(&data_bufferB[buffer_index_curr],&(measurement.bytes),bytesInPrevBuffer);
				buffer_selection=0;
				buffer_index_curr = 0;
			}

			//Put the rest of the measurement in the next buffer.
			if(buffer_selection == 0){
				memcpy(&data_bufferA[buffer_index_curr],&(measurement.bytes[bytesInPrevBuffer]),bytesLeft);
				buffer_index_curr = bytesLeft;
			}
			else if(buffer_selection == 1){
				memcpy(&data_bufferB[buffer_index_curr],&(measurement.bytes[bytesInPrevBuffer]),bytesLeft);
				buffer_index_curr = bytesLeft;
			}

			//reset the measurement.
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.bytes[i] = 0;
			}
			measurement_data_index=0;
			measurement_length=0;
			//Flash write buffer not in use! then clear old buffer?

			if(buffer_selection == 0){
				//We just switched to A so transmit B.
				program_page(flash,flash_address,data_bufferB,DATA_BUFFER_SIZE);
				flash_address += DATA_BUFFER_SIZE;
				flash_address = flash_address % FLASH_SIZE_BYTES;
			}
			else if (buffer_selection == 1){

				program_page(flash,flash_address,data_bufferA,DATA_BUFFER_SIZE);
				flash_address += DATA_BUFFER_SIZE;
				flash_address = flash_address % FLASH_SIZE_BYTES;
			}
		}

	};

}
