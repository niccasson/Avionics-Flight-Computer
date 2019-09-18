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

uint16_t delay_ematch_menu_fire = 10000;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_xtract(void *pvParameters){

	xtractParams * params = (xtractParams *)pvParameters;

	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;

	menuState_t state = MAIN_MENU;
	intro(uart); //display help on start up
	char *cmd_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE); //command buffer
	state = MAIN_MENU;
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
	TaskHandle_t startupTaskHandle = params->startupTaskHandle;


	char output[BUFFER_SIZE];

	if(strcmp(command, "help") == 0 && *state == MAIN_MENU){
		help(uart);
	}
	else if((strcmp(command, "read") == 0 && *state == MAIN_MENU )|| *state == READ_MENU){
		read(params);
	}
	else if((strcmp(command, "config") == 0 && *state == MAIN_MENU )|| *state == CONFIG_MENU){

		if(strcmp(command,"return")==0){
			transmit_line(uart,"Returning to main menu");
			*state = MAIN_MENU;
		}else{
			*state = CONFIG_MENU;
		}
		configure(command,params);
	}
	else if((strcmp(command, "ematch") == 0 && *state == MAIN_MENU) || *state == EMATCH_MENU){
		*state = EMATCH_MENU;
		if(strcmp(command,"return")==0){
			transmit_line(uart,"Returning to main menu");
			*state = MAIN_MENU;
		}else{
			*state = EMATCH_MENU;
		}
		ematch(command,params);
	}
	else if((strcmp(command, "mem") == 0 && *state == MAIN_MENU )|| *state == MEM_MENU){
		*state = MEM_MENU;
		if(strcmp(command,"return")==0){
			transmit_line(uart,"Returning to main menu");
			*state = MAIN_MENU;
		}else{
			*state = MEM_MENU;
		}
		memory_menu(command,params);

	}
	else if((strcmp(command, "save") == 0 && *state == MAIN_MENU )|| *state == SAVE_MENU){
		write_config(config);

	}
	else if((strcmp(command, "start") == 0 && *state == MAIN_MENU )){

		config->values.state = STATE_LAUNCHPAD_ARMED;
		vTaskResume(startupTaskHandle);

		vTaskSuspend(NULL);

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


void memory_menu(char* command, xtractParams * params){

	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;
	configData_t * config = params->flightCompConfig;

	char output [256];
	if(strcmp(command, "help") == 0 || strcmp(command, "mem") == 0){

		transmit_line(uart, "Commands:\r\n"
						"\t[help] - displays the help menu and more commands\r\n"
						"\t[return] - Return to main menu\r\n"
						"\t[a] - Read 256 bytes (hex address 0-7FFFFF).\r\n"
						"\t[b] - Scan Memory\r\n"
						"\t[c] - Erase data section\r\n"
						"\t[d] - Erase config section.\r\n"
						"\t[e] - Erase all flash memory.\r\n"
						);

	}
	else if (strcmp(command,"return")==0){

	}
	else if (command[0] == 'a'){


		char val_str[10];

		strcpy(val_str,&command[1]);

		uint32_t value = strtol(val_str,NULL,16);

		if(value>=0 && value<= FLASH_END_ADDRESS){


			sprintf(output,"Reading 256 bytes starting at address %ld ...",value);
			transmit_line(uart,output);

			uint8_t data_rx[FLASH_PAGE_SIZE];

			FlashStatus_t stat;
			stat = read_page(flash,value,data_rx,FLASH_PAGE_SIZE);

			uint8_t busy = stat;

			while(IS_DEVICE_BUSY(busy)){

				busy = get_status_reg(flash);

				vTaskDelay(pdMS_TO_TICKS(1));
			}

			if(stat == FLASH_OK){
				sprintf(output,"Success:");
			}
			else{
				sprintf(output,"Failed:");

			}
			transmit_line(uart,output);

			int i;
			for(i=0;i<FLASH_PAGE_SIZE;i++){


				if((i+1)%16 == 0){
					sprintf(output,"0x%02X ",data_rx[i]);
					transmit_line(uart,output);
				}
				else{
					sprintf(output,"0x%02X ",data_rx[i]);
					transmit(uart,output);
				}


			}
				transmit_line(uart,"\r\n");
		}



		}
	else if (command[0] == 'b'){


		uint32_t end_Address = scan_flash(flash);
		sprintf(output,"end address :%ld \n",end_Address);
		transmit_line(uart,output);

		}
	else if (command[0] == 'c'){

			uint8_t dataRX[FLASH_PAGE_SIZE];

			sprintf(output,"Erasing data section ...");
			transmit_line(uart,output);

			  uint32_t address = FLASH_START_ADDRESS;
			  FlashStatus_t stat;
			  while(address <= FLASH_END_ADDRESS){

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

				  HAL_GPIO_TogglePin(USR_LED_PORT,USR_LED_PIN);
					sprintf(output,"Erasing sector %ld ...",address);
					transmit_line(uart,output);
			  }

			  read_page(flash,FLASH_START_ADDRESS,dataRX,256);
			  uint16_t empty = 0xFFFF;
			  	  int i;
			  for(i=0;i<256;i++){

				  if(dataRX[i] != 0xFF){
					 empty --;
				  }
			  }

			  if(empty == 0xFFFF){

				  transmit_line(uart,"Flash Erased Success!");
			  }

			if(stat == FLASH_OK){
				sprintf(output,"Success:");
			}
			else{
				sprintf(output,"Failed:");

			}
			transmit_line(uart,output);



		}
	else if (command[0] == 'd'){

		}
	else if (command[0] == 'f'){

		}

}

void ematch(char* command, xtractParams * params){

	UART_HandleTypeDef * uart = params->huart;
	FlashStruct_t * flash = params->flash;
	configData_t * config = params->flightCompConfig;

	char output [256];


	if(strcmp(command, "help") == 0 || strcmp(command, "ematch") == 0){

		transmit_line(uart, "E-Matches:\r\n"
						"\t[help] - displays the help menu and more commands\r\n"
						"\t[return] - Return to main menu\r\n"
						"\t[a] - Check continuity Drogue\r\n"
						"\t[b] - Check continuity Main\r\n"
						"\t[c] - Check overcurrent Drogue\r\n"
						"\t[d] - Check overcurrent Main\r\n"
						"\t[e] - Enable Drogue\r\n"
						"\t[f] - Enable Main\r\n"
						"\t[g] - Fire Drogue (delayed)\r\n"
						"\t[i] - Fire Main   (delayed)\r\n"
						"\t[j] - Set delay (5-60)\r\n "
						);

	}
	else if (strcmp(command,"return")==0){

	}
	else if (command[0] == 'a'){

		recoverySelect_t event = DROGUE;
		continuityStatus_t cont = check_continuity(event);

		 if(cont == OPEN_CIRCUIT){
			 sprintf(output,"No continuity was detected on the drogue circuit.\n");
			 transmit_line(uart,output);
		 }
		 else{
			 sprintf(output,"Continuity was detected on the drogue circuit.\n");
			 transmit_line(uart,output);
		 }

		}
	else if (command[0] == 'b'){

		recoverySelect_t event = MAIN;
		continuityStatus_t cont = check_continuity(event);

		 if(cont == OPEN_CIRCUIT){
			 sprintf(output,"No continuity was detected on the main circuit.\n");
			 transmit_line(uart,output);
		 }
		 else{
			 sprintf(output,"Continuity was detected on the main circuit.\n");
			 transmit_line(uart,output);
		 }

		}
	else if (command[0] == 'c'){

		recoverySelect_t event = DROGUE;
		overcurrentStatus_t over = check_overcurrent(event);

		 if(over == NO_OVERCURRENT){
			 sprintf(output,"No overcurrent was detected on the drogue circuit.\n");
			 transmit_line(uart,output);
		 }
		 else{
			 sprintf(output,"Overcurrent was detected on the drogue circuit.\n");
			 transmit_line(uart,output);
		 }

		}
	else if (command[0] == 'd'){

		recoverySelect_t event = MAIN;
		overcurrentStatus_t over = check_overcurrent(event);

		 if(over == NO_OVERCURRENT){
			 sprintf(output,"No overcurrent was detected on the main circuit.\n");
			 transmit_line(uart,output);
		 }
		 else{
			 sprintf(output,"Overcurrent was detected on the main circuit.\n");
			 transmit_line(uart,output);
		 }

		}
	else if (command[0] == 'e'){

		recoverySelect_t event = DROGUE;
		enable_mosfet(event);
		sprintf(output,"DROGUE DEPLOYMENT CIRCUIT IS NOW ARMED!.\n");
		 transmit_line(uart,output);
		}
	else if (command[0] == 'f'){

		recoverySelect_t event = MAIN;
		enable_mosfet(event);
		sprintf(output,"MAIN DEPLOYMENT CIRCUIT IS NOW ARMED!.\n");
		transmit_line(uart,output);

		}
	else if (command[0] == 'g'){

			recoverySelect_t event = DROGUE;

			sprintf(output,"DROGUE WILL FIRE IN %d SECONDS!.\n",delay_ematch_menu_fire/1000);
			transmit_line(uart,output);

			int time_left = delay_ematch_menu_fire;

			while(time_left>0){

				vTaskDelay(pdMS_TO_TICKS(1000));
				time_left -= (1000);
				sprintf(output,"DROGUE WILL FIRE IN %d SECONDS!.\n",time_left/1000);
				transmit_line(uart,output);
			}
			activate_mosfet(event);
		}
	else if (command[0] == 'i'){

		recoverySelect_t event = MAIN;

		sprintf(output,"MAIN WILL FIRE IN %d SECONDS!.\n",delay_ematch_menu_fire/1000);
		transmit_line(uart,output);

		int time_left = delay_ematch_menu_fire;

		while(time_left>0){

			vTaskDelay(pdMS_TO_TICKS(1000));
			time_left -= (1000);
			sprintf(output,"MAIN WILL FIRE IN %d SECONDS!.\n",time_left/1000);
			transmit_line(uart,output);
		}
			activate_mosfet(event);

		}
	else if (command[0] == 'j'){

		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		if(value>5 && value<60){

			delay_ematch_menu_fire = value *1000;
			sprintf(output,"E-match fire delay set to %d.\n",value);
			transmit_line(uart,output);
		}

		}



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
						"\t[z] - Set the initial time to wait (0-10000000)\r\n"
						"\t[b] - set if recording to flash (1/0)\r\n"
						"\t[c] - set accelerometer bandwidth (0,2,4)\r\n"
						"\t[d] - set accelerometer range (3,6,12,24)\r\n"
						"\t[e] - set accelerometer odr (12,25,50,100,200,400,800,1600)\r\n"
						"\t[f] - set gyro bandwidth and odr (32_100,64_200,12_100,23_200,47_400,116_1000,230_2000,532_2000)\r\n\t(Enter the number (1-8) for the option to select)\r\n"
						"\t[g] - set gyro range (125,250,500,1000,2000)\r\n"
						"\t[i] - set BMP388 odr (1,12,25,50,100,200) \r\n"
						"\t[j] - set pressure oversampling (0,2,4,8,16,32) \r\n"
						"\t[k] - set temperature oversampling (0,2,4,8,16,32) \r\n"
						"\t[l] - set BMP388 IIR filter coefficient (0,1,3,7,15,31,63,127) \r\n"
						"\t[m] - Read the current settings\r\n"
						"\t[n] - Set if in flight (1/0)\r\n"
						);

	}
	else if (strcmp(command,"return")==0){

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
	else if (command[0] == 'z'){

		char val_str[15];

				strcpy(val_str,&command[1]);

				int value = atoi(val_str);
				if( value >0 && value <=10000000){

					sprintf(output,"Setting initial time to wait to %d ms.\n",value);

					transmit_line(uart,output);
					config->values.initial_time_to_wait = value;

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

		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

				case 125:
					sprintf(output,"Setting gyroscope range to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.gy_range = BMI08X_GYRO_RANGE_125_DPS;
					break;

				case 250:
					sprintf(output,"Setting gyroscope range to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.gy_range = BMI08X_GYRO_RANGE_250_DPS;
					break;

				case 500:
					sprintf(output,"Setting gyroscope range to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.gy_range = BMI08X_GYRO_RANGE_500_DPS;
					break;

				case 1000:
					sprintf(output,"Setting gyroscope range to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.gy_range = BMI08X_GYRO_RANGE_1000_DPS;
					break;

				case 2000:
					sprintf(output,"Setting gyroscope range to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.gy_range = BMI08X_GYRO_RANGE_2000_DPS;
					break;

		}

	}
	else if (command[0] == 'i'){

		//set BMP388 odr (1,12,25,50,100,200) \r\n"
		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

				case 1:
					sprintf(output,"Setting bmp odr to 1.5 Hz .\n" );
					transmit_line(uart,output);
					config->values.bmp_odr = BMP3_ODR_1_5_HZ;
					break;

				case 12:
					sprintf(output,"Setting bmp odr to 12.5 Hz .\n");
					transmit_line(uart,output);
					config->values.bmp_odr = BMP3_ODR_12_5_HZ;
					break;

				case 25:
					sprintf(output,"Setting bmp odr to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.bmp_odr = BMP3_ODR_25_HZ;
					break;

				case 50:
					sprintf(output,"Setting bmp odr to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.bmp_odr = BMP3_ODR_50_HZ;
					break;

				case 100:
					sprintf(output,"Setting bmp odr to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.bmp_odr = BMP3_ODR_100_HZ;
					break;

				case 200:
					sprintf(output,"Setting bmp odr to %d Hz .\n",value );
					transmit_line(uart,output);
					config->values.bmp_odr = BMP3_ODR_200_HZ;
					break;

		}
	}
	else if (command[0] == 'j'){
		//set pressure oversampling (0,2,4,8,16,32)


		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

				case 0:
					sprintf(output,"Setting pressure oversampling 0x .\n" );
					transmit_line(uart,output);
					config->values.pres_os =  BMP3_NO_OVERSAMPLING;
					break;

				case 2:
					sprintf(output,"Setting pressure oversampling 2x  .\n");
					transmit_line(uart,output);
					config->values.pres_os = BMP3_OVERSAMPLING_2X;
					break;

				case 4:
					sprintf(output,"Setting pressure oversampling 4x .\n");
					transmit_line(uart,output);
					config->values.pres_os = BMP3_OVERSAMPLING_4X;
					break;

				case 8:
					sprintf(output,"Setting pressure oversampling to 8x Hz .\n" );
					transmit_line(uart,output);
					config->values.pres_os = BMP3_OVERSAMPLING_8X;
					break;

				case 16:
					sprintf(output,"Setting pressure oversampling to 16x Hz .\n" );
					transmit_line(uart,output);
					config->values.pres_os = BMP3_OVERSAMPLING_16X;
					break;

				case 32:
					sprintf(output,"Setting pressure oversampling to 32x Hz .\n" );
					transmit_line(uart,output);
					config->values.pres_os = BMP3_OVERSAMPLING_32X;
					break;

		}
	}
	else if (command[0] == 'k'){

		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

				case 0:
					sprintf(output,"Setting temperature oversampling 0x .\n" );
					transmit_line(uart,output);
					config->values.temp_os =  BMP3_NO_OVERSAMPLING;
					break;

				case 2:
					sprintf(output,"Setting temperature oversampling 2x  .\n");
					transmit_line(uart,output);
					config->values.temp_os = BMP3_OVERSAMPLING_2X;
					break;

				case 4:
					sprintf(output,"Setting temperature oversampling 4x .\n");
					transmit_line(uart,output);
					config->values.temp_os = BMP3_OVERSAMPLING_4X;
					break;

				case 8:
					sprintf(output,"Setting temperature oversampling to 8x Hz .\n" );
					transmit_line(uart,output);
					config->values.temp_os = BMP3_OVERSAMPLING_8X;
					break;

				case 16:
					sprintf(output,"Setting temperature oversampling to 16x Hz .\n" );
					transmit_line(uart,output);
					config->values.temp_os = BMP3_OVERSAMPLING_16X;
					break;

				case 32:
					sprintf(output,"Setting temperature oversampling to 32x Hz .\n" );
					transmit_line(uart,output);
					config->values.temp_os = BMP3_OVERSAMPLING_32X;
					break;

		}

	}
	else if (command[0] == 'l'){

		//set BMP388 IIR filter coefficient (1,3,7,15,31,63,127)

		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

				case 0:
					sprintf(output,"Setting bmp IIR filter coefficient to off .\n" );
					transmit_line(uart,output);
					config->values.temp_os =  BMP3_IIR_FILTER_DISABLE;
					break;

				case 1:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n",value );
					transmit_line(uart,output);
					config->values.temp_os =  BMP3_IIR_FILTER_COEFF_1;
					break;

				case 3:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n",value);
					transmit_line(uart,output);
					config->values.temp_os = BMP3_IIR_FILTER_COEFF_3;
					break;

				case 7:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n",value);
					transmit_line(uart,output);
					config->values.temp_os = BMP3_IIR_FILTER_COEFF_7;
					break;

				case 15:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n" ,value);
					transmit_line(uart,output);
					config->values.temp_os = BMP3_IIR_FILTER_COEFF_15;
					break;

				case 31:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n",value );
					transmit_line(uart,output);
					config->values.temp_os = BMP3_IIR_FILTER_COEFF_31;
					break;

				case 63:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n",value);
					transmit_line(uart,output);
					config->values.temp_os = BMP3_IIR_FILTER_COEFF_63;
					break;


				case 127:
					sprintf(output,"Setting bmp IIR filter coefficient to %d .\n",value );
					transmit_line(uart,output);
					config->values.temp_os = BMP3_IIR_FILTER_COEFF_127;
					break;
		}

	}
	else if (command[0] == 'm'){

		sprintf(output,"The current settings (not in flash):");
		transmit_line(uart,output);

		sprintf(output,"ID: %d \tIntitial Time To Wait: %ld \r\n",config->values.id,config->values.initial_time_to_wait);
		transmit_line(uart,output);

		sprintf(output,"data rate: %d Hz \tSet to record: %d \r\n",1000/config->values.data_rate,IS_RECORDING(config->values.flags));
		transmit_line(uart,output);

		sprintf(output,"start of data: %ld \tend of data: %ld \r\n",config->values.start_data_address,config->values.end_data_address);
		transmit_line(uart,output);

		sprintf(output,"reference altitude: %ld \t reference pressure: %ld \r\n",(uint32_t)config->values.ref_alt,(uint32_t)config->values.ref_pres);
		transmit_line(uart,output);

	}
	else if (command[0] == 'n'){


		char val_str[10];

		strcpy(val_str,&command[1]);

		int value = atoi(val_str);

		switch(value){

		case 0:
			sprintf(output,"Setting to not in flight.\n");
			transmit_line(uart,output);
			config->values.flags &= ~(0x1D);
			break;
		case 1:
			sprintf(output,"Setting to in flight.\n");
			transmit_line(uart,output);
			config->values.flags |= (0x01);
			break;

		}
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

	vTaskDelay(pdMS_TO_TICKS(1000*10));	//Delay 10 seconds

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	uint32_t endAddress = scan_flash(flash);
	while (bytesRead < endAddress){

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
