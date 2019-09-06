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
	configData_t * configParams = logStruct->flightCompConfig;
	TaskHandle_t *timerTask_h = logStruct->timerTask_h;
	uint32_t flash_address = FLASH_START_ADDRESS;

	if(IS_IN_FLIGHT(configParams->values.flags)){

		flash_address = configParams->values.end_data_address;
	}

	//If start and end are equal there is no other flight data, otherwise start recording after already saved data.
//	if(configParams->values.start_data_address == configParams->values.end_data_address){
//
//		flash_address = configParams->values.start_data_address;
//	}else{
//		flash_address = configParams->values.end_data_address;
//	}

//	int32_t velocity_a = 0;
//	int32_t velocity_p = 0;
//	int32_t acc_x_filtered;
//	int32_t acc_y_filtered;
	int32_t acc_z_filtered;
	uint8_t acc_z_filter_index=0;
	float altFiltered = 0;
	uint8_t alt_filter_count=0;

	uint8_t running = 1;
	uint8_t data_bufferA[DATA_BUFFER_SIZE];			//This stores the data until we have enough to write to flash.
	uint8_t	data_bufferB[DATA_BUFFER_SIZE]; 		//This stores the data until we have enough to write to flash.
	uint16_t ring_buff_size  = DATA_BUFFER_SIZE*25;
	uint8_t launchpadBuffer[ring_buff_size];
	BufferSelection_t buffer_selection = BUFFER_A;
	uint16_t buffer_index_curr = 0;					//The current index in the buffer.


	uint8_t is_there_data; 				//Used to keep track of if the current measurement has data.
	Measurement_t measurement;
	uint8_t measurement_length = 0;

	uint32_t prev_time_ticks = 0;	//Holds the previous time to calculate the change in time.

	char buf[120];

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);

	//Make sure the measurement starts empty.
	int i;
	for(i=0;i<sizeof(Measurement_t);i++){

		measurement.data[i] = 0;
	}


	imu_data_struct  imu_reading;

	bmp_data_struct	bmp_reading;

	prev_time_ticks = xTaskGetTickCount();


	alt_value altitude;
	alt_value alt_prev;
	alt_prev.float_val = 0;
	uint8_t alt_count = 0;
	uint8_t alt_main_count = 0;
	uint16_t apogee_holdout_count = 0;
	uint8_t landed_counter = 0;

	//buzz(250);
	if(!IS_IN_FLIGHT(configParams->values.flags)){
	recoverySelect_t event_d = DROGUE;
	continuityStatus_t cont_d = check_continuity(event_d);

	recoverySelect_t event_m = MAIN;
	continuityStatus_t cont_m = check_continuity(event_m);

	while(cont_m == OPEN_CIRCUIT || cont_d == OPEN_CIRCUIT){

		cont_m = check_continuity(event_m);
		cont_d = check_continuity(event_d);
	}
	configParams->values.state = STATE_LAUNCHPAD_ARMED;
	write_config(configParams);}

	buzz(250); // CHANGE TO 2 SECONDS!!!!!!!
	while(1){

		measurement_length=0;

		/* IMU READING*******************************************************************************************************************************/

		//Try and get data from the IMU queue. Block for up to a quarter of the time between the fastest measurement.
		BaseType_t stat = xQueueReceive(logStruct->IMU_data_queue,&imu_reading,configParams->values.data_rate/4);

		if(stat == pdPASS){
			//We have read data from the IMU.

			//Check if the current measurement has data.
			is_there_data = isMeasurementEmpty(&measurement);

			if(!is_there_data){

				uint16_t delta_t = imu_reading.time_ticks-prev_time_ticks;

				uint32_t header  = (ACC_TYPE | GYRO_TYPE) + (delta_t & 0x0FFF);// Make sure time doesn't overwrite type and event bits.

				measurement.data[0] = (header >> 16) & 0xFF;
				measurement.data[1] = (header >> 8) & 0xFF;
				measurement.data[2] = (header) & 0xFF;

				measurement_length = ACC_LENGTH + GYRO_LENGTH;

				prev_time_ticks = imu_reading.time_ticks;

				measurement.data[3] = ((uint16_t)imu_reading.data_acc.x) >>8;
				measurement.data[4] = ((uint16_t)imu_reading.data_acc.x) & 0xFF;

				measurement.data[5] = ((uint16_t)imu_reading.data_acc.y) >>8;
				measurement.data[6] = ((uint16_t)imu_reading.data_acc.y) & 0xFF;

				measurement.data[7] = ((uint16_t)imu_reading.data_acc.z) >>8;
				measurement.data[8] = ((uint16_t)imu_reading.data_acc.z) & 0xFF;

				measurement.data[9] = ((uint16_t)imu_reading.data_gyro.x) >>8;
				measurement.data[10] = ((uint16_t)imu_reading.data_gyro.x) & 0xFF;

				measurement.data[11] = ((uint16_t)imu_reading.data_gyro.y) >>8;
				measurement.data[12] = ((uint16_t)imu_reading.data_gyro.y) & 0xFF;

				measurement.data[13] = ((uint16_t)imu_reading.data_gyro.z) >>8;
				measurement.data[14] = ((uint16_t)imu_reading.data_gyro.z) & 0xFF;

//				int32_t acc_z_temp = imu_reading.data_acc.z - 2732;
//				acc_z_filtered += acc_z_temp;
//				acc_z_filter_index ++;
//				if(acc_z_filter_index == 8){
//
//					acc_z_filtered = acc_z_filtered >>3;
//					acc_z_filter_index = 0;
//
//				}



//				sprintf(buf,"%d acc.z %ld acc.z filtered\n",imu_reading.data_acc.z,acc_z_filtered);
//				transmit_line(huart, buf);
			}


			HAL_GPIO_TogglePin(USR_LED_PORT,USR_LED_PIN);
		}
		else{

			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.data[i] = 0;
			}
			continue;
		}

		/* BMP READING*******************************************************************************************************************************/
		//Try and get data from the BMP queue. Block for up to a quarter of the time between the fastest measurement.
		stat = xQueueReceive(logStruct->PRES_data_queue,&bmp_reading,configParams->values.data_rate/4);

		if(stat == pdPASS){

			is_there_data = isMeasurementEmpty(&measurement);

			if(is_there_data){
				//We already have a imu reading.

				measurement_length += (PRES_LENGTH + TEMP_LENGTH + ALT_LENGTH);

				//Update the header bytes.
				uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
				header |= PRES_TYPE | TEMP_TYPE;

				measurement.data[0] = (header >> 16) & 0xFF;
				measurement.data[1] = (header >> 8) & 0xFF;
				measurement.data[2] = (header) & 0xFF;

				measurement.data[15]= (((uint32_t)bmp_reading.data.pressure) >>16) &0xFF ;	//MSB
				measurement.data[16]= (((uint32_t)bmp_reading.data.pressure) >> 8) & 0xFF;	//LSB
				measurement.data[17]= ((uint32_t)bmp_reading.data.pressure) & 0xFF;		//XLSB

				measurement.data[18]= (((uint32_t)bmp_reading.data.temperature) >>16) & 0xFF;	//MSB
				measurement.data[19]= ((uint32_t)bmp_reading.data.temperature >> 8) & 0xFF;	//LSB
				measurement.data[20]= (uint32_t)bmp_reading.data.temperature & 0xFF; //XLSB

		    	altitude = altitude_approx((float)bmp_reading.data.pressure, (float)bmp_reading.data.temperature,configParams);
		    	measurement.data[21] = (altitude.byte_val>>24) & 0xFF;
		    	measurement.data[22] = (altitude.byte_val>>16) & 0xFF;
		    	measurement.data[23] = (altitude.byte_val>>8) & 0xFF;
		    	measurement.data[24] = (altitude.byte_val) & 0xFF;


		    	altFiltered = altFiltered + (altitude.float_val - altFiltered)*0.2;

//		    	int16_t alt_int = altitude.float_val;
//		    	int16_t alt_dec = (altitude.float_val*100)-(alt_int*100);
//				int16_t alt_int = altFiltered;
//				int16_t alt_dec = (altFiltered*100)-(alt_int*100);
//		    	char c = ' ';
//		    	if(alt_int < 0 ){
//		    		alt_int = -alt_int;
//		    		c = '-';
//		    	}
//		    	if(alt_dec < 0 ){
//		    		alt_dec = -alt_dec;
//		    		c = '-';
//		    	}
//
//		    	sprintf(buf, "Altitude: %c%d.%02d [m]", c,alt_int,alt_dec);
//		    	transmit_line(huart, buf);

			}


		}
		else{
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.data[i] = 0;
			}
			continue;
		}


		if(configParams->values.state == STATE_LAUNCHPAD_ARMED && imu_reading.data_acc.x>10892){
			
			buzz(250);
			vTaskResume(*timerTask_h); //start fixed timers.
			configParams->values.state = STATE_IN_FLIGHT_PRE_APOGEE;
			configParams->values.flags = configParams->values.flags |  0x04;
			//configParams->values.state = STATE_IN_FLIGHT_POST_APOGEE;
			//Record the launch event.
			uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
			header |= LAUNCH_DETECT;
			configParams->values.flags = configParams->values.flags | 0x01;
			write_config(configParams);

			measurement.data[0] = (header >> 16) & 0xFF;
			measurement.data[1] = (header >> 8) & 0xFF;
			measurement.data[2] = (header) & 0xFF;
			
			int p;
			uint16_t buff_end = (ring_buff_size);
			for(p=0;p<25;p++){
				//need to copy last portion into the bufferA. Make sure to start from right place, which wont be the next spot.

				if((buffer_index_curr + 256) < buff_end){
				FlashStatus_t stat_f2 = program_page(flash_ptr,flash_address,&launchpadBuffer[buffer_index_curr],DATA_BUFFER_SIZE);
				while(IS_DEVICE_BUSY(stat_f2)){
					  stat_f2 = get_status_reg(flash_ptr);
					 vTaskDelay(1);
				 }
				 buffer_index_curr += 256;
				}
				else{

					uint8_t buff_temp[256];
					memcpy(&buff_temp,&launchpadBuffer[buffer_index_curr],buff_end-buffer_index_curr);
					memcpy(&buff_temp[buff_end-buffer_index_curr],&launchpadBuffer,DATA_BUFFER_SIZE-(buff_end-buffer_index_curr));

					FlashStatus_t stat_f2 = program_page(flash_ptr,flash_address,buff_temp,DATA_BUFFER_SIZE);
					  while(IS_DEVICE_BUSY(stat_f2)){
						  stat_f2 = get_status_reg(flash_ptr);
						 vTaskDelay(1);
					  }
					  buffer_index_curr = DATA_BUFFER_SIZE-(buff_end-buffer_index_curr);
				}

				flash_address += DATA_BUFFER_SIZE;
			}
			buffer_index_curr = 0 ;
		}

		//Check if the rocket has landed.
 		if(configParams->values.state == STATE_IN_FLIGHT_POST_MAIN){

			if(alt_count >0){

				//If altitude is within a 1m range for 20 samples
				if(altitude.float_val>(alt_prev.float_val - 1.0) && altitude.float_val < (alt_prev.float_val+1.0)){
					alt_count++;
					if(alt_count >245){
						alt_count = 201;
					}
				}else{
					alt_count = 0;
				}

			}
			else{

				alt_prev.float_val = altitude.float_val;
				alt_count++;
				if(alt_count >245){
					alt_count = 201;
				}
			}

			if((pow(imu_reading.data_gyro.x,2)+pow(imu_reading.data_gyro.y,2)+pow(imu_reading.data_gyro.z,2))<63075){
			//If the gyro readings are all less than ~4.4 deg/sec and the altitude is not changing then the rocket has probably landed.

				if(alt_count > 200){

					configParams->values.state = STATE_LANDED;
					configParams->values.flags = configParams->values.flags & ~(0x01);
					write_config(configParams);
					uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
					header |= LAND_DETECT;

					measurement.data[0] = (header >> 16) & 0xFF;
					measurement.data[1] = (header >> 8) & 0xFF;
					measurement.data[2] = (header) & 0xFF;

					//Put everything into low power mode.
					running = 0;

				}
			}
		}
		
		
		//Check if the altitude is below 1500ft, after the drogue has been deployed.
		if(configParams->values.state == STATE_IN_FLIGHT_POST_APOGEE ){

			if(altFiltered<375.0){
				//375m ==  1230 ft
				alt_main_count ++;
			}
			else{
				alt_main_count = 0;
			}
			if(alt_main_count>5){
				//deploy main
				buzz(250);
				recoverySelect_t event = MAIN;
				enable_mosfet(event);
				activate_mosfet(event);
				continuityStatus_t cont = check_continuity(event);
				uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
				header |= MAIN_DETECT;

				measurement.data[0] = (header >> 16) & 0xFF;
				measurement.data[1] = (header >> 8) & 0xFF;
				measurement.data[2] = (header) & 0xFF;

				if(cont == OPEN_CIRCUIT){

					configParams->values.state =  STATE_IN_FLIGHT_POST_MAIN;
					configParams->values.flags = configParams->values.flags |  0x10;
					write_config(configParams);
					uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
					header |= MAIN_DEPLOY;
					measurement.data[0] = (header >> 16) & 0xFF;
					measurement.data[1] = (header >> 8) & 0xFF;
					measurement.data[2] = (header) & 0xFF;

				}
			}
		}

		//check if rocket has reached apogee.
		if(configParams->values.state == STATE_IN_FLIGHT_PRE_APOGEE){

			apogee_holdout_count ++;
			if(apogee_holdout_count >(20*15)){

				uint64_t acc_mag = pow(imu_reading.data_acc.x,2)+pow(imu_reading.data_acc.y,2)+pow(imu_reading.data_acc.z,2);
				if(acc_mag < 1 && altFiltered > 9000.0){
					//5565132 = 3 * 1362^2 (aprox 0.5 g on all direction)
					//2438m -> 8,000 ft

					buzz(250);
					recoverySelect_t event = DROGUE;
					enable_mosfet(event);
					activate_mosfet(event);
					continuityStatus_t cont = check_continuity(event);
					uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
					header |= DROGUE_DETECT;
					measurement.data[0] = (header >> 16) & 0xFF;
					measurement.data[1] = (header >> 8) & 0xFF;
					measurement.data[2] = (header) & 0xFF;

					if(cont == OPEN_CIRCUIT){
						configParams->values.state =  STATE_IN_FLIGHT_POST_APOGEE;
						configParams->values.flags = configParams->values.flags |  0x08;
						write_config(configParams);

						uint32_t header = (measurement.data[0]<<16)+(measurement.data[1]<<8) + measurement.data[2];
						header |= DROGUE_DEPLOY;
						measurement.data[0] = (header >> 16) & 0xFF;
						measurement.data[1] = (header >> 8) & 0xFF;
						measurement.data[2] = (header) & 0xFF;

					}

				}
			}
		}


		/* Fill Buffer and/or write to flash*********************************************************************************************************/
		is_there_data = isMeasurementEmpty(&measurement);

		if(is_there_data && configParams->values.state == STATE_LAUNCHPAD_ARMED && ((buffer_index_curr+measurement_length + HEADER_SIZE) <= ring_buff_size)){

			//check if room in launchpad buffer.


			memcpy(&launchpadBuffer[buffer_index_curr],&(measurement.data),measurement_length+HEADER_SIZE);

			buffer_index_curr += (measurement_length+HEADER_SIZE);
			buffer_index_curr = buffer_index_curr % (ring_buff_size);

			//Reset the measurement.
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.data[i] = 0;
			}
		}

//		else if (is_there_data && configParams->values.state == STATE_LAUNCHPAD_ARMED){
//			uint8_t bytesInPrevBuffer = ring_buff_size - buffer_index_curr;
//			uint8_t bytesLeft = (measurement_length+HEADER_SIZE)-bytesInPrevBuffer;
//
//			memcpy(&launchpadBuffer[buffer_index_curr],&(measurement.data),bytesInPrevBuffer);
//			buffer_index_curr = 0;
//			memcpy(&launchpadBuffer[buffer_index_curr],&(measurement.data[bytesInPrevBuffer]),bytesLeft);
//
//			buffer_index_curr = bytesLeft;
//		}
		else if(((buffer_index_curr+measurement_length + HEADER_SIZE) < DATA_BUFFER_SIZE) && (is_there_data)){

			//There is room in the current buffer for the full measurement.

			if(buffer_selection == BUFFER_A){

				memcpy(&data_bufferA[buffer_index_curr],&(measurement.data),measurement_length+HEADER_SIZE);
				//transmit_bytes(huart,&data_bufferA[buffer_index_curr],measurement_length+2);
			}
			else if(buffer_selection == BUFFER_B){

				memcpy(&data_bufferB[buffer_index_curr],&(measurement.data),measurement_length+HEADER_SIZE);
				//transmit_bytes(huart,&data_bufferB[buffer_index_curr],measurement_length+2);
			}

			buffer_index_curr += (measurement_length+HEADER_SIZE);

			//Reset the measurement.
			for(i=0;i<sizeof(Measurement_t);i++){

				measurement.data[i] = 0;
			}



		}else if(is_there_data){

			//Split measurement across the buffers, and write to flash.
			uint8_t bytesInPrevBuffer = DATA_BUFFER_SIZE - buffer_index_curr;
			uint8_t bytesLeft = (measurement_length+HEADER_SIZE)-bytesInPrevBuffer;

//			if((((measurement.data[1]<<8)+measurement.data[0])&0x0FFF)>300){
//
//				while(1);
//			}

//			if((bytesLeft+bytesInPrevBuffer)!=20 && ((bytesLeft+bytesInPrevBuffer)!=14)){
//
//				while(1){}
//			}

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

				if(IS_RECORDING(configParams->values.flags)){

					FlashStatus_t stat_f = program_page(flash_ptr,flash_address,data_bufferB,DATA_BUFFER_SIZE);
					  while(IS_DEVICE_BUSY(stat_f)){
						  stat_f = get_status_reg(flash_ptr);
						  vTaskDelay(1);
					  }

					flash_address += DATA_BUFFER_SIZE;
					if(flash_address>=FLASH_SIZE_BYTES){
						while(1);
					}

				}
				else{
					transmit_bytes(huart,data_bufferB,256);
				}
			}
			else if (buffer_selection == 1){
				//We just switched to B so transmit A

				if(IS_RECORDING(configParams->values.flags)){
					FlashStatus_t stat_f2 = program_page(flash_ptr,flash_address,data_bufferA,DATA_BUFFER_SIZE);
					  while(IS_DEVICE_BUSY(stat_f2)){
						  stat_f2 = get_status_reg(flash_ptr);
						  vTaskDelay(1);
					  }

					flash_address += DATA_BUFFER_SIZE;

					if(flash_address>=FLASH_SIZE_BYTES){
						while(1);
					}
				}
				else{
					transmit_bytes(huart,data_bufferA,256);
				}
			}


		}

		for(i=0;i<sizeof(Measurement_t);i++){

			measurement.data[i] = 0;
		}

		measurement_length=0;

		if(!running){
			vTaskSuspend(NULL);
		}
	};

}
