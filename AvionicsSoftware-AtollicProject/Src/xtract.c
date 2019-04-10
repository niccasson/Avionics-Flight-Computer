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
static UART_HandleTypeDef* uart;
static FlashStruct_t * flash;
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

	uart = params->uart;
	flash = params->flash;


	intro(); //display help on start up
	char *cmd_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE); //command buffer

	/* As per most FreeRTOS tasks, this task is implemented in an infinite loop. */
	while(1){
		transmit(uart, ">> ");
		cmd_buf = receive_command(uart); //puts input into buffrx
		handle_command(cmd_buf); //handles command sitting in buffrx
	}
} //vTaskUART_CLI END

void handle_command(char* command){
	char output[BUFFER_SIZE];

	if(strcmp(command, "help") == 0){
		help();
	}
	else if(strcmp(command, "read") == 0){
		read();
	}
	else if(strcmp(command, "start") == 0){
		//start();
	}
	else{
		sprintf(output, "Command [%s] not recognized.", command);
		transmit_line(uart, output);
	}

}

void intro(void){
	transmit_line(uart, "========== Welcome to Xtract ==========\r\n"
				"This is a command line interface tool made by the Avionics subdivison of the Rockets team.\r\n\r\n"
				"Here are some commands to get you started:");
	help();
}

void help(void){
	transmit_line(uart, "Commands:\r\n"
					"\t[help] - displays the help menu and more commands\r\n"
					"\t[read] - reads the test array to your console");
}

void read(void){
	//transmit_line(uart, "1010101001001010010100101.... haha like that, right?");

	uint8_t buffer[256*10]; 	//Read 10 pages from flash at a time;

	uint32_t bytesRead = 0;
	uint32_t currentAddress = FLASH_START_ADDRESS;

	while (bytesRead < FLASH_SIZE_BYTES){

		read_page(flash,currentAddress,buffer,256*10);
		transmit_bytes(uart,buffer,256*10);

		currentAddress += (256*10);
		currentAddress = currentAddress % FLASH_SIZE_BYTES;

		bytesRead += 256*10;
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
