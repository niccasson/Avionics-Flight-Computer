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
static char buf[128];
static bmp280_sensor* static_bmp280_sensor;

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
 * Brief: Configures the  static_bmp280_sensor according to parameterized filter, os_pres, and odr
 * Param:
 *   - filter: filter coefficient
 *   - os_pres: oversampling rate for pressure
 *   - odr: output data rate
 */
static uint8_t bmp280_config(uint8_t filter, uint8_t os_pres, uint8_t odr);

static void delay_ms(uint32_t period_ms);

static int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

static int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

static void print_rslt(const char api_name[], int8_t rslt);

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
int8_t init_bmp280_sensor(bmp280_sensor* bmp280_sensor_ptr){
	int8_t rslt;
	struct bmp280_dev* bmp280_ptr;
	SPI_HandleTypeDef* hspi_ptr;

	 //Initialize SPI Handler
	hspi_ptr = malloc(sizeof(SPI_HandleTypeDef));
	while(!hspi_ptr){} //Could not malloc a hspi
	spi1_init(hspi_ptr);

	//Initialize BMP280 Handler
	bmp280_ptr = malloc(sizeof(struct bmp280_dev));

	/* Set bmp280_sensor_ptr members to newly initialized handlers */
	bmp280_sensor_ptr->bmp_ptr = bmp280_ptr;
	bmp280_sensor_ptr->hspi_ptr = hspi_ptr;

	// Save static reference to bmp280_sensor_ptr for use in spi_reg_read/write wrapper functions
	// The spi_reg_read/write functions have function signatures defined by the BOSCH API which they conform to.
	// Unforuntately, a reference to the SPI connection is not in that signature, so I keep a static reference to it for use in said wrapper functions.
	static_bmp280_sensor = bmp280_sensor_ptr;

	/* Map the delay function pointer with the function responsible for implementing the delay */
	bmp280_ptr->delay_ms = delay_ms;

	/* Select the interface mode as SPI */
	bmp280_ptr->intf = BMP280_SPI_INTF;
	bmp280_ptr->read = spi_reg_read;
	bmp280_ptr->write = spi_reg_write;
	bmp280_ptr->dev_id = 0;

	rslt = bmp280_init(bmp280_ptr); //bosch API initialization method

	while(rslt != BMP280_OK){} //stop if initialization failed

	rslt = bmp280_selftest(bmp280_ptr);
	while(rslt != BMP280_OK){} //spin wait if self test failed
	return rslt;
}

uint32_t bmp280_get_press(){
	int8_t rslt;
	struct bmp280_uncomp_data ucomp_data;
	uint32_t pres32;

    /* Reading the raw data from sensor */
    rslt = bmp280_get_uncomp_data(&ucomp_data, static_bmp280_sensor->bmp_ptr);

    /* Getting the compensated pressure using 32 bit precision */
    rslt = bmp280_get_comp_pres_32bit(&pres32, ucomp_data.uncomp_press, static_bmp280_sensor->bmp_ptr);

    return pres32;
}

int32_t bmp280_get_temp(){
	int8_t rslt;
	struct bmp280_uncomp_data ucomp_data;
	int32_t temp32;

	/* Reading the raw data from sensor */
	rslt = bmp280_get_uncomp_data(&ucomp_data, static_bmp280_sensor->bmp_ptr);

	/* Getting the compensated temperature using 32 bit precision */
	//result is integer (i.e. 1234 is 12.34 C)
	rslt = bmp280_get_comp_temp_32bit(&temp32, ucomp_data.uncomp_temp, static_bmp280_sensor->bmp_ptr);

	return temp32;
}

static uint8_t bmp280_config(uint8_t filter, uint8_t os_pres, uint8_t odr){
	struct bmp280_config conf;
	uint8_t rslt;

	/* Configuration */
	/* Always read the current settings before writing, especially when
	 * all the configuration is not modified */
	rslt = bmp280_get_config(&conf, static_bmp280_sensor->bmp_ptr);
	//print_rslt("bmp280_get_config status", rslt);

    /* configuring the temperature oversampling, filter coefficient and output data rate */
    /* Overwrite the desired settings */
    conf.filter = filter;

    /* Pressure oversampling */
    conf.os_pres = os_pres;

    /* Setting the output data rate */
	conf.odr = odr;

	/* lock in the changes */
	rslt = bmp280_set_config(&conf, static_bmp280_sensor->bmp_ptr);
	//print_rslt("bmp280_set_config status", rslt);

	/* Always set the power mode after setting the configuration */
	rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, static_bmp280_sensor->bmp_ptr);
    //print_rslt("bmp280_set_power_mode status", rslt);
	return rslt;
}

void vTask_pressure_sensor_280(void *pvParameters){
	int rslt;
    int32_t temp32;
    uint32_t pres32;

    uart = (UART_HandleTypeDef*) pvParameters; //Get uart for printing to console

	bmp280_sensor* bmp280_sensor_ptr = malloc(sizeof(bmp280_sensor));
	rslt = init_bmp280_sensor(bmp280_sensor_ptr);

    /* Configuration */
	rslt = bmp280_config(BMP280_FILTER_COEFF_16, BMP280_OS_4X, BMP280_ODR_1000_MS);


    while(1){
    	pres32 = bmp280_get_press(bmp280_sensor_ptr);
    	temp32 = bmp280_get_temp(bmp280_sensor_ptr);
    	sprintf(buf, "Pressure: %ld [Pa]\tTemperature: %ld [0.01 C]", pres32, temp32);
    	transmit_line(uart, buf);
    	bmp280_sensor_ptr->bmp_ptr->delay_ms(1000);
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

	spi_send(*(static_bmp280_sensor->hspi_ptr), &reg_addr, 1, reg_data, length, TIMEOUT);

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

    spi_receive( *(static_bmp280_sensor->hspi_ptr), &reg_addr, 1, reg_data, length, TIMEOUT);

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
