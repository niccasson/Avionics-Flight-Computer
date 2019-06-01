#ifndef FLASH_H
#define FLASH_H
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS/Avionics-2019
//
// File Description:
//  Header file for the flash memory interface.
//
// History
// 2019-03-28 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "hardwareDefs.h"
#include "stm32f4xx_hal.h"
#include "SPI.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//Commands

#define		READ_ID_COMMAND			0x9F
#define 	WE_COMMAND				0x06		//Write Enable
#define		PP_COMMAND				0x02		//Page Program Command (write)
#define		READ_COMMAND			0x03
#define 	ERASE_SEC_COMMAND		0xD8
#define 	ERASE_PARAM_SEC_COMMAND 0x20
#define		GET_STATUS_REG_COMMAND	0x05
#define		BULK_ERASE_COMMAND		0x60		//Command to erase the whole device.

//Constants
#define		MANUFACTURER_ID			0x01
#define		DEVICE_ID_MSB			0x02
#define		DEVICE_ID_LSB			0x16

#define 	HIGH_BYTE_MASK_24B		0x00FF0000
#define		MID_BYTE_MASK_24B		0x0000FF00
#define 	LOW_BYTE_MASK_24B		0x000000FF

#define 	FLASH_PAGE_SIZE			256
#define 	FLASH_PARAM_SECTOR_SIZE (FLASH_PAGE_SIZE*16)
#define		FLASH_SECTOR_SIZE		(FLASH_PAGE_SIZE*64)
#define 	FLASH_START_ADDRESS		(0x00000000+FLASH_PARAM_SECTOR_SIZE)
#define		FLASH_SIZE_BYTES		(8000000-FLASH_PARAM_SECTOR_SIZE)
#define 	FLASH_PARAM_END_ADDRESS (0x0001FFFF)
#define 	FLASH_END_ADDRESS		(0x7FFFFF)

//Status Reg. Bits
#define 	P_ERR_BIT				0x06		//Programming Error Bit.
#define		E_ERR_BIT				0x05		//Erase Error Bit.
#define		WEL_BIT					0x01		//Write Enable Latch Bit.
#define		WIP_BIT					0x00		//Write In Progress Bit.

//Macros
#define		WAS_PROGRAMING_ERROR(x)	((x >> P_ERR_BIT) & 0x01)
#define		WAS_ERASE_ERROR(x)		((x >> E_ERR_BIT) & 0x01)
#define		IS_WRITE_ENABLE(x)		((x >> WEL_BIT) & 0x01)
#define		IS_DEVICE_BUSY(x)		((x >> WIP_BIT) & 0x01)

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef enum {FLASH_ERROR,FLASH_OK,FLASH_BUSY} FlashStatus_t;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct {

	SPI_HandleTypeDef hspi;


} FlashStruct_t;
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
//  This function sets up the flash memory.
//  Right now, this consists of setting up the SPI interface and
//	checking the ID of the flash. We could also check to make sure memory is not full etc.
//
// Returns:
//  Returns FLASH_OK if the setup is successful, HAL_ERROR otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t		initialize_flash(FlashStruct_t * flash);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This function reads the manufacturer and device IDs of the flash memory.
//	The values are checked against the correct values.
//
// Returns:
//  Returns FLASH_OK if the IDs match and FLASH_ERROR if they do not match.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t		check_flash_id(FlashStruct_t * flash);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This writes up to 256 bytes (one page) to a specified location in the flash memory.
//	The address should be 3 bytes long (0x000000 to 0x7FFFFF).
//	If the LSB of the address is not all 0, then data written past the page will wrap around!
//
//	If the device is busy the function exits early and returns FLASH_BUSY.
//
// Returns:
//  Returns a status. Will be FLASH_BUSY if there is another operation in progress, FLASH_OK otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t	program_page(FlashStruct_t * flash,uint32_t address,uint8_t * data_buffer,uint16_t num_bytes);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This reads from a specified location in the flash memory.
//	The whole memory array may be read using a single read command.
//
//	If the device is busy the function exits early and returns FLASH_BUSY.
//
// Returns:
//  Returns a status. Will be FLASH_BUSY if there is another operation in progress, FLASH_OK otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t 	read_page(FlashStruct_t * flash,uint32_t address,uint8_t * data_buffer,uint16_t num_bytes);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This erases a specified sector(64 kb) in the flash memory. Will take up to 2 seconds.
//	The address can be any address in the desired sector.
//
//	If the device is busy the function exits early and returns FLASH_BUSY.
//
// Returns:
//  Returns a status. Will be FLASH_BUSY if there is another operation in progress, FLASH_OK otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t 	erase_sector(FlashStruct_t * flash,uint32_t address);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This erases a specified parameter sector(4 kb) in the flash memory. Theses are located at the start(0x00000000) of the address space.
//	Will take up to 2 seconds.
//	The address can be any address in the desired sector.
//
//	If the device is busy the function exits early and returns FLASH_BUSY.
//
// Returns:
//  Returns a status. Will be FLASH_BUSY if there is another operation in progress, FLASH_OK otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t 	erase_param_sector(FlashStruct_t * flash,uint32_t address);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This erases the whole flash memory. Will take up to 128 seconds.
//
//	If the device is busy the function exits early and returns FLASH_BUSY.
//
// Returns:
//  Returns a status. Will be FLASH_BUSY if there is another operation in progress, FLASH_OK otherwise.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
FlashStatus_t 	erase_device(FlashStruct_t * flash);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This returns the status register of teh flash.
//
//
// Returns:
//  The status register value (8 bits).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t get_status_reg(FlashStruct_t * flash);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  This returns the address of the first empty page in memory.
//	Assumes continuous block of memory used.
//
// Returns:
//  The address  (32 bits).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
uint32_t scan_flash(FlashStruct_t * flash);

#endif // TEMPLATE_H
