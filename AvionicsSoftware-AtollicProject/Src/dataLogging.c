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

		if(measurement->data[i] != 0){
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
	UART_HandleTypeDef * huart = logStruct->uart;

	uint32_t flash_address = FLASH_START_ADDRESS;


	uint8_t data_bufferA[DATA_BUFFER_SIZE];			//This stores the data until we have enough to write to flash.
	uint8_t	data_bufferB[DATA_BUFFER_SIZE]; 		//This stores the data until we have enough to write to flash.
	BufferSelection_t buffer_selection = BUFFER_A;
	uint16_t buffer_index_curr = 0;					//The current index in the buffer.

	uint8_t is_there_data; 				//Used to keep track of if the current measurement has data.
	Measurement_t measurement;
	uint8_t measurement_length = 0;

	uint32_t prev_time_ticks = 0;	//Holds the previous time to calculate the change in time.



	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);

	//Make sure the measurement starts empty.
	int i;
	for(i=0;i<sizeof(Measurement_t);i++){

		measurement.data[i] = 0;
	}


	imu_data_struct  imu_reading;

	bmp_data_struct	bmp_reading;

	prev_time_ticks = xTaskGetTickCount();

	while(1){

		measurement_length=0;

		/* IMU READING*******************************************************************************************************************************/

		//Try and get data from the IMU queue. Block for up to a quarter of the time between the fastest measurement.
		BaseType_t stat = xQueueReceive(logStruct->IMU_data_queue,&imu_reading,TIME_RESOLUTION/4);

		if(stat == pdPASS){
			//We have read data from the IMU.

			//Check if the current measurement has data.
			is_there_data = isMeasurementEmpty(&measurement);

			if(!is_there_data){

				uint16_t delta_t = imu_reading.time_ticks-prev_time_ticks;

				uint16_t header  = (ACC_TYPE | GYRO_TYPE) + (delta_t & 0x0FFF);// Make sure time doesn't overwrite type bits.

				measurement.data[0] = header& 0x00FF;
				measurement.data[1] = header>>8;

				measurement_length = ACC_LENGTH + GYRO_LENGTH;

				prev_time_ticks = imu_reading.time_ticks;

				measurement.data[2] = ((uint16_t)imu_reading.data_acc.x) >>8;
				measurement.data[3] = ((uint16_t)imu_reading.data_acc.x) & 0xFF;

				measurement.data[4] = ((uint16_t)imu_reading.data_acc.y) >>8;
				measurement.data[5] = ((uint16_t)imu_reading.data_acc.y) & 0xFF;

				measurement.data[6] = ((uint16_t)imu_reading.data_acc.z) >>8;
				measurement.data[7] = ((uint16_t)imu_reading.data_acc.z) & 0xFF;

				measurement.data[8] = ((uint16_t)imu_reading.data_gyro.x) >>8;
				measurement.data[9] = ((uint16_t)imu_reading.data_gyro.x) & 0xFF;

				measurement.data[10] = ((uint16_t)imu_reading.data_gyro.y) >>8;
				measurement.data[11] = ((uint16_t)imu_reading.data_gyro.y) & 0xFF;

				measurement.data[12] = ((uint16_t)imu_reading.data_gyro.z) >>8;
				measurement.data[13] = ((uint16_t)imu_reading.data_gyro.z) & 0xFF;


			}


			HAL_GPIO_TogglePin(USR_LED_PORT,USR_LED_PIN);
		}

		/* BMP READING*******************************************************************************************************************************/
		//Try and get data from the BMP queue. Block for up to a quarter of the time between the fastest measurement.
		stat = xQueueReceive(logStruct->PRES_data_queue,&bmp_reading,TIME_RESOLUTION/4);

		if(stat == pdPASS){

			is_there_data = isMeasurementEmpty(&measurement);

			if(is_there_data){
				//We already have a imu reading.

				measurement_length += (PRES_LENGTH + TEMP_LENGTH);

				//Update the header bytes.
				uint16_t header = (measurement.data[1]<<8) + measurement.data[0];
				header |= PRES_TYPE | TEMP_TYPE;
				measurement.data[0] =header&0xFF;
				measurement.data[1] =(header)>>8;


				measurement.data[14]= (((uint32_t)bmp_reading.data.pressure) >>16) &0xFF ;	//MSB
				measurement.data[15]= (((uint32_t)bmp_reading.data.pressure) >> 8) & 0xFF;	//LSB
				measurement.data[16]= ((uint32_t)bmp_reading.data.pressure) & 0xFF;		//XLSB

				measurement.data[17]= (((uint32_t)bmp_reading.data.temperature) >>16) & 0xFF;	//MSB
				measurement.data[18]= ((uint32_t)bmp_reading.data.temperature >> 8) & 0xFF;	//LSB
				measurement.data[19]= (uint32_t)bmp_reading.data.temperature & 0xFF; //XLSB

			}


		}


		/* Fill Buffer and/or write to flash*********************************************************************************************************/
		is_there_data = isMeasurementEmpty(&measurement);

		if(((buffer_index_curr+measurement_length +2) < DATA_BUFFER_SIZE) && (is_there_data)){

			//There is room in the current buffer for the full measurement.

			if(buffer_selection == BUFFER_A){

				memcpy(&data_bufferA[buffer_index_curr],&(measurement.data),measurement_length+2);
				//transmit_bytes(huart,&data_bufferA[buffer_index_curr],measurement_length+2);
			}
			else if(buffer_selection == BUFFER_B){

				memcpy(&data_bufferB[buffer_index_curr],&(measurement.data),measurement_length+2);
				//transmit_bytes(huart,&data_bufferB[buffer_index_curr],measurement_length+2);
			}

			buffer_index_curr += (measurement_length+2);

			//Reset the measurement.
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.data[i] = 0;
			}



		}else if(is_there_data){

			//Split measurement across the buffers, and write to flash.
			uint8_t bytesInPrevBuffer = DATA_BUFFER_SIZE- buffer_index_curr;
			uint8_t bytesLeft = (measurement_length+2)-bytesInPrevBuffer;

			if((((measurement.data[1]<<8)+measurement.data[0])&0x0FFF)>300){

				while(1);
			}

			if((bytesLeft+bytesInPrevBuffer)!=20 && ((bytesLeft+bytesInPrevBuffer)!=14)){

				while(1){}
			}

			//Put as much data as will fit into the almost full buffer.
			if(buffer_selection == BUFFER_A){

				memcpy(&data_bufferA[buffer_index_curr],&(measurement.data),bytesInPrevBuffer);
				buffer_selection = BUFFER_B;
				buffer_index_curr = 0;

			}
			else if(buffer_selection == BUFFER_B){

				memcpy(&data_bufferB[buffer_index_curr],&(measurement.data),bytesInPrevBuffer);
				buffer_index_curr = BUFFER_A;
				buffer_selection=0;

			}

			//Put the rest of the measurement in the next buffer.
			if(buffer_selection == BUFFER_A){

				memcpy(&data_bufferA[buffer_index_curr],&(measurement.data[bytesInPrevBuffer]),bytesLeft);
				buffer_index_curr = bytesLeft;

			}
			else if(buffer_selection == BUFFER_B){

				memcpy(&data_bufferB[buffer_index_curr],&(measurement.data[bytesInPrevBuffer]),bytesLeft);
				buffer_index_curr = bytesLeft;

			}

			//reset the measurement.
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.data[i] = 0;
			}

			measurement_length=0;

			//Flash write buffer not in use! then clear old buffer?

			if(buffer_selection == 0){
				//We just switched to A so transmit B.

				FlashStatus_t stat_f = program_page(flash_ptr,flash_address,data_bufferB,DATA_BUFFER_SIZE);
			  	  while(IS_DEVICE_BUSY(stat_f)){
			  		  stat_f = get_status_reg(flash_ptr);
			  		 vTaskDelay(1);
			  	  }
				//transmit_bytes(huart,data_bufferB,256);

				flash_address += DATA_BUFFER_SIZE;
				if(flash_address>=FLASH_SIZE_BYTES){
					while(1);
				}
			}
			else if (buffer_selection == 1){
				//We just switched to B so transmit A

				FlashStatus_t stat_f2 = program_page(flash_ptr,flash_address,data_bufferA,DATA_BUFFER_SIZE);
			  	  while(IS_DEVICE_BUSY(stat_f2)){
			  		  stat_f2 = get_status_reg(flash_ptr);
			  		 vTaskDelay(1);
			  	  }
				//transmit_bytes(huart,data_bufferA,256);

				flash_address += DATA_BUFFER_SIZE;

				if(flash_address>=FLASH_SIZE_BYTES){
					while(1);
				}
			}


		}

		for(i=0;i<sizeof(Measurement_t);i++){

			measurement.data[i] = 0;
		}

		measurement_length=0;
	};

}
