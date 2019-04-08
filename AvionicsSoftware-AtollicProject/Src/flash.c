//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS Rocketry Division
//
// Repository:
//  UMSATS/Avionics-2019
//
// File Description:
//  Source file for the flash memory interface.
//
// History
// 2019-03-29 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "flash.h"



//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

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
// FUNCTION PROTOTYPES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function sets the write enable. This is needed before a
//	write status register, program or erase command.
//
// Returns:
//  Returns nothing.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void		enable_write(FlashStruct_t * flash);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void enable_write(FlashStruct_t * flash){
		uint8_t command = WE_COMMAND;

		spi_transmit(flash->hspi,&command,NULL,1,10);

}

void 	erase_sector(FlashStruct_t * flash,uint32_t address){

	enable_write(flash);
	uint8_t command = ERASE_SEC_COMMAND;
	uint8_t command_address [] = { PP_COMMAND, address & (HIGH_BYTE_MASK_24B), address & (MID_BYTE_MASK_24B), address & (LOW_BYTE_MASK_24B)};

	spi_transmit_long(flash->hspi,command_address,4,NULL,0,10);

}

uint8_t get_Status_reg(FlashStruct_t * flash){

	uint8_t command = GET_STATUS_REG_COMMAND;
	uint8_t status_reg;


	spi_read(flash->hspi,&command,&status_reg,2,10);

	return status_reg;
}

void program_page(FlashStruct_t * flash,uint32_t address,uint8_t * data_buffer,uint8_t num_bytes){

	//Writes must be enabled.
	enable_write(flash);
	uint8_t command_address [] = { PP_COMMAND, address & (HIGH_BYTE_MASK_24B), address & (MID_BYTE_MASK_24B), address & (LOW_BYTE_MASK_24B)};

	spi_transmit_long(flash->hspi,command_address,4,data_buffer,num_bytes,10);

}

void 	read_page(FlashStruct_t * flash,uint32_t address,uint8_t * data_buffer,uint8_t num_bytes){

	uint8_t command_address [] = { READ_COMMAND, address & (HIGH_BYTE_MASK_24B), address & (MID_BYTE_MASK_24B), address & (LOW_BYTE_MASK_24B)};

	spi_read_long(flash->hspi,command_address,4,data_buffer,num_bytes,10);

}

FlashStatus_t		check_flash_id(FlashStruct_t * flash){

	FlashStatus_t result = FLASH_ERROR;
	uint8_t command = READ_ID_COMMAND;
	uint8_t id[3] = {0,0,0};

	uint8_t bytes_to_send = sizeof(command)+sizeof(id)/sizeof(id[0]);
	spi_read(flash->hspi,(uint8_t *)&command,id,bytes_to_send,10);

	if((id[0] == MANUFACTURER_ID) && (id[1] == DEVICE_ID_MSB) && (id[2] == DEVICE_ID_LSB) ){

		result = FLASH_OK;
	}

	return result;
}


FlashStatus_t		initialize_flash(FlashStruct_t * flash){

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	//This configures the write protect pin(Active Low).
    GPIO_InitStruct.Pin = FLASH_WP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = 0;

    HAL_GPIO_Init(FLASH_WP_PORT,&GPIO_InitStruct);

    GPIO_InitTypeDef GPIO_InitStruct2 = {0};
    //This configures the hold pin(Active Low).
    GPIO_InitStruct2.Pin = FLASH_HOLD_PIN;
    GPIO_InitStruct2.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct2.Pull = GPIO_NOPULL;
    GPIO_InitStruct2.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct2.Alternate = 0;

    HAL_GPIO_Init(FLASH_HOLD_PORT,&GPIO_InitStruct2);

    HAL_GPIO_WritePin(FLASH_WP_PORT,FLASH_WP_PIN,GPIO_PIN_SET);
    HAL_GPIO_WritePin(FLASH_HOLD_PORT,FLASH_HOLD_PIN,GPIO_PIN_SET);

	//Set up the SPI interface
	spi1_init(&(flash->hspi));

	FlashStatus_t result = FLASH_ERROR;
	result = check_flash_id(flash);

	return result;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
