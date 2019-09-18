//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS>Avionics-2019
//
// File Description:
//  Source file for communicating with STM32 microchip via UART Serial Connection. Handles initialization and transmission/reception.
//
// History
// 2019-02-13 Eric Kapilik
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "main.h"
#include <stm32f4xx_hal_uart_io.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t bufftx[BUFFER_SIZE] = ""; //transmit buffer
uint8_t buffrx[BUFFER_SIZE] = ""; //receive buffer

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
// FUNCTION PROTOTYPES (static functions)
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Executed in case of UART error
//
// Parameters:
//  VOID
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static void Error_Handler_UART(void);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void MX_HAL_UART2_Init(UART_HandleTypeDef* uart){
	__HAL_RCC_USART2_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;

	// GPIO uses pins 2 & 3

	/* Setup UART2 TX Pin */
	  GPIO_InitStruct.Pin = GPIO_PIN_2; //USART_TX_Pin
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	   /* Setup UART2 RX Pin */
	   GPIO_InitStruct.Pin = GPIO_PIN_3; //USART_RX_Pin
	   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	   GPIO_InitStruct.Pull = GPIO_NOPULL;
	   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	   GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	   /* Create UART struct */
	   uart->Instance = USART2;
	   uart->Init.BaudRate = 9600;
	   uart->Init.WordLength = UART_WORDLENGTH_8B;
	   uart->Init.StopBits = UART_STOPBITS_1;
	   uart->Init.Parity = UART_PARITY_NONE;
	   uart->Init.Mode = UART_MODE_TX_RX;
	   uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	   uart->Init.OverSampling = UART_OVERSAMPLING_16;
	   if (HAL_UART_Init(uart) != HAL_OK)
	   {
	 	  Error_Handler_UART();
	   }
}

void MX_HAL_UART6_Init(UART_HandleTypeDef* uart){
	__HAL_RCC_USART6_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/* Setup UART6 TX Pin */
	  GPIO_InitStruct.Pin = UART_TX_PIN; //USART_TX_Pin
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
	  HAL_GPIO_Init(UART_TX_PORT, &GPIO_InitStruct);

	   /* Setup UART6 RX Pin */
	   GPIO_InitStruct.Pin = UART_RX_PIN; //USART_RX_Pin
	   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	   GPIO_InitStruct.Pull = GPIO_NOPULL;
	   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	   GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
	   HAL_GPIO_Init(UART_RX_PORT, &GPIO_InitStruct);

	   /* Create UART struct */
	   uart->Instance = USART6;
	   uart->Init.BaudRate = 115200;
	   uart->Init.WordLength = UART_WORDLENGTH_8B;
	   uart->Init.StopBits = UART_STOPBITS_1;
	   uart->Init.Parity = UART_PARITY_NONE;
	   uart->Init.Mode = UART_MODE_TX_RX;
	   uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	   uart->Init.OverSampling = UART_OVERSAMPLING_16;
	   if (HAL_UART_Init(uart) != HAL_OK)
	   {
	 	  Error_Handler_UART();
	   }
}

void transmit(UART_HandleTypeDef* uart, char* message){
	int i;

	for(i = 0; i < strlen(message); i++){
		bufftx[i] = (uint8_t) message[i];
	}

	if(HAL_UART_Transmit(uart, (uint8_t*)bufftx, sizeof(uint8_t) * (i), TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
	}
}

void transmit_line(UART_HandleTypeDef* uart, char* message){
	int i;

	//convert to uint8_t array
	for(i = 0; i < strlen(message); i++){
		bufftx[i] = (uint8_t) message[i];
	}
	bufftx[i++] = '\r';
	bufftx[i++] = '\n';
	bufftx[i++] = '\0';

	if(HAL_UART_Transmit(uart, (uint8_t*)bufftx, sizeof(uint8_t) * (i), TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
	}
}

void transmit_bytes(UART_HandleTypeDef* uart, uint8_t *bytes,uint16_t numBytes){

	if(HAL_UART_Transmit(uart, bytes, numBytes, TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
		while(1);
	}
}

char* receive_command(UART_HandleTypeDef* uart){
	uint8_t c; //key pressed character
	int i;

	c = '\0'; //clear out character received
	buffrx[0] = '\0'; //clear out receive buffer
	i = 0; //start at beginning of index

	while(i < BUFFER_SIZE){
		//get character (BLOCKING COMMAND)
		if (HAL_UART_Receive(uart, &c, 1, 0xFFFF) != HAL_OK){
			//did not receive character for some reason.
		}

		//print the character back.
		if(c != '\0'){

			if(HAL_UART_Transmit(uart, &c, sizeof(c), TIMEOUT_MAX) != HAL_OK){
					//Do something meaningful here...
			}

		//adjust our buffer
			if(c == '\r'){ //return entered, command is complete
				break;
			}
			else if(c == 127){ //User hits backspace, clear from buffer and display (backspace is \177 or 127)
				if(i > 0){ i--; } //don't let i become negative
				buffrx[i] = '\0';
			}
			else{ //add character to end of receive buffer
				buffrx[i++] = c;
			}

		}
	}

	//put a new line for user display
	c = '\n';
	if(HAL_UART_Transmit(uart, &c, sizeof(c), TIMEOUT_MAX) != HAL_OK){
					//handle transmission error
	}
	buffrx[i] = '\0'; //string terminator added to the end of the message

	return (char*) buffrx;
}

static void Error_Handler_UART(void)
{
  /* User can add his own implementation to report the HAL error return state */

}
