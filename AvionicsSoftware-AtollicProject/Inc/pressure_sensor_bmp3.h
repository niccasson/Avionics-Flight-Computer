#ifndef PRESSURE_SENSOR_BMP3_H
#define PRESSURE_SENSOR_BMP3_H
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// pressure_sensor_bmp3.h
// UMSATS 2018-2020
//
// Repository:
//  UMSATS > Avionics 2019
//
// File Description:
//  Control and usage of BMP3 sensor inside of RTOS task.
//
// History
// 2019-03-04 Eric Kapilik
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "bmp3.h"
#include "stm32f4xx_hal_uart_io.h"
#include "cmsis_os.h"
#include "task.h"
#include "SPI.h"
#include "configuration.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#define TIMEOUT 100 // milliseconds

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// ENUMS AND ENUM TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// STRUCTS AND STRUCT TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

// Keep SPI connection and BMP sensor struct together
struct bmp3_sensor_struct{
	struct bmp3_dev* bmp_ptr;
	SPI_HandleTypeDef* hspi_ptr;
};
typedef struct bmp3_sensor_struct bmp3_sensor;

//Groups a time stamp with the reading.
typedef struct {

	struct bmp3_data data;
	uint32_t time_ticks; //time of sensor reading in ticks.

} bmp_data_struct;

//Parameters for vTask_pressure_sensor_bmp3.
typedef struct{

	UART_HandleTypeDef * huart;
	QueueHandle_t	bmp388_queue;
	configData_t *flightCompConfig;

} PressureTaskParams;
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// TYPEDEFS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// CONSTANTS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTION PROTOTYPES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

void init_bmp(configData_t * configParams);

void calibrate_bmp(configData_t * configParams);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Task for testing the BMP3 sensor.
//    - initializes sensor
//    - read data & print to UART screen cycle
//
// Returns:
//  VOID
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_pressure_sensor_bmp3(void *pvParameters);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Initialize BMP3 sensor and be ready to read via SPI 4w.
//  Also performs unit self test.
// *Based off of https://github.com/BoschSensortec/BMP3_driver/blob/master/examples/basic.c
//
// Returns:
//  0 if no errors.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
int8_t init_bmp3_sensor(bmp3_sensor* bmp3_sensor_ptr);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Captures pressure and temperature reading (64 bit precision) from BMP3 sensor.
// NOTE:
//  The sensor that this function will get measurements from is the one that was passed in via init_bmp3_sensor
//
// Returns:
//  0 if success
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
int8_t get_sensor_data(struct bmp3_dev *dev, struct bmp3_data* data);

/*!
 *  @brief Prints the execution status of the APIs.
 *
 *  @param[in] api_name : name of the API whose execution status has to be printed.
 *  @param[in] rslt     : error code returned by the API whose execution status has to be printed.
 *
 *  @return void.
 */
void bmp3_print_rslt(const char api_name[], int8_t rslt);
#endif // PRESSURE_SENSOR_BMP3_H
