//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// pressure_sensor_bmp280.h
// UMSATS 2018-2020
//
// Repository:
//  UMSATS > Avionics 2019
//
// File Description:
//  Control and usage of BMP280 sensor inside of RTOS task.
//
// History
// 2019-03-04 Eric Kapilik
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <pressure_sensor_bmp280.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static UART_HandleTypeDef* uart;
SPI_HandleTypeDef* hspi;
struct bmp280_dev bmp;
static char buf[128];

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
static int init_bmp280( void );


static void delay_ms(uint32_t period_ms);
static int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
static int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
static void print_rslt(const char api_name[], int8_t rslt);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void vTask_pressure_sensor(void *pvParameters){
	delay_ms(5000); //to manually start the logic analyzer

    uart = (UART_HandleTypeDef*) pvParameters;
    transmit_line(uart, "\r\n---------------------------------------------");
    transmit_line(uart, "Pressure / Temperature Sensor Initialization:");
    transmit_line(uart, "---------------------------------------------");

    hspi = malloc(sizeof(SPI_HandleTypeDef));
    if(!hspi){
    	transmit_line(uart, "\tmalloc failed: sizeof(UART_HandleTypeDef)");
    }

	transmit(uart, "\tSPI Initialization...");
    spi1_init(hspi);
    transmit(uart, "\tComplete.\r\n"); //SPI1_INIT returns VOID -- there is no real check here

    transmit(uart, "\tBMP280 Initialization...");
    int rslt = init_bmp280();
    if(rslt == 0)
    {
    	transmit(uart, "\tComplete.\r\n");
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Description:
//  Initialize BMP280 sensor and be ready to read via SPI 4w.
//  Also performs unit self test.
//
// Returns:
//  0 if no errors.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static int init_bmp280( void ){
	//Based off of https://github.com/BoschSensortec/BMP280_driver/blob/master/examples/basic.c
	int8_t rslt;

	/* Map the delay function pointer with the function responsible for implementing the delay */
	bmp.delay_ms = delay_ms;

	/* Select the interface mode as SPI */
	bmp.intf = BMP280_SPI_INTF;
	bmp.read = spi_reg_read;
	bmp.write = spi_reg_write;
	bmp.dev_id = 0;

	rslt = bmp280_init(&bmp); //bosch API initialization method
	print_rslt("bmp280_init status", rslt);

	if(rslt != 0) //stop if initialization failed
	{
		rslt = bmp280_selftest(&bmp);
		print_rslt("bmp280_perform_self_test status", rslt);
	}

	return rslt;
}

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs such as "bmg250_soft_reset",
 *      "bmg250_set_foc", "bmg250_perform_self_test"  and so on.
 *
 *  @param[in] period_ms  : the required wait time in milliseconds.
 *  @return void.
 *
 */
static void delay_ms(uint32_t period_ms)
{
    vTaskDelay((TickType_t) period_ms);
}

/*!
 *  @brief Function for writing the sensor's registers through SPI bus.
 *
 *  @param[in] cs           : Chip select to enable the sensor.
 *  @param[in] reg_addr     : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose data has to be written.
 *  @param[in] length       : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
static int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
	int8_t rslt = 0; //assume success

	spi_transmit(*hspi, &reg_addr, reg_data, length, TIMEOUT);
	/* can do confirmation on reg_data if want to make rslt more useful.. so far we just assume spi_read worked! */

    return rslt;
}

/*!
 *  @brief Function for reading the sensor's registers through SPI bus.
 *
 *  @param[in] cs       : Chip select to enable the sensor.
 *  @param[in] reg_addr : Register address.
 *  @param[out] reg_data    : Pointer to the data buffer to store the read data.
 *  @param[in] length   : No of bytes to read.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */
static int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
	int8_t rslt = 0; //assume success

	//1 + length because spi_read needs to know that 1 byte will be used for selecting register address, and length refers to the message size
    spi_read(*hspi, &reg_addr, reg_data, 1+length, TIMEOUT);
    /* can do confirmation on reg_data if want to make rslt more useful.. so far we just assume spi_read worked! */
    return rslt;
}

/*!
 *  @brief Prints the execution status of the APIs.
 *
 *  @param[in] api_name : name of the API whose execution status has to be printed.
 *  @param[in] rslt     : error code returned by the API whose execution status has to be printed.
 *
 *  @return void.
 */
static void print_rslt(const char api_name[], int8_t rslt)
{
	/*! @name Error codes
	#define BMP280_E_NULL_PTR                    INT8_C(-1)
	#define BMP280_E_DEV_NOT_FOUND               INT8_C(-2)
	#define BMP280_E_INVALID_LEN                 INT8_C(-3)
	#define BMP280_E_COMM_FAIL                   INT8_C(-4)
	#define BMP280_E_INVALID_MODE                INT8_C(-5)
	#define BMP280_E_BOND_WIRE                   INT8_C(-6)
	#define BMP280_E_IMPLAUS_TEMP                INT8_C(-7)
	#define BMP280_E_IMPLAUS_PRESS               INT8_C(-8)
	#define BMP280_E_CAL_PARAM_RANGE             INT8_C(-9)
	#define BMP280_E_UNCOMP_TEMP_RANGE           INT8_C(-10)
	#define BMP280_E_UNCOMP_PRES_RANGE           INT8_C(-11)
	#define BMP280_E_UNCOMP_TEMP_AND_PRESS_RANGE INT8_C(-12)
	#define BMP280_E_UNCOMP_DATA_CALC            INT8_C(-13)
	#define BMP280_E_32BIT_COMP_TEMP             INT8_C(-14)
	#define BMP280_E_32BIT_COMP_PRESS            INT8_C(-15)
	#define BMP280_E_64BIT_COMP_PRESS            INT8_C(-16)
	#define BMP280_E_DOUBLE_COMP_TEMP            INT8_C(-17)
	#define BMP280_E_DOUBLE_COMP_PRESS           INT8_C(-18) */
    if (rslt != BMP280_OK)
    {
    	char error_msg[64];
        if (rslt == BMP280_E_NULL_PTR)
        {
        	sprintf(error_msg, "Null pointer error");
        }
        else if (rslt == BMP280_E_DEV_NOT_FOUND)
        {
        	sprintf(error_msg, "Device not found");
        }
        else if (rslt == BMP280_E_INVALID_LEN)
		{
        	sprintf(error_msg, "BMP280_E_INVALID_LEN");
		}
        else if (rslt == BMP280_E_COMM_FAIL)
        {
        	sprintf(error_msg, "Bus communication failed");
        }
        else if (rslt == BMP280_E_INVALID_MODE)
		{
        	sprintf(error_msg, "Invalid Mode.");
		}
        else if (rslt == BMP280_E_BOND_WIRE)
		{
        	sprintf(error_msg, "Bond wire.");
		}
        else if (rslt == BMP280_E_IMPLAUS_PRESS)
		{
        	sprintf(error_msg, "Invalid Pressure");
		}
        else if (rslt == BMP280_E_IMPLAUS_TEMP)
		{
        	sprintf(error_msg, "Invalid Temperature");
		}
        else if (rslt == BMP280_E_INVALID_MODE)
		{
        	sprintf(error_msg, "Invalid Mode");
		}
        else if (rslt == BMP280_E_CAL_PARAM_RANGE)
        {
        	sprintf(error_msg, "Calibration Parameter range invalid");
        }
        else
        {
            /* For more error codes refer "*_defs.h" */
        	sprintf(error_msg, "Unknown error code");
        }
        sprintf(buf, "\r\nERROR [%d] %s : %s\r\n", rslt, api_name, error_msg);
    }

    transmit(uart, buf);
}
