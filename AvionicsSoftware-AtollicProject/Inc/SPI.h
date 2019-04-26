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

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include "hardwareDefs.h"


#define SPI1_CS_PIN		FLASH_SPI_CS_PIN
#define SPI1_CS_PORT	FLASH_SPI_CS_PORT

#define SPI2_CS_PIN		PRES_SPI_CS_PIN
#define SPI2_CS_PORT	PRES_SPI_CS_PORT

//Right now the timeout value is used to select between chip selects.
//We should really find a better way to do this!
//Currently only works with the send and receive functions.
//timeout=10 for acc
//timeout=other for gyro
#define SPI3_CS1_PIN	IMU_SPI_ACC_CS_PIN
#define SPI3_CS1_PORT	IMU_SPI_ACC_CS_PORT

#define SPI3_CS2_PIN	IMU_SPI_GYRO_CS_PIN
#define SPI3_CS2_PORT	IMU_SPI_GYRO_CS_PORT

// Description:
//  This function initializes the SPI1 interface.
//
// Parameters:
//     hspi		       A reference used to refer to the SPI interface.
void spi1_init(SPI_HandleTypeDef *hspi);


// Description:
//  This function initializes the SPI2 interface.
//
// Parameters:
//     hspi		       A reference used to refer to the SPI interface.
void spi2_init(SPI_HandleTypeDef *hspi);


// Description:
//  This function initializes the SPI3 interface.
//
// Parameters:
//     hspi		       A reference used to refer to the SPI interface.
void spi3_init(SPI_HandleTypeDef *hspi);




// Description:
//  This function reads one or more bytes over the SPI bus, by sending multiple address bytes
//  and then reading multiple bytes.
//
// Parameters:
//     hspi             A reference used to refer to the SPI interface.
//     addr_buffer      A pointer to the buffer holding address to read from.
//	   addr_buffer_size The number of bytes in the address/command.
//     rx_buffer        A pointer to where the received bytes should be stored
//     rx_buffer_size   The number of bytes being  received.
//     timeout          The timeout value in milliseconds.
void spi_receive(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t addr_buffer_size,uint8_t *rx_buffer,uint16_t rx_buffer_size, uint32_t timeout);


// Description:
//  This function transfers one or more bytes over the SPI bus.
//  It firstly sends multiple register address bytes.
//
// Parameters:
//     hspi            	A reference used to refer to the SPI interface.
//     addr_buffer     	A pointer to the buffer holding the address to write to.
//	   addr_buffer_size	Number of bytes in the address/command.
//     tx_buffer       	A pointer to the bytes to send.
//     size            	The number of bytes being sent.
//     timeout         	The timeout value in milliseconds.
void spi_send(SPI_HandleTypeDef hspi, uint8_t *reg_addr,uint8_t reg_addr_size, uint8_t *tx_buffer, uint16_t tx_buffer_size, uint32_t timeout);


// Description: DO NOT USE. Will be deleted in future versions of the code!
//  This function reads one or more bytes over the SPI bus, by sending the address
//  and then reading
//
// Parameters:
//     hspi            A reference used to refer to the SPI interface.
//     addr_buffer     A pointer to the address to read from.
//     rx_buffer       A pointer to where the received bytes should be stored
//     total_size      The number of bytes being sent and received. (# of bytes read + 1)
//     timeout         The timeout value in milliseconds.

void spi_read(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t *rx_buffer,uint16_t total_size, uint32_t timeout);


// Description:  DO NOT USE! Will be deleted in future versions of the code!
//  This function transfers one or more bytes over the SPI bus.
//  It firstly sends the register address (hard coded to be a 1 byte address).
//
// Parameters:
//     hspi            A reference used to refer to the SPI interface.
//     addr_buffer     A pointer to the address to write to.
//     tx_buffer       A pointer to the bytes to send.
//     size            The number of bytes being sent.
//     timeout         The timeout value in milliseconds.
void spi_transmit(SPI_HandleTypeDef hspi, uint8_t *reg_addr, uint8_t *tx_buffer,uint16_t size, uint32_t timeout);

#endif /* SPI_H_ */

