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
//	If the device is busy the function exits early and returns FLASH_BUSY.
//
// Returns:
//  Returns a status. Will be FLASH_BUSY if there is another operation in progress, FLASH_OK otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t		enable_write(FlashStruct_t * flash);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t enable_write(FlashStruct_t * flash){

	FlashStatus_t result = FLASH_ERROR;

	uint8_t status_reg = get_status_reg(flash);


	if(IS_DEVICE_BUSY(status_reg)){

		result = FLASH_BUSY;
	}
	else{

		uint8_t command = WE_COMMAND;

		spi_transmit(flash->hspi,&command,NULL,1,10);

		result = FLASH_OK;
	}
	return result;
}

FlashStatus_t 	erase_sector(FlashStruct_t * flash,uint32_t address){

	FlashStatus_t result = FLASH_ERROR;

	uint8_t status_reg = get_status_reg(flash);


	if(IS_DEVICE_BUSY(status_reg)){

		result = FLASH_BUSY;
	}
	else{

		enable_write(flash);

		uint8_t command_address [] = { ERASE_SEC_COMMAND, (address & (HIGH_BYTE_MASK_24B))>>16, (address & (MID_BYTE_MASK_24B))>>8, address & (LOW_BYTE_MASK_24B)};

		spi_send(flash->hspi,command_address,4,NULL,0,10);

		result = FLASH_OK;
	}
	return result;
}

FlashStatus_t 	erase_param_sector(FlashStruct_t * flash,uint32_t address){

	FlashStatus_t result = FLASH_ERROR;

	uint8_t status_reg = get_status_reg(flash);


	if(IS_DEVICE_BUSY(status_reg)){

		result = FLASH_BUSY;
	}
	else{

		enable_write(flash);

		uint8_t command_address [] = { ERASE_PARAM_SEC_COMMAND, (address & (HIGH_BYTE_MASK_24B))>>16, (address & (MID_BYTE_MASK_24B))>>8, address & (LOW_BYTE_MASK_24B)};

		spi_send(flash->hspi,command_address,4,NULL,0,10);

		result = FLASH_OK;
	}
	return result;
}


FlashStatus_t 	erase_device(FlashStruct_t * flash){

	FlashStatus_t result = FLASH_ERROR;

	uint8_t status_reg = get_status_reg(flash);


	if(IS_DEVICE_BUSY(status_reg)){

		result = FLASH_BUSY;
	}
	else{

		enable_write(flash);

		uint8_t command = BULK_ERASE_COMMAND;

		spi_send(flash->hspi,&command,1,NULL,0,10);

		result = FLASH_OK;
	}
	return result;
}

uint8_t get_status_reg(FlashStruct_t * flash){

	uint8_t command = GET_STATUS_REG_COMMAND;
	uint8_t status_reg;


	spi_receive(flash->hspi,&command,1,&status_reg,1,10);

	return status_reg;
}

FlashStatus_t program_page(FlashStruct_t * flash,uint32_t address,uint8_t * data_buffer,uint16_t num_bytes){

	FlashStatus_t result = FLASH_ERROR;

	uint8_t status_reg = get_status_reg(flash);


	if(IS_DEVICE_BUSY(status_reg)){

		result = FLASH_BUSY;
	}
	else{


		//Writes must be enabled.
		enable_write(flash);
		uint8_t command_address [] = { PP_COMMAND, (address & (HIGH_BYTE_MASK_24B))>>16, (address & (MID_BYTE_MASK_24B))>>8, address & (LOW_BYTE_MASK_24B)};

		spi_send(flash->hspi,command_address,4,data_buffer,num_bytes,200);
		result = FLASH_OK;
	}
	return result;
}
FlashStatus_t 	read_page(FlashStruct_t * flash,uint32_t address,uint8_t * data_buffer,uint16_t num_bytes){


	FlashStatus_t result = FLASH_ERROR;

	uint8_t status_reg = get_status_reg(flash);


	if(IS_DEVICE_BUSY(status_reg)){

		result = FLASH_BUSY;
	}
	else{

		uint8_t command_address [] = { READ_COMMAND, (address & (HIGH_BYTE_MASK_24B))>>16, (address & (MID_BYTE_MASK_24B))>>8, address & (LOW_BYTE_MASK_24B)};

		spi_receive(flash->hspi,command_address,4,data_buffer,num_bytes,200);
		result = FLASH_OK;
	}
	return result;
}

FlashStatus_t		check_flash_id(FlashStruct_t * flash){

	FlashStatus_t result = FLASH_ERROR;
	uint8_t command = READ_ID_COMMAND;
	uint8_t id[3] = {0,0,0};

	//uint8_t bytes_to_send = sizeof(command)+sizeof(id)/sizeof(id[0]);
	spi_receive(flash->hspi,(uint8_t *)&command,1,id,3,10);

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

    HAL_GPIO_WritePin(FLASH_SPI_CS_PORT,FLASH_SPI_CS_PIN,GPIO_PIN_SET);
	FlashStatus_t result = FLASH_ERROR;
	result = check_flash_id(flash);

	return result;
}

uint32_t scan_flash(FlashStruct_t * flash){


	uint32_t result = 0;

	uint8_t dataRX[256];
	uint32_t i;
	int j;
	i=FLASH_START_ADDRESS;
	while(i<FLASH_SIZE_BYTES){

		FlashStatus_t stat;

		for(j=0;j<256;j++){
			dataRX[j] = 0;
		}

		stat = read_page(flash,i,dataRX,256);

		uint16_t empty= 0xFFFF;
		for(j=0;j<256;j++){

			if(dataRX[j] != 0xFF){
				empty --;
			}
		}

		if(empty == 0xFFFF){

			result = i;
			break;
		}

		i = i + 256;
	}

	if (result == 0) result = FLASH_SIZE_BYTES; //ADDED AFTER RECOVERY!!!!
	return result;

}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
