//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// UMSATS 2018-2020
//
// Repository:
//  UMSATS Google Drive: UMSATS/Guides and HowTos.../Command and Data Handling (CDH)/Coding Standards
//
// File Description:
//  Reads sensor data for accelerometer and gyroscope from the BMI088
//  On prototype flight computer:
//			+Z is out of the board (perpendicular to board surface when on a table).
//			+X is towards the recovery circuit (away from where the battery connects).
//			+Y is towards the crystal (away from the programming header).
// History
// 2019-03-29 by Benjamin Zacharias
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sensorAG.h"
#include "bmi088.h"
#include "cmsis_os.h" //used in the delay function
#include "hardwareDefs.h"
#include "main.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static struct bmi08x_dev bmi088dev = {
        .accel_id = 0,
        .gyro_id = 0,
        .intf = BMI08X_SPI_INTF, // determines if we use SPI or I2C
        .read = user_spi_read,   //a function pointer to our spi read function
        .write = user_spi_write, //a function pointer to our spi write function
        .delay_ms = delay//user_delay_milli_sec
};

static SPI_HandleTypeDef hspi; //using global variable
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
int8_t user_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
int8_t user_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
void delay(uint32_t period);

//configures the accelerometer with hard-coded specifications
int8_t accel_config(struct bmi08x_dev *bmi088dev, int8_t rslt);
//configures the gyroscope with hard-coded specifications
int8_t gyro_config(struct bmi08x_dev *bmi088dev, int8_t rslt);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_sensorAG(void *param){

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	int8_t rslt;
	UART_HandleTypeDef * uart_ptr = (UART_HandleTypeDef *)param;
	//These structures store XYZ data as a 16-bit int. For a float, add "_f" to the end of the structure name
	struct bmi08x_sensor_data accel_data;
	//struct bmi08x_sensor_data gyro_data;

	//initialize the SPI
	spi3_init(&hspi); //use the already made SPI interface

	//initialize the sensors
	rslt = bmi088_init(&bmi088dev);

	if(rslt == BMI08X_OK){

		transmit_line(uart_ptr,"ACC INIT SUCCESS!");
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	}
	//configure accelerometer
	rslt = accel_config(&bmi088dev, rslt);

	if(rslt == BMI08X_OK){

		transmit_line(uart_ptr,"ACC CONFIG SUCCESS!");
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	}
	//configure gyroscope
	//rslt = gyro_config(&bmi088dev, rslt);

	//main loop: continuously read sensor data
	while(1){
		rslt = bmi08a_get_data(&accel_data, &bmi088dev);
		//rslt = bmi08g_get_data(&gyro_data, &bmi088dev);
		char data_str[30];
		snprintf(data_str,27,"x: %d y: %d z: %d",accel_data.x,accel_data.y,accel_data.z);
		transmit_line(uart_ptr,data_str);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}


//set the accelerometer starting configurations
int8_t accel_config(struct bmi08x_dev *dev, int8_t rslt){
	uint8_t data = 0;

	//not sure if necessary
	if(rslt == BMI08X_OK)
	{
		/* Read accel chip id */
		rslt = bmi08a_get_regs(BMI08X_ACCEL_CHIP_ID_REG, &data, 1, dev);
	}
	if (!rslt){
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_SET);
		delay(200);
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_RESET);
		delay(200);
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_SET);
		delay(200);
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_RESET);
	}

	/* Assign the desired configurations */
	//Not sure yet what configurations we want
	dev->accel_cfg.bw = BMI08X_ACCEL_BW_NORMAL;
	dev->accel_cfg.odr = BMI08X_ACCEL_ODR_100_HZ;
	dev->accel_cfg.range = BMI088_ACCEL_RANGE_12G;
	dev->accel_cfg.power = BMI08X_ACCEL_PM_ACTIVE;

	rslt = bmi08a_set_power_mode(dev);
	/* Wait for 10ms to switch between the power modes - delay taken care inside the function*/

	rslt = bmi08a_set_meas_conf(dev);

	return rslt;
}

//set the accelerometer starting configurations
int8_t gyro_config(struct bmi08x_dev *dev, int8_t rslt){
	uint8_t data = 0;

	//not sure if necessary
	if(rslt == BMI08X_OK)
	{
	    /* Read gyro chip id */
	    rslt = bmi08g_get_regs(BMI08X_GYRO_CHIP_ID_REG, &data, 1, dev);
	    bmi088dev.gyro_id = data;
	}

	//set power mode
	dev->gyro_cfg.power = BMI08X_GYRO_PM_NORMAL;
	rslt = bmi08g_set_power_mode(dev);
	/* Wait for 30ms to switch between the power modes - delay taken care inside the function*/

	/* Assign the desired configurations */
	dev->gyro_cfg.odr = BMI08X_GYRO_BW_23_ODR_200_HZ;
	dev->gyro_cfg.range = BMI08X_GYRO_RANGE_1000_DPS;
	dev->gyro_cfg.bw = BMI08X_GYRO_BW_23_ODR_200_HZ;

	rslt = bmi08g_set_meas_conf(dev);

	return rslt;
}

int8_t user_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len){
	//debug removeHAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_SET);
	spi_receive(hspi, &reg_addr,1, data, len, 10); // The register address will always be 1.
	//delay(500);
	//HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_RESET);
	//delay(500);
	return BMI08X_OK;
}
int8_t user_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len){
	spi_send(hspi, &reg_addr,1, data, len, 10);
	return BMI08X_OK;
}

void delay(uint32_t period){
	vTaskDelay(pdMS_TO_TICKS(period)); // wait for the given amount of milliseconds
}
