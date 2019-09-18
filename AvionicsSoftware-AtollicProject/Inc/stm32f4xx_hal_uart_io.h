#ifndef STM32F4XX_HAL_UART_CLI_H
#define STM32F4XX_HAL_UART_CLI_H
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS>Avionics-2019
//
// File Description:
//  Header file for communicating with STM32 microchip via UART Serial Connection. Handles initialization and transmission/reception.
//
// History
// 2019-02-13 Eric Kapilik
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_conf.h"
#include "hardwareDefs.h"
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#define TIMEOUT_MAX 0xFFFF
#define BUFFER_SIZE 2048
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
// CONSTANTS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//	For UART port 2 only, this should only be run ONCE. (i.e. only one program should call it, once.)
//  Enable UART clock, initialize GPIO pins, and create UART structure required for communication.
//	Uses GPIO pins 2 & 3 of STM32F401RE
//
// NOTE: If you wish to use another UART port, please create another function to initialize that port.
//
// Parameters:
//  UART_HandleTypeDef Pointer (needed by communication functions)
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void MX_HAL_UART2_Init(UART_HandleTypeDef* uart);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//	For UART port 6 only, this should only be run ONCE. (i.e. only one program should call it, once.)
//  Enable UART clock, initialize GPIO pins, and create UART structure required for communication.
//
// NOTE: If you wish to use another UART port, please create another function to initialize that port.
//
// Parameters:
//  UART_HandleTypeDef Pointer (needed by communication functions)
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void MX_HAL_UART6_Init(UART_HandleTypeDef* uart);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Transmit message to UART port. Does not add new line to message.
//
// Parameters:
//  UART_HandleTypeDef* uart - UART port to transmit to
//  char* message - the message you wish to send, ending string with null character ('\0')
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmit(UART_HandleTypeDef* uart, char* message);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Transmit message to UART port. Adds new line characters to end of message.
//
// Parameters:
//  UART_HandleTypeDef* uart - UART port to transmit to
//  char* message - the message you wish to send, ending string with null character ('\0')
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmit_line(UART_HandleTypeDef* uart, char* message);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Transmit bytes to UART port.
//
// Parameters:
//  UART_HandleTypeDef* uart - UART port to transmit to
//  uint8_t* bytes - A pointer to the bytes you want to send.
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void transmit_bytes(UART_HandleTypeDef* uart, uint8_t *bytes, uint16_t numBytes);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//	BLOCKING FUNCTION
//  Receive message from UART port. Prints back what the user is typing so that they can see it. Their message ends when they press enter.
//
// Parameters:
//  UART_HandleTypeDef* uart - UART port to transmit to
//
// Returns:
//  Pointer to character array containing user entered message
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
char* receive_command(UART_HandleTypeDef* uart);

#endif //STM32F4XX_HAL_UART_CLI_H
