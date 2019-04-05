//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS Rocketry 2019
//
// File Description:
//  Functions for sending and reading over SPI.
//
// History
// 2019-02-06 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "SPI.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void spi1_init(SPI_HandleTypeDef * hspi);
void spi2_init(SPI_HandleTypeDef * hspi);
void spi3_init(SPI_HandleTypeDef * hspi);
void spi_transmit(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t *tx_buffer,uint16_t size, uint32_t timeout);
void spi_read(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t *rx_buffer,uint16_t total_size, uint32_t timeout);



//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void spi1_init(SPI_HandleTypeDef *hspi){


	  __HAL_RCC_SPI1_CLK_ENABLE();


	  hspi->Instance = SPI1;
	  hspi->Init.Mode = SPI_MODE_MASTER;
	  hspi->Init.Direction = SPI_DIRECTION_2LINES;
	  hspi->Init.DataSize = SPI_DATASIZE_8BIT;
	  hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
	  hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
	  hspi->Init.NSS = SPI_NSS_SOFT;
	  hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	  hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
	  hspi->Init.TIMode = SPI_TIMODE_DISABLE;
	  hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	  hspi->Init.CRCPolynomial = 10;
	  if (HAL_SPI_Init(hspi) != HAL_OK)
	  {
	    while(1){ } //SPI setup failed!
	  }


	  /*SPI1 GPIO Configuration
	    PA5     ------> SPI1_SCK
	    PA6     ------> SPI1_MISO
	    PA7     ------> SPI1_MOSI
	    PA9     ------> SPI1_CS
	  */
		GPIO_InitTypeDef GPIO_InitStruct = {0};

	    __HAL_RCC_GPIOA_CLK_ENABLE();

	    //Setup the SPI MOSI,MISO and SCK. These pins are fixed.
	    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	    //Setup the SPI CS. This can be any pin.
	    GPIO_InitStruct.Pin = SPI1_CS_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = 0;

	    HAL_GPIO_Init(SPI1_CS_PORT,&GPIO_InitStruct);


}

void spi2_init(SPI_HandleTypeDef *hspi){}
void spi3_init(SPI_HandleTypeDef *hspi){}

void spi_transmit(SPI_HandleTypeDef hspi, uint8_t *reg_addr, uint8_t *tx_buffer,uint16_t size, uint32_t timeout){

	HAL_StatusTypeDef stat;
	GPIO_TypeDef * port = SPI1_CS_PORT;
	uint16_t pin = 0;

    if(hspi.Instance == SPI1){
  	  port = SPI1_CS_PORT;
  	  pin = SPI1_CS_PIN;
    }
    else if (hspi.Instance == SPI2){
    	  port = SPI2_CS_PORT;
    	  pin  = SPI2_CS_PIN;
    }
    else if (hspi.Instance == SPI3){
    	  port = SPI3_CS_PORT;
    	  pin = SPI3_CS_PIN;
    }
	//Write the CS low (lock)
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);

	/* Select the slave register (**1 byte address**) first via a transmit */
	stat = HAL_SPI_Transmit(&hspi,reg_addr,1,timeout);
    while(stat != HAL_OK){}

    /* Send the tx_buffer to slave */
	stat = HAL_SPI_Transmit(&hspi,tx_buffer,size,timeout);
	while(stat != HAL_OK){}

	//Write the CS hi (release)
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET);
}

void spi_read(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t *rx_buffer,uint16_t total_size, uint32_t timeout){

	GPIO_TypeDef * port = SPI1_CS_PORT;
	uint16_t pin = 0;
	HAL_StatusTypeDef stat;

    if(hspi.Instance == SPI1){
  	  port = SPI1_CS_PORT;
  	  pin = SPI1_CS_PIN;
    }
    else if (hspi.Instance == SPI2){
    	  port = SPI2_CS_PORT;
    	  pin  = SPI2_CS_PIN;
    }
    else if (hspi.Instance == SPI3){
    	  port = SPI3_CS_PORT;
    	  pin = SPI3_CS_PIN;
    }
	//Write the CS low
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);

	//Could also use HAL_TransmittReceive.

	//Send the address to read from.
	stat = HAL_SPI_Transmit(&hspi,addr_buffer,1,timeout);
	while(stat != HAL_OK){}

	//Read in the specified number of bytes.
	stat = HAL_SPI_Receive(&hspi,rx_buffer,total_size-1,timeout);
	while(stat != HAL_OK){}
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET);

}

