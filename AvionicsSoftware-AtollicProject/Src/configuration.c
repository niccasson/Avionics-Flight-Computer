//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS Rocketry 2019
//
// Repository:
//  UMSATS/Avionics-2019
//
// File Description:
// Header file for the configuration functions.
//
// History
// 2019-05-26 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef CONFIG_H
#define CONFIG_H
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------


#include "configuration.h"

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
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Enter description for static function here.
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
configStatus_t init_config(configData_t* configuration){

	configuration->values.id = ID;

	configuration->values.initial_time_to_wait = INITIAL_WAIT_TIME;
	configuration->values.data_rate = DATA_RATE;
	configuration->values.flags = FLAGS;
	configuration->values.start_data_address = DATA_START_ADDRESS;
	configuration->values.end_data_address = DATA_END_ADDRESS;

	configuration->values.ac_bw = ACC_BANDWIDTH;
	configuration->values.ac_odr= ACC_ODR;
	configuration->values.ac_range = ACC_RANGE;
	configuration->values.ac_pwr = ACC_PWR;

	configuration->values.gy_bw = GYRO_BANDWIDTH;
	configuration->values.gy_odr = GYRO_ODR;
	configuration->values.gy_range = GYRO_RANGE;
	configuration->values.gy_pwr = GYRO_PWR;

	configuration->values.bmp_odr = BMP_ODR;
	configuration->values.temp_os = TEMP_OS;
	configuration->values.pres_os = PRES_OS;
	configuration->values.iir_coef = BMP_IIR;

	configStatus_t result = CONFIG_OK;
	return result;

}

configStatus_t read_config(configData_t* configuration){

	configStatus_t stat = CONFIG_ERROR;

	FlashStatus_t result = read_page(configuration->values.flash,0x00000000,configuration->bytes,sizeof(configData_t)-sizeof(FlashStruct_t*));

	if(result == FLASH_OK){
		stat = CONFIG_OK;
	}

	return stat;
}

configStatus_t write_config(configData_t* configuration){


	configStatus_t stat = CONFIG_ERROR;

	FlashStatus_t result = program_page(configuration->values.flash,0x00000000,configuration->bytes,sizeof(configData_t)-sizeof(FlashStruct_t*));

	if(result == FLASH_OK){
		stat = CONFIG_OK;
	}

	return stat;
}

#endif
