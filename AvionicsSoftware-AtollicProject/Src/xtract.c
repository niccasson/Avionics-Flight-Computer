#include "xtract.h"

/**
* @brief
* @param  pvParameters - the UART target
* @retval void
*/
void vTask_xtract(void *pvParameters){

	uart = (UART_HandleTypeDef*) pvParameters;

	help(); //display help on start up
	char *cmd_buf = (char*) malloc(sizeof(char) * BUFFER_SIZE); //command buffer

	/* As per most FreeRTOSD tasks, this task is implemented in an infinite loop. */
	for(;;){
		transmit(uart, ">> ");

		cmd_buf = receive_command(uart); //puts input into buffrx

		handle_command(cmd_buf); //handles command sitting in buffrx

		//transmit_line(buffrx);
	}
} //vTaskUART_CLI END

/*** Commands ****/
/** Handle the command in the buffrx */
void handle_command(char* command){
	char* output = (char*) malloc(BUFFER_SIZE * sizeof(char));

	if(strcmp(command, "help") == 0){
		help();
	}
	else if(strcmp(command, "read") == 0){
		read();
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

/**
 * @brief COMMAND: Display help menu
 * @param void
 * @return void
 */
void help(void){
	transmit_line(uart, "Commands:\r\n"
					"\t[help] - displays the help menu and more commands\r\n"
					"\t[read] - reads the test array to your console");
}


void read(void){
	transmit_line(uart, "1010101001001010010100101.... haha like that, right?");
}
