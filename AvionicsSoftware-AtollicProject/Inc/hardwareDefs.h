#ifndef HARDWARE_DEF_H
#define HARDWARE_DEF_H	
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS/Avionics/2019
//
// File Description:
//  Definitions for all the pins and other hardware constants for the prototype flight computer
//
// History
// 2019-03-27 by Joseph Howarth
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//User LED(red)
#define USR_LED_PIN				GPIO_PIN_5
#define USR_LED_PORT			GPIOB

//User Pushbutton (S1)
#define USR_PB_PIN				GPIO_PIN_1
#define USR_PB_PORT				GPIOB

//Buzzer
#define BUZZER_PORT				GPIOB
#define BUZZER_PIN				GPIO_PIN_2


//User GPIO
#define USR_GPIO_P3_4_PIN		GPIO_PIN_0		//Unused GPIO on P3	header, pin closest to crystal.
#define USR_GPIO_P3_4_PORT		GPIOC

#define USR_GPIO_P3_3_PIN		GPIO_PIN_1		//Unused GPIO on P3 header, pin second closest to crystal (next to other pin).
#define USR_GPIO_P3_3_PORT		GPIOC

//UART 6
#define UART_TX_PIN				GPIO_PIN_11
#define UART_TX_PORT			GPIOA

#define UART_RX_PIN				GPIO_PIN_12
#define UART_RX_PORT			GPIOA

//Flash Memory on SPI1

#define FLASH_SPI_PORT			GPIOA

#define FLASH_SPI_SCK_PIN		GPIO_PIN_5
#define FLASH_SPI_MISO_PIN		GPIO_PIN_6
#define FLASH_SPI_MOSI_PIN		GPIO_PIN_7

#define FLASH_SPI_CS_PIN		GPIO_PIN_5
#define FLASH_SPI_CS_PORT		GPIOC

#define FLASH_WP_PIN			GPIO_PIN_0
#define FLASH_WP_PORT			GPIOB

#define	FLASH_HOLD_PIN			GPIO_PIN_4
#define FLASH_HOLD_PORT			GPIOC



//Pressure Sensor on SPI2

#define PRES_SPI_PORT			GPIOB

#define PRES_SPI_SCK_PIN		GPIO_PIN_13
#define PRES_SPI_MISO_PIN		GPIO_PIN_14
#define PRES_SPI_MOSI_PIN		GPIO_PIN_15

#define PRES_SPI_CS_PIN			GPIO_PIN_7
#define PRES_SPI_CS_PORT		GPIOC

#define	PRES_INT_PIN			GPIO_PIN_6
#define PRES_INT_PORT			GPIOC



//IMU on SPI3

#define IMU_SPI_PORT			GPIOC

#define IMU_SPI_SCK_PIN			GPIO_PIN_10
#define IMU_SPI_MISO_PIN		GPIO_PIN_11
#define IMU_SPI_MOSI_PIN		GPIO_PIN_12

#define IMU_SPI_ACC_CS_PIN  	GPIO_PIN_9
#define IMU_SPI_ACC_CS_PORT 	GPIOB

#define IMU_SPI_GYRO_CS_PIN  	GPIO_PIN_6
#define IMU_SPI_GYRO_CS_PORT 	GPIOB

#define IMU_ACC_INT_PIN  		GPIO_PIN_7
#define IMU_ACC_INT_PORT 		GPIOB

#define IMU_GYRO_INT_PIN  		GPIO_PIN_8
#define IMU_GYRO_INT_PORT 		GPIOB

//Recovery Circuit (driver1 -> drogue | driver2 -> main)
#define RECOV_DROGUE_ACTIVATE_PIN		GPIO_PIN_2	//Output
#define RECOV_DROGUE_ACTIVATE_PORT		GPIOA

#define RECOV_DROGUE_ENABLE_PIN			GPIO_PIN_1	//Output
#define RECOV_DROGUE_ENABLE_PORT		GPIOA

#define RECOV_DROGUE_OVERCURRENT_PIN	GPIO_PIN_0	//Input
#define RECOV_DROGUE_OVERCURRENT_PORT	GPIOA

#define RECOV_DROGUE_CONTINUITY_PIN		GPIO_PIN_3	//Input
#define RECOV_DROGUE_CONTINUITY_PORT	GPIOA


#define RECOV_MAIN_ACTIVATE_PIN			GPIO_PIN_8	//Output
#define RECOV_MAIN_ACTIVATE_PORT		GPIOA

#define RECOV_MAIN_ENABLE_PIN			GPIO_PIN_9	//Output
#define RECOV_MAIN_ENABLE_PORT			GPIOA

#define RECOV_MAIN_OVERCURRENT_PIN		GPIO_PIN_10	//Input
#define RECOV_MAIN_OVERCURRENT_PORT		GPIOA

#define RECOV_MAIN_CONTINUITY_PIN		GPIO_PIN_9	//Input
#define RECOV_MAIN_CONTINUITY_PORT		GPIOC

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
//  Enter description for public function here.
//
// Returns:
//  Enter description of return values (if any).
//-------------------------------------------------------------------------------------------------------------------------------------------------------------


#endif // HARDWARE_DEFS_H
