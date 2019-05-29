//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS>Avionics-2019
//
// File Description:
//  xtraxt.c UART CLI utility to pull data off of STM32 flash memory
//
// History
// 2019-02-15 by Eric Kapilik
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xtract.h"
#include "buttonpress.h"
#include "cmsis_os.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//static UART_HandleTypeDef* uart;
//static FlashStruct_t * flash;
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
//static int private_fnc(
//    const void * param1, // Enter description of param1.
//    const void * param2  // Enter description of param2.
//    );

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_xtract(void *pvParameters){

	xtractParams * params = (xtractParams *)pvParameters;

	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;

	menuState_t state = MAIN;
	intro(uart); //display help on start up
	char *cmd_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE); //command buffer
	state = MAIN;
	/* As per most FreeRTOS tasks, this task is implemented in an infinite loop. */
	while(1){
		transmit(uart, ">> ");
		cmd_buf = receive_command(uart); //puts input into buffrx
		handle_command(cmd_buf,params,&state); //handles command sitting in buffrx
	}
} //vTaskUART_CLI END

void handle_command(char* command,xtractParams * params,menuState_t * state){


	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;
	configData_t * config = params->flightCompConfig;


	char output[BUFFER_SIZE];

	if(strcmp(command, "help") == 0 && *state == MAIN){
		help(uart);
	}
	else if(strcmp(command, "start") == 0 && *state == MAIN){
		//start();
	}
	else if((strcmp(command, "read") == 0 && *state == MAIN )|| *state == READ){
		read(params);
	}
	else if((strcmp(command, "config") == 0 && *state == MAIN )|| *state == CONFIG){

		if(strcmp(command,"return")==0){
			*state = MAIN;
		}else{
			*state = CONFIG;
		}
		configure(command,params);
	}
	else if((strcmp(command, "ematch") == 0 && *state == MAIN) || *state == EMATCH){
		*state = 2;

	}
	else if((strcmp(command, "mem") == 0 && *state == MAIN )|| *state == MEM){
		*state = 3;

	}
	else if((strcmp(command, "save") == 0 && *state == MAIN )|| *state == SAVE){

	}
	else{
		sprintf(output, "Command [%s] not recognized.", command);
		transmit_line(uart, output);
	}

}

void intro(UART_HandleTypeDef * uart){

	transmit_line(uart, "========== Welcome to Xtract ==========\r\n"
				"This is a command line interface tool made by the Avionics subdivison of the Rockets team.\r\n\r\n"
				"Here are some commands to get you started:");
	help(uart);
}

void help(UART_HandleTypeDef * uart){
	transmit_line(uart, "Commands:\r\n"
					"\t[help] - displays the help menu and more commands\r\n"
					"\t[read] - Downloads flight data\r\n"
					"\t[config] - Setup flight computer\r\n"
					"\t[ematch] - check and fire ematches\r\n"
					"\t[mem] - Check on and erase the flash memory\r\n"
					"\t[save] - Save all setting to the flight computer\r\n"
					"\t[start] - Start the flight computer\r\n"
					);
}

void configure(char* command,xtractParams * params){

	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;
	configData_t * config = params->flightCompConfig;

	char output [256];

	if(strcmp(command, "help") == 0 || strcmp(command, "config") == 0){

		transmit_line(uart, "Commands:\r\n"
						"\t[help] - displays the help menu and more commands\r\n"
						"\t[return] - Return to main menu\r\n"
						"\t[a] - Set data rate Hz(0-100)\r\n"
						"\t[b] - set if recording to flash (1/0)\r\n"
						"\t[c] - set accelerometer bandwidth (0,2,4)\r\n"
						"\t[d] - set accelerometer range (3,6,12,24)\r\n"
						"\t[e] - set accelerometer odr (12,25,50,100,200,400,800,1600)\r\n"
						"\t[f] - set gyro bandwidth and odr (32_100,64_200,12_100,23_200,47_400,116_1000,230_2000,532_2000)\r\n\t(Enter the number (1-8) for the option to select)\r\n"
						"\t[g] - set gyro range (125,250,500,1000,2000)\r\n"
						"\t[i] - set BMP388 odr (1,12,25,50,100,200) \r\n"
						"\t[j] - set pressure oversampling (0,2,4,8,16,32) \r\n"
						"\t[k] - set temperature oversampling (0,2,4,8,16,32) \r\n"
						"\t[l] - set BMP388 IIR filter coefficient (1,3,7,15,31,63,127) \r\n"
						"\t[m] - Read the current settings\r\n"
						);

	}
	else if (command[0] == 'a'){
		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);
		if( value >0 && value <=100){

			sprintf(output,"Setting data rate to %d Hz.\n",value);

			transmit_line(uart,output);
			config->values.data_rate = 1000/value;

		}
	}
	else if (command[0] == 'b'){

		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

		case 0:
			sprintf(output,"Turning off flash recording.\n");
			transmit_line(uart,output);
			config->values.flags &= ~(0x02);
			break;
		case 1:
			sprintf(output,"Turning on flash recording.\n");
			transmit_line(uart,output);
			config->values.flags |= (0x02);
			break;

		}

	}
	else if (command[0] == 'c'){

		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

		case 0:
			sprintf(output,"Setting accelerometer to no over-sampling .\n");
			transmit_line(uart,output);
			config->values.ac_bw = BMI08X_ACCEL_BW_NORMAL;
			break;
		case 2:
			sprintf(output,"Setting accelerometer to 2x over-sampling .\n");
			transmit_line(uart,output);
			config->values.ac_bw = BMI08X_ACCEL_BW_OSR2;
			break;
		case 4:
			sprintf(output,"Setting accelerometer to 4x over-sampling .\n");
			transmit_line(uart,output);
			config->values.ac_bw = BMI08X_ACCEL_BW_OSR4;
			break;
		}

	}
	else if (command[0] == 'd'){


		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

		case 3:
			sprintf(output,"Setting accelerometer range to %d g .\n",value);
			transmit_line(uart,output);
			config->values.ac_range = BMI088_ACCEL_RANGE_3G;
			break;

		case 6:
			sprintf(output,"Setting accelerometer range to %d g .\n",value);
			transmit_line(uart,output);
			config->values.ac_range = BMI088_ACCEL_RANGE_6G;
			break;

		case 12:
			sprintf(output,"Setting accelerometer range to %d g .\n",value);
			transmit_line(uart,output);
			config->values.ac_range = BMI088_ACCEL_RANGE_12G;
			break;

		case 24:
			sprintf(output,"Setting accelerometer range to %d g .\n",value);
			transmit_line(uart,output);
			config->values.ac_range = BMI088_ACCEL_RANGE_24G;
			break;

		}


	}
	else if (command[0] == 'e'){
		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

		case 12:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_12_5_HZ;
			break;

		case 25:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_25_HZ;
			break;

		case 50:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_50_HZ;
			break;

		case 100:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_100_HZ;
			break;

		case 200:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_200_HZ;
			break;

		case 400:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_400_HZ;
			break;

		case 800:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_800_HZ;
			break;

		case 1600:
			sprintf(output,"Setting accelerometer odr to %d Hz .\n",value);
			transmit_line(uart,output);
			config->values.ac_odr = BMI08X_ACCEL_ODR_1600_HZ;
			break;

		}
	}
	else if (command[0] == 'f'){
		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

		case 1:
			sprintf(output,"Setting gyroscope to BW_32_ODR_100_HZ .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_32_ODR_100_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_32_ODR_100_HZ;
			break;

		case 2:
			sprintf(output,"Setting gyroscope BW_64_ODR_200_HZ .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_64_ODR_200_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_64_ODR_200_HZ;
			break;

		case 3:
			sprintf(output,"Setting gyroscope BW_12_ODR_100_HZ  .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_12_ODR_100_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_12_ODR_100_HZ;
			break;

		case 4:
			sprintf(output,"Setting gyroscope BW_23_ODR_200_HZ  .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_23_ODR_200_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_23_ODR_200_HZ;
			break;

		case 5:
			sprintf(output,"Setting gyroscope BW_47_ODR_400_HZ  .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_47_ODR_400_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_47_ODR_400_HZ;
			break;

		case 6:
			sprintf(output,"Setting gyroscope BW_116_ODR_1000_HZ  .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_116_ODR_1000_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_116_ODR_1000_HZ;
			break;

		case 7:
			sprintf(output,"Setting gyroscope BW_230_ODR_2000_HZ  .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_230_ODR_2000_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_230_ODR_2000_HZ;
			break;

		case 8:
			sprintf(output,"Setting gyroscope BW_532_ODR_2000_HZ  .\n" );
			transmit_line(uart,output);
			config->values.gy_odr = BMI08X_GYRO_BW_532_ODR_2000_HZ;
			config->values.gy_bw = BMI08X_GYRO_BW_532_ODR_2000_HZ;
			break;

		}
	}
	else if (command[0] == 'g'){

	}
	else if (command[0] == 'i'){

	}
	else if (command[0] == 'j'){

	}
	else if (command[0] == 'k'){

	}
	else if (command[0] == 'l'){

	}
	else if (command[0] == 'm'){

	}
	else{
		sprintf(output, "Command [%s] not recognized.", command);
		transmit_line(uart, output);
	}

}

void read(xtractParams * params){

	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;
	configData_t * config = params->flightCompConfig;

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	transmit_line(uart, "Data transfer will start in 20 seconds. The LED will turn off when the transfer is complete.");

	uint8_t buffer[256*5]; 	//Read 5 pages from flash at a time;

	uint32_t bytesRead = 0;
	uint32_t currentAddress = FLASH_START_ADDRESS;

	vTaskDelay(pdMS_TO_TICKS(1000*20));	//Delay 10 seconds

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	while (bytesRead < scan_flash(flash)){

		read_page(flash,currentAddress,buffer,256*5);

		int i;
		uint16_t empty =0 ;
		for(i=0;i<256*5;i++){

			if(buffer[i] == 0xFF){
				empty += 1;
			}
		}
		if(empty == (256*5)){
			break;
		}
		transmit_bytes(uart,buffer,256*5);

		currentAddress += (256*5);
		currentAddress = currentAddress % FLASH_SIZE_BYTES;

		bytesRead += 256*5;
		vTaskDelay(1);
	}
	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
}
