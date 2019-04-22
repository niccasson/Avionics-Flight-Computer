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


	intro(uart); //display help on start up
	char *cmd_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE); //command buffer

	/* As per most FreeRTOS tasks, this task is implemented in an infinite loop. */
	while(1){
		transmit(uart, ">> ");
		cmd_buf = receive_command(uart); //puts input into buffrx
		handle_command(cmd_buf,uart,flash); //handles command sitting in buffrx
	}
} //vTaskUART_CLI END

void handle_command(char* command,UART_HandleTypeDef * uart,FlashStruct_t * flash){
	char output[BUFFER_SIZE];

	if(strcmp(command, "help") == 0){
		help(uart);
	}
	else if(strcmp(command, "read") == 0){
		read(uart,flash);
	}
	else if(strcmp(command, "start") == 0){
		//start();
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
					"\t[read] - reads the test array to your console");
}

void read(UART_HandleTypeDef * uart,FlashStruct_t * flash){

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	transmit_line(uart, "Data transfer will start in 20 seconds. The LED will turn off when the transfer is complete.");

	uint8_t buffer[256*5]; 	//Read 10 pages from flash at a time;

	uint32_t bytesRead = 0;
	uint32_t currentAddress = FLASH_START_ADDRESS;

	vTaskDelay(pdMS_TO_TICKS(1000*20));	//Delay 10 seconds

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	while (bytesRead < FLASH_SIZE_BYTES){

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
