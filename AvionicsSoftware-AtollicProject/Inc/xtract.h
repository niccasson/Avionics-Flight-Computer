#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_conf.h"
#include <string.h>
#include <stdlib.h>
#include "stm32f4xx_hal_uart_cli.h"

UART_HandleTypeDef* uart;

void vTask_xtract(void *pvParameters);
void handle_command(char* command);
void intro(void); //display on start up
void help(void); //display help menu
void read(void); //reads test array to console

