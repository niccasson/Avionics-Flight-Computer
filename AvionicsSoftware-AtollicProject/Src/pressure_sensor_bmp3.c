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
// 2019-04-06 Eric Kapilik
// - Created.
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// INCLUDES
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <pressure_sensor_bmp3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// DEFINITIONS AND MACROS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
static UART_HandleTypeDef* uart;
static char buf[128];
static bmp3_sensor* static_bmp3_sensor;

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
/*
 * Brief: Configures the  static_bmp3_sensor according to parameterized filter, os_pres, and odr
 * Param:
 *   - filter: filter coefficient
 *   - os_pres: oversampling rate for pressure
 *   - odr: output data rate
 */
static uint8_t bmp3_config(uint8_t filter, uint8_t os_pres, uint8_t odr);

static void delay_ms(uint32_t period_ms);

static int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

static int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

static void print_rslt(const char api_name[], int8_t rslt);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
int8_t init_bmp3_sensor(bmp3_sensor* bmp3_sensor_ptr){
	int8_t rslt;
	struct bmp3_dev* bmp3_ptr;
	SPI_HandleTypeDef* hspi_ptr;

	 //Initialize SPI Handler
	hspi_ptr = malloc(sizeof(SPI_HandleTypeDef));
	while(!hspi_ptr){} //Could not malloc a hspi
	spi1_init(hspi_ptr);

	//Initialize BMP3 Handler
	bmp3_ptr = malloc(sizeof(struct bmp3_dev));

	/* Set bmp3_sensor_ptr members to newly initialized handlers */
	bmp3_sensor_ptr->bmp_ptr = bmp3_ptr;
	bmp3_sensor_ptr->hspi_ptr = hspi_ptr;

	// Save static reference to bmp3_sensor_ptr for use in spi_reg_read/write wrapper functions
	// The spi_reg_read/write functions have function signatures defined by the BOSCH API which they conform to.
	// Unforuntately, a reference to the SPI connection is not in that signature, so I keep a static reference to it for use in said wrapper functions.
	static_bmp3_sensor = bmp3_sensor_ptr;

	/* Map the delay function pointer with the function responsible for implementing the delay */
	bmp3_ptr->delay_ms = delay_ms;

	/* Select the interface mode as SPI */
	bmp3_ptr->intf = BMP3_SPI_INTF;
	bmp3_ptr->read = spi_reg_read;
	bmp3_ptr->write = spi_reg_write;
	bmp3_ptr->dev_id = 0;

	rslt = bmp3_init(bmp3_ptr); //bosch API initialization method

	while(rslt != BMP3_OK){} //stop if initialization failed

	return rslt;
}

uint32_t bmp3_get_press(){
	int8_t rslt;
	struct bmp3_uncomp_data ucomp_data;
	uint32_t pres32;

    /* Reading the raw data from sensor */
	//ERROR
   // rslt = bmp3_get_uncomp_data(&ucomp_data, static_bmp3_sensor->bmp_ptr);

    /* Getting the compensated pressure using 32 bit precision */
    //ERROR
    //rslt = bmp3_get_comp_pres_32bit(&pres32, ucomp_data.uncomp_press, static_bmp3_sensor->bmp_ptr);

    return pres32;
}

int32_t bmp3_get_temp(){
	int8_t rslt;
	struct bmp3_uncomp_data ucomp_data;
	int32_t temp32;

	/* Reading the raw data from sensor */
	//ERROR
	//rslt = bmp3_get_uncomp_data(&ucomp_data, static_bmp3_sensor->bmp_ptr);

	/* Getting the compensated temperature using 32 bit precision */
	//result is integer (i.e. 1234 is 12.34 C)
	//ERROR
	//rslt = bmp3_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, static_bmp3_sensor->bmp_ptr);

	return temp32;
}

static uint8_t bmp3_config(uint8_t os_pres, uint8_t os_temp, uint8_t odr){
	int8_t rslt;
	struct bmp3_dev *dev = static_bmp3_sensor->bmp_ptr;

	/* Used to select the settings user needs to change */
	uint16_t settings_sel;

	/* Select the pressure and temperature sensor to be enabled */
	dev->settings.press_en = BMP3_ENABLE;
	dev->settings.temp_en = BMP3_ENABLE;
	/* Select the output data rate and oversampling settings for pressure and temperature */
	dev->settings.odr_filter.press_os = os_pres;
	dev->settings.odr_filter.temp_os = os_temp;
	dev->settings.odr_filter.odr = odr;
	/* Assign the settings which needs to be set in the sensor */
	settings_sel = BMP3_PRESS_EN_SEL | BMP3_TEMP_EN_SEL | BMP3_PRESS_OS_SEL | BMP3_TEMP_OS_SEL | BMP3_ODR_SEL;
	rslt = bmp3_set_sensor_settings(settings_sel, dev);

	/* Set the power mode to normal mode */
	dev->settings.op_mode = BMP3_NORMAL_MODE;
	rslt = bmp3_set_op_mode(dev);

	return rslt;
}

void vTask_pressure_sensor_bmp3(void *pvParameters){
	int rslt;
    int32_t temp32;
    uint32_t pres32;

    uart = (UART_HandleTypeDef*) pvParameters; //Get uart for printing to console

	bmp3_sensor* bmp3_sensor_ptr = malloc(sizeof(bmp3_sensor));
	rslt = init_bmp3_sensor(bmp3_sensor_ptr);

    /* Configuration */
	//ERROR
	//rslt = bmp3_config(BMP3_FILTER_COEFF_16, BMP3_OS_4X, BMP3_ODR_1000_MS);


    while(1){
    	pres32 = bmp3_get_press(bmp3_sensor_ptr);
    	temp32 = bmp3_get_temp(bmp3_sensor_ptr);
    	sprintf(buf, "Pressure: %ld [Pa]\tTemperature: %ld [0.01 C]", pres32, temp32);
    	transmit_line(uart, buf);
    	bmp3_sensor_ptr->bmp_ptr->delay_ms(1000);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
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

	spi_transmit( *(static_bmp3_sensor->hspi_ptr), &reg_addr, reg_data, length, TIMEOUT);
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
    spi_read( *(static_bmp3_sensor->hspi_ptr), &reg_addr, reg_data, 1+length, TIMEOUT);
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
	/* name API success code
	#define BMP3_OK				INT8_C(0)

	name API error codes
	#define BMP3_E_NULL_PTR			INT8_C(-1)
	#define BMP3_E_DEV_NOT_FOUND			INT8_C(-2)
	#define BMP3_E_INVALID_ODR_OSR_SETTINGS	INT8_C(-3)
	#define BMP3_E_CMD_EXEC_FAILED		INT8_C(-4)
	#define BMP3_E_CONFIGURATION_ERR		INT8_C(-5)
	#define BMP3_E_INVALID_LEN			INT8_C(-6)
	#define BMP3_E_COMM_FAIL			INT8_C(-7)
	#define BMP3_E_FIFO_WATERMARK_NOT_REACHED	INT8_C(-8)

    if (rslt != BMP3_OK)
    {
    	char error_msg[64];
        if (rslt == BMP3_E_NULL_PTR)
        {
        	sprintf(error_msg, "Null pointer error");
        }
        else if (rslt == BMP3_E_DEV_NOT_FOUND)
        {
        	sprintf(error_msg, "Device not found");
        }
        else if (rslt == BMP3_E_INVALID_LEN)
		{
        	sprintf(error_msg, "BMP3_E_INVALID_LEN");
		}
        else if (rslt == BMP3_E_COMM_FAIL)
        {
        	sprintf(error_msg, "Bus communication failed");
        }
        else if (rslt == BMP3_E_INVALID_MODE)
		{
        	sprintf(error_msg, "Invalid Mode.");
		}
        else if (rslt == BMP3_E_BOND_WIRE)
		{
        	sprintf(error_msg, "Bond wire.");
		}
        else if (rslt == BMP3_E_IMPLAUS_PRESS)
		{
        	sprintf(error_msg, "Invalid Pressure");
		}
        else if (rslt == BMP3_E_IMPLAUS_TEMP)
		{
        	sprintf(error_msg, "Invalid Temperature");
		}
        else if (rslt == BMP3_E_INVALID_MODE)
		{
        	sprintf(error_msg, "Invalid Mode");
		}
        else if (rslt == BMP3_E_CAL_PARAM_RANGE)
        {
        	sprintf(error_msg, "Calibration Parameter range invalid");
        }
        else
        {
            //For more error codes refer "*_defs.h"
        	sprintf(error_msg, "Unknown error code");
        }
        sprintf(buf, "\r\nERROR [%d] %s : %s\r\n", rslt, api_name, error_msg);
    }
	*/
    transmit(uart, buf);
}
