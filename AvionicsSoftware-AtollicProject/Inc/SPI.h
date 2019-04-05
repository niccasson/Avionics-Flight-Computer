//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS Rocketry 2019
//
// Repository:
//  UMSATS>Avionics-2019
//
//
// File Description:
//  Header file for SPI.c.
//
// History
// 2019-02-06 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------


#ifndef SPI_H
#define SPI_H

#define SPI1_CS_PIN		GPIO_PIN_9
#define SPI1_CS_PORT	GPIOA

#define SPI2_CS_PIN		0	//PLACEHOLDER
#define SPI2_CS_PORT	0	//PLACEHOLDER

#define SPI3_CS_PIN		0	//PLACEHOLDER
#define SPI3_CS_PORT	0	//PLACEHOLDER



#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

void spi1_init(SPI_HandleTypeDef *hspi);
// Description:
//  This function initializes the SPI1 interface.
//
// Parameters:
//     hspi		       A reference used to refer to the SPI interface.

void spi2_init(SPI_HandleTypeDef *hspi); //NOT FINISHED
// Description:
//  This function initializes the SPI2 interface.
//
// Parameters:
//     hspi		       A reference used to refer to the SPI interface.

void spi3_init(SPI_HandleTypeDef *hspi);//NOT FINISHED
// Description:
//  This function initializes the SPI3 interface.
//
// Parameters:
//     hspi		       A reference used to refer to the SPI interface.

void spi_transmit(SPI_HandleTypeDef hspi, uint8_t *reg_addr, uint8_t *tx_buffer,uint16_t size, uint32_t timeout);
// Description:
//  This function transfers one or more bytes over the SPI bus.
//  It firstly sends the register address (hard coded to be a 1 byte address).
//
// Parameters:
//     hspi            A reference used to refer to the SPI interface.
//     addr_buffer     A pointer to the address to write to.
//     tx_buffer       A pointer to the bytes to send.
//     size            The number of bytes being sent.
//     timeout         The timeout value in milliseconds.


void spi_read(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t *rx_buffer,uint16_t total_size, uint32_t timeout);
// Description:
//  This function reads one or more bytes over the SPI bus, by sending the address
//  and then reading
//
// Parameters:
//     hspi            A reference used to refer to the SPI interface.
//     addr_buffer     A pointer to the address to read from.
//     rx_buffer       A pointer to where the received bytes should be stored
//     total_size      The number of bytes being sent and received. (# of bytes read + 1)
//     timeout         The timeout value in milliseconds.

#endif /* SPI_H_ */

