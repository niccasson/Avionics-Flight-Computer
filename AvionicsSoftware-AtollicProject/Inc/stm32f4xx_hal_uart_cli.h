#ifndef STM32F4XX_HAL_UART_CLI
#define STM32F4XX_HAL_UART_CLI

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_conf.h"
#include <string.h>
#include <stdlib.h>

#define TIMEOUT_MAX 0xFFFF
#define BUFFER_SIZE 2048

UART_HandleTypeDef* g_uart; //global variable to store pointer to UART handler struct

UART_HandleTypeDef* MX_HAL_UART2_Init(void);

void transmit(char* message);

void transmit_line(char* message);

char* receive_command(void);

void Error_Handler_UART(void );

#endif
