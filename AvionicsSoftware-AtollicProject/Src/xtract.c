#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_conf.h"
#include <string.h>
#include <stdlib.h>

#define TIMEOUT_MAX 0xFFFF
#define BUFFER_SIZE 2048


uint8_t bufftx[BUFFER_SIZE] = ""; //transmit buffer
uint8_t buffrx[BUFFER_SIZE] = ""; //receive buffer


UART_HandleTypeDef* g_uart; //global variable to store pointer to UART handler struct

void transmit(char* message){
	int i;

	for(i = 0; i < strlen(message); i++){
		bufftx[i] = (uint8_t) message[i];
	}

	if(HAL_UART_Transmit(g_uart, (uint8_t*)bufftx, sizeof(uint8_t) * (i), TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
	}
}


void transmit_line(char* message){
	int i;

	for(i = 0; i < strlen(message); i++){
		bufftx[i] = (uint8_t) message[i];
	}
	bufftx[i++] = '\r';
	bufftx[i++] = '\n';
	bufftx[i++] = '\0';

	if(HAL_UART_Transmit(g_uart, (uint8_t*)bufftx, sizeof(uint8_t) * (i), TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
	}
}

void receive_command(void){
	uint8_t c; //key pressed character
	int i;


	c = '\0'; //clear out character received
	buffrx[0] = '\0'; //clear out receive buffer
	i = 0; //start at beginning of index

	while(i < BUFFER_SIZE){
		//get character (BLOCKING COMMAND)
		if (HAL_UART_Receive(g_uart, &c, 1, 0xFFFF) != HAL_OK){
			//did not receive character for some reason.
		}

		//print the character back.
		if(HAL_UART_Transmit(g_uart, &c, sizeof(c), TIMEOUT_MAX) != HAL_OK){
				//Do something meaningful here...
		}

		if(c == '\r'){ //return entered, command is complete
			break;
		}
		else if(c == 127){ //backspace is \177 or 127
			if(i > 0){ i--; } //don't let i become negative
			buffrx[i] = '\0';
		}
		else{ //add character to end of receive buffer
			buffrx[i++] = c;
		}
	}

	//put a new line for user
	c = '\n';
	if(HAL_UART_Transmit(g_uart, &c, sizeof(c), TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
			}
	buffrx[i] = '\0'; //string terminator
}

/** Handle the command in the buffrx */
void handle_command(void){
	char* output = (char*) malloc(BUFFER_SIZE * sizeof(char));


	if(strcmp((char*) buffrx, "help") == 0){
		help();
	}
	else if(strcmp((char*) buffrx, "read") == 0){
		read();
	}
	else{
		sprintf(output, "Command [%s] not recognized.", buffrx);
		transmit_line(output);
	}

}

/**
* @brief
* @param  pvParameters - the UART target
* @retval void
*/
void vTaskUART_CLI(void *pvParameters){
	g_uart = (UART_HandleTypeDef *) pvParameters;

	help(); //display help on start up

	/* As per most FreeRTOSD tasks, this task is implemented in an infinite loop. */
	for(;;){
		transmit(">> ");

		receive_command(); //puts input into buffrx

		handle_command(); //handles command sitting in buffrx

		//transmit_line(buffrx);
	}
} //vTaskUART_CLI END


/*** Commands ****/

void intro(void){
	transmit_line("========== Welcome to Xtract ==========\r\n"
				"This is a command line interface tool made by the Avionics subdivison of the Rockets team.\r\n\r\n"
				"Here are some commands to get you started:");
	help();
}

/**
 * @brief COMMAND: Display help menu
 * @param void
 * @return void
 */
void help(void){
	transmit_line("Commands:\r\n"
					"\t[help] - displays the help menu and more commands\r\n"
					"\t[read] - reads the test array to your console");
}


void read(void){
	transmit_line("1010101001001010010100101.... haha like that, right?");
}
