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
#include "sensorAG.h"


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
struct bmi08x_dev bmi088dev = {
        .accel_id = 0,
        .gyro_id = 1,
        .intf = BMI08X_SPI_INTF, // determines if we use SPI or I2C
        .read = user_spi_read,   //a function pointer to our spi read function
        .write = user_spi_write, //a function pointer to our spi write function
        .delay_ms = delay//user_delay_milli_sec
};

 SPI_HandleTypeDef hspi; //using global variable
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
//int8_t user_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
//int8_t user_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
//
//void delay(uint32_t period);
//
////configures the accelerometer with hard-coded specifications
//int8_t accel_config(struct bmi08x_dev *bmi088dev, int8_t rslt);
//
////configures the gyroscope with hard-coded specifications
//int8_t gyro_config(struct bmi08x_dev *bmi088dev, int8_t rslt);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_sensorAG(void *param){

	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);

	int8_t rslt;

	//Get the parameters.
	ImuTaskStruct * params = (ImuTaskStruct *)param;
	QueueHandle_t  queue = params->imu_queue;
	UART_HandleTypeDef * uart_ptr = params->huart;

	configData_t * configParams = params->flightCompConfig;


	TickType_t prevTime;

	imu_data_struct dataStruct;

	//initialize the SPI
	spi3_init(&hspi); //use the already made SPI interface

	//initialize the sensors
	rslt = bmi088_init(&bmi088dev);

	if(rslt == BMI08X_OK){

		//transmit_line(uart_ptr,"IMU INIT SUCCESS!");
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	}else{
		//transmit_line(uart_ptr,"IMU INIT FAILURE!");
	}

	//configure accelerometer
	rslt = accel_config(&bmi088dev,configParams, rslt);

	if(rslt == BMI08X_OK){

		//transmit_line(uart_ptr,"ACC CONFIG SUCCESS!");
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);
	}
	else{

		//transmit_line(uart_ptr,"ACC CONFIG FAILURE!");
	}

	//configure gyroscope
	rslt = gyro_config(&bmi088dev,configParams, rslt);
	if(rslt == BMI08X_OK){

		//transmit_line(uart_ptr,"GYRO CONFIG SUCCESS!");
		HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_SET);
	}
	else{

		//transmit_line(uart_ptr,"GYRO CONFIG FAILURE!");
	}
	HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN,GPIO_PIN_RESET);

	//main loop: continuously read sensor data
	//vTaskDelay(pdMS_TO_TICKS(100));//Wait so to make sure the other tasks have started.

	prevTime=xTaskGetTickCount();

	while(1){

		rslt = bmi08a_get_data(&dataStruct.data_acc, &bmi088dev);
		rslt = bmi08g_get_data(&dataStruct.data_gyro, &bmi088dev);
		dataStruct.time_ticks = xTaskGetTickCount();

		xQueueSend(queue,&dataStruct,1);

		//char data_str[100];
		//sprintf(data_str,"x: %d y: %d z: %d  | Rx: %d Ry: %d Rz: %d, at time %lu",dataStruct.data_acc.x,dataStruct.data_acc.y,dataStruct.data_acc.z,dataStruct.data_gyro.x,dataStruct.data_gyro.y,dataStruct.data_gyro.z,dataStruct.time_ticks);
		//sprintf(data_str,"i %d",dataStruct.time_ticks);
		//transmit_line(uart_ptr,data_str);

		vTaskDelayUntil(&prevTime,configParams->values.data_rate);
	}
}


//set the accelerometer starting configurations
int8_t accel_config(struct bmi08x_dev *dev,configData_t * configParams, int8_t rslt){
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
	dev->accel_cfg.bw = configParams->values.ac_bw;
	dev->accel_cfg.odr = configParams->values.ac_odr;
	dev->accel_cfg.range = configParams->values.ac_range;
	dev->accel_cfg.power = configParams->values.ac_pwr;

	rslt = bmi08a_set_power_mode(dev);
	/* Wait for 10ms to switch between the power modes - delay taken care inside the function*/

	rslt = bmi08a_set_meas_conf(dev);

	return rslt;
}

//set the accelerometer starting configurations
int8_t gyro_config(struct bmi08x_dev *dev,configData_t * configParams, int8_t rslt){
	uint8_t data = 0;

	//not sure if necessary
	if(rslt == BMI08X_OK)
	{
	    /* Read gyro chip id */
	    rslt = bmi08g_get_regs(BMI08X_GYRO_CHIP_ID_REG, &data, 1, dev);
	    bmi088dev.gyro_id = data;
	}

	//set power mode
	dev->gyro_cfg.power = configParams->values.gy_pwr;
	rslt = bmi08g_set_power_mode(dev);
	/* Wait for 30ms to switch between the power modes - delay taken care inside the function*/

	/* Assign the desired configurations */
	dev->gyro_cfg.odr = configParams->values.gy_odr;
	dev->gyro_cfg.range = configParams->values.gy_range;
	dev->gyro_cfg.bw = configParams->values.gy_bw;

	rslt = bmi08g_set_meas_conf(dev);

	return rslt;
}

int8_t user_spi_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len){
	//debug removeHAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_SET);

	if(dev_addr == 0x00|| dev_addr == 0x1E){
		//Accelerometer.
		spi_receive(hspi, &reg_addr,1, data, len, 10); // The register address will always be 1.

	}
	else if(dev_addr == 0x01|| dev_addr == 0x0F){
//		//Gyroscope.
		spi_receive(hspi, &reg_addr,1, data, len, 11); // The register address will always be 1.
	}
	//delay(500);
	//HAL_GPIO_WritePin(USR_LED_PORT,USR_LED_PIN, GPIO_PIN_RESET);
	//delay(500);
	return BMI08X_OK;
}
int8_t user_spi_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len){

	if(dev_addr == 0x00 || dev_addr == 0x1E){
	spi_send(hspi, &reg_addr,1, data, len, 10);
	}
	else if(dev_addr == 0x01 || dev_addr == 0x0F){
		spi_send(hspi, &reg_addr,1, data, len, 11);

	}
	return BMI08X_OK;
}

void delay(uint32_t period){
	vTaskDelay(pdMS_TO_TICKS(period)); // wait for the given amount of milliseconds
}
