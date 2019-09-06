#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS Google Drive: UMSATS/Guides and HowTos.../Command and Data Handling (CDH)/Coding Standards
//
// File Description:
//  Template header file for C / C++ projects. Unused sections can be deleted.
//
// History
// 2019-05-27 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "bmi08x_defs.h"
#include "bmp3_defs.h"
#include "flash.h"


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//Defaults for the configuration options.
#define ID						0x5A

#define DATA_RATE 				50
#define INITIAL_WAIT_TIME 		10000			//in milliseconds
#define FLAGS 					0x00			//default not in flight, not recording.
#define DATA_START_ADDRESS		0x00001000		//Start writing to second page of memory.
#define DATA_END_ADDRESS		0x00001000		//Assume no saved data.

#define ACC_BANDWIDTH			BMI08X_ACCEL_BW_NORMAL
#define ACC_ODR					BMI08X_ACCEL_ODR_100_HZ
#define ACC_RANGE				BMI088_ACCEL_RANGE_12G
#define ACC_PWR					BMI08X_ACCEL_PM_ACTIVE

#define GYRO_BANDWIDTH			BMI08X_GYRO_BW_23_ODR_200_HZ
#define GYRO_ODR				BMI08X_GYRO_BW_23_ODR_200_HZ
#define GYRO_RANGE				BMI08X_GYRO_RANGE_1000_DPS
#define GYRO_PWR				BMI08X_GYRO_PM_NORMAL

#define BMP_ODR					BMP3_ODR_50_HZ
#define PRES_OS					BMP3_OVERSAMPLING_4X
#define TEMP_OS					BMP3_OVERSAMPLING_4X
#define BMP_IIR					BMP3_IIR_FILTER_COEFF_15

#define GND_ALT					0
#define GND_PRES				101325


#define STATE_XTRACT					0x01
#define STATE_LAUNCHPAD					0x02
#define STATE_LAUNCHPAD_ARMED			0x03
#define STATE_IN_FLIGHT_PRE_APOGEE		0x04
#define STATE_IN_FLIGHT_POST_APOGEE		0x05
#define STATE_IN_FLIGHT_POST_MAIN		0x06
#define STATE_LANDED					0x07

//Macros to get flags.
#define IS_IN_FLIGHT(x)		((x>>0)&0x01)
#define	IS_RECORDING(x)		((x>>1)&0x01)
#define IS_PRE_DROGUE(x)	((x>>2)&0x01)
#define IS_POST_DROGUE(x)	((x>>3)&0x01)
#define IS_POST_MAIN(x)		((x>>4)&0x01)


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum{
	CONFIG_OK,
	CONFIG_ERROR
} configStatus_t;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct {

	uint8_t 	 id;
	uint32_t	 initial_time_to_wait;
	uint8_t 	 data_rate;
	uint8_t 	 flags;
	uint32_t	 start_data_address;
	uint32_t	 end_data_address;

	uint8_t		 ac_bw;
	uint8_t 	 ac_odr;
	uint8_t  	 ac_range;
	uint8_t  	 ac_pwr;

	uint8_t		 gy_bw;
	uint8_t 	 gy_odr;
	uint8_t 	 gy_range;
	uint8_t 	 gy_pwr;

	uint8_t 	 bmp_odr;
	uint8_t 	 temp_os;
	uint8_t 	 pres_os;
	uint8_t  	 iir_coef;

	float	 	 ref_alt;
	float 	 	 ref_pres;


	FlashStruct_t * flash;
	uint8_t state;



}configDataStruct_t;

typedef union{

	uint8_t  bytes[sizeof(configDataStruct_t)] ;
	configDataStruct_t values;
} configData_t;
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
//  Populates the configuration struct with the defaults listed in this header file.
//
// Returns:
//  Returns a configStatus_t with OK or ERROR.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
configStatus_t init_config(configData_t* configuration);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Reads the configuration values from flash memory..
//
// Returns:
//  Returns a configStatus_t with OK or ERROR.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
configStatus_t read_config(configData_t* configuration);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Writes the configuration values to flash memory..
//
// Returns:
//  Returns a configStatus_t with OK or ERROR.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
configStatus_t write_config(configData_t* configuration);


#endif // CONFIGURATION_H
