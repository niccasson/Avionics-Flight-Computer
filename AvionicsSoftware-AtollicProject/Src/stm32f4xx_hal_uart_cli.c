#include "stm32f4xx_hal_uart_cli.h"

UART_HandleTypeDef huart;

UART_HandleTypeDef* MX_HAL_UART2_Init(void){
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

	   huart.Instance = USART2;
	   huart.Init.BaudRate = 9600;
	   huart.Init.WordLength = UART_WORDLENGTH_8B;
	   huart.Init.StopBits = UART_STOPBITS_1;
	   huart.Init.Parity = UART_PARITY_NONE;
	   huart.Init.Mode = UART_MODE_TX_RX;
	   huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	   huart.Init.OverSampling = UART_OVERSAMPLING_16;
	   if (HAL_UART_Init(&huart) != HAL_OK)
	   {
	 	  Error_Handler_UART();
	   }

	   return &huart; //main needs this to create a task
	   //this allows for the potential to manage multiple UART connections
}


uint8_t bufftx[BUFFER_SIZE] = ""; //transmit buffer
uint8_t buffrx[BUFFER_SIZE] = ""; //receive buffer




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

char* receive_command(void){
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

	return (char*) buffrx;
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler_UART(void)
{
  /* User can add his own implementation to report the HAL error return state */

}
