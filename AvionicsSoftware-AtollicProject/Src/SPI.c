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

void spi_send(SPI_HandleTypeDef hspi, uint8_t *reg_addr,uint8_t reg_addr_size, uint8_t *tx_buffer, uint16_t tx_buffer_size, uint32_t timeout);
void spi_receive(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t addr_buffer_size,uint8_t *rx_buffer,uint16_t rx_buffer_size, uint32_t timeout);

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

	    __HAL_RCC_GPIOC_CLK_ENABLE();

	    //Setup the SPI MOSI,MISO and SCK. These pins are fixed.
	    GPIO_InitStruct.Pin = FLASH_SPI_SCK_PIN|FLASH_SPI_MOSI_PIN|FLASH_SPI_MISO_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	    HAL_GPIO_Init(FLASH_SPI_PORT, &GPIO_InitStruct);

	    //Setup the SPI CS. This can be any pin.
	    GPIO_InitStruct.Pin = SPI1_CS_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = 0;

	    HAL_GPIO_Init(SPI1_CS_PORT,&GPIO_InitStruct);
	    HAL_GPIO_WritePin(SPI1_CS_PORT,SPI1_CS_PIN,GPIO_PIN_SET);

}


void spi2_init(SPI_HandleTypeDef *hspi){


	  __HAL_RCC_SPI2_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  hspi->Instance = SPI2;
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


	  /*SPI2 GPIO Configuration
	    PB13     ------> SPI1_SCK
	    PB14     ------> SPI1_MISO
	    PB15     ------> SPI1_MOSI
	    PC7     ------> SPI1_CS
	  */
		GPIO_InitTypeDef GPIO_InitStruct = {0};

	    __HAL_RCC_GPIOC_CLK_ENABLE();

	    //Setup the SPI MOSI,MISO and SCK. These pins are fixed.
	    GPIO_InitStruct.Pin = PRES_SPI_SCK_PIN|PRES_SPI_MOSI_PIN|PRES_SPI_MISO_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	    HAL_GPIO_Init(PRES_SPI_PORT, &GPIO_InitStruct);

	    //Setup the SPI CS. This can be any pin.
	    GPIO_InitStruct.Pin = SPI2_CS_PIN;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = 0;

	    HAL_GPIO_Init(SPI2_CS_PORT,&GPIO_InitStruct);
}
void spi3_init(SPI_HandleTypeDef *hspi){

	  __HAL_RCC_SPI3_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();


	  hspi->Instance = SPI3;
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



	  /*SPI3 GPIO Configuration
	    PC10     ------> SPI1_SCK
	    PC11     ------> SPI1_MISO
	    PC12     ------> SPI1_MOSI


	  */
		GPIO_InitTypeDef GPIO_InitStruct = {0};

	    __HAL_RCC_GPIOC_CLK_ENABLE();
	    __HAL_RCC_GPIOB_CLK_ENABLE();

	    //Setup the SPI MOSI,MISO and SCK. These pins are fixed.

	    GPIO_InitStruct.Pin = IMU_SPI_SCK_PIN|IMU_SPI_MOSI_PIN|IMU_SPI_MISO_PIN;

	    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;

	    HAL_GPIO_Init(IMU_SPI_PORT, &GPIO_InitStruct);

	    GPIO_InitTypeDef GPIO_InitStruct2 = {0};
	    //Setup the SPI CS. This can be any pin.
	    GPIO_InitStruct2.Pin = SPI3_CS1_PIN|SPI3_CS2_PIN;

	    GPIO_InitStruct2.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct2.Pull = GPIO_NOPULL;
	    GPIO_InitStruct2.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    GPIO_InitStruct2.Alternate = 0;

	    HAL_GPIO_Init(SPI3_CS1_PORT,&GPIO_InitStruct2);

	    //Make sure both CS are high. Undefined behavior if these are not high be for communication begins.
	    HAL_GPIO_WritePin(SPI3_CS1_PORT,SPI3_CS1_PIN,GPIO_PIN_SET);
	    HAL_GPIO_WritePin(SPI3_CS2_PORT,SPI3_CS2_PIN,GPIO_PIN_SET);

}

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
    	  port = SPI3_CS1_PORT;
    	  pin = SPI3_CS1_PIN;
    }
	//Write the CS low (lock)
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);

	/* Select the slave register (**1 byte address**) first via a transmit */
	stat = HAL_SPI_Transmit(&hspi,reg_addr,1,timeout);
    while(stat != HAL_OK){stat = HAL_SPI_GetState(&hspi);}
    /* Send the tx_buffer to slave */

    if(size>1){
		stat = HAL_SPI_Transmit(&hspi,tx_buffer,size,timeout);
		while(stat != HAL_OK){stat = HAL_SPI_GetState(&hspi);}
    }

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
    	  port = SPI3_CS1_PORT;
    	  pin = SPI3_CS1_PIN;
    }
	//Write the CS low
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);

	//Could also use HAL_TransmittReceive.

	//Send the address to read from.
	stat = HAL_SPI_Transmit(&hspi,addr_buffer,1,timeout);
	while(stat != HAL_OK){
		stat = HAL_SPI_GetState(&hspi);
	}

	//Read in the specified number of bytes.
	stat = HAL_SPI_Receive(&hspi,rx_buffer,total_size-1,timeout);
	while(stat != HAL_OK){
		stat = HAL_SPI_GetState(&hspi);
	}
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET);

}



void spi_receive(SPI_HandleTypeDef hspi,uint8_t *addr_buffer,uint8_t addr_buffer_size,uint8_t *rx_buffer,uint16_t rx_buffer_size, uint32_t timeout){


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

    	if(timeout == 10){
    		port = SPI3_CS1_PORT;
    		pin = SPI3_CS1_PIN;
    	}
    	else{
    		port = SPI3_CS2_PORT;
    		pin = SPI3_CS2_PIN;
    	}

    }
	//Write the CS low
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);

	//Could also use HAL_TransmittReceive.

	//Send the address to read from.
	if(addr_buffer_size>0){
		stat = HAL_SPI_Transmit(&hspi,addr_buffer,addr_buffer_size,timeout);
		while(stat != HAL_OK){
			stat = HAL_SPI_GetState(&hspi);}
	}
	//Read in the specified number of bytes.
	if(rx_buffer_size>0){
		stat = HAL_SPI_Receive(&hspi,rx_buffer,rx_buffer_size,timeout);
		while(stat != HAL_OK){
			stat = HAL_SPI_GetState(&hspi);
		}
	}
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET);

}


void spi_send(SPI_HandleTypeDef hspi, uint8_t *reg_addr,uint8_t reg_addr_size, uint8_t *tx_buffer, uint16_t tx_buffer_size, uint32_t timeout){


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

    	if(timeout == 10){
    		port = SPI3_CS1_PORT;
    		pin = SPI3_CS1_PIN;
    	}
    	else{
    		port = SPI3_CS2_PORT;
    		pin = SPI3_CS2_PIN;
    	}
    }
	//Write the CS low (lock)
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_RESET);

	/* Select the slave register (**1 byte address**) first via a transmit */
	if(reg_addr_size>0){
		stat = HAL_SPI_Transmit(&hspi,reg_addr,reg_addr_size,timeout);
		while(stat != HAL_OK){
			stat = HAL_SPI_GetState(&hspi);
		}
	}
    /* Send the tx_buffer to slave */
	if(tx_buffer_size>0){
		stat = HAL_SPI_Transmit(&hspi,tx_buffer,tx_buffer_size,timeout);
		while(stat != HAL_OK){
			stat = HAL_SPI_GetState(&hspi);
		}
    }
	//Write the CS hi (release)
	HAL_GPIO_WritePin(port,pin,GPIO_PIN_SET);

}

