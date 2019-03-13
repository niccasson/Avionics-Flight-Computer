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
    uart = (UART_HandleTypeDef*) pvParameters;
    transmit_line(uart, "BMP280: SPI Initialization");
    hspi = malloc(sizeof(SPI_HandleTypeDef));
    if(!hspi){
    	transmit_line(uart, "malloc failed: sizeof(UART_HandleTypeDef)");
    }
    spi1_init(hspi);
    init_bmp280();
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

	rslt = bmp280_init(&bmp);
	print_rslt(" bmp280_init status", rslt);
	rslt = bmp280_selftest(&bmp);
	print_rslt(" bmp280_perform_self_test status", rslt);
	if(rslt == BMP280_OK){
		transmit_line(uart, "Self test success");
	}
	else{
		transmit_line(uart, "Self test failed");
	}

	return 0;
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
    if (rslt != BMP280_OK)
    {
        printf("%s\t", api_name);
        if (rslt == BMP280_E_NULL_PTR)
        {
            sprintf(buf, "%s Error [%d] : Null pointer error\r\n", api_name, rslt);
        }
        else if (rslt == BMP280_E_COMM_FAIL)
        {
            sprintf(buf, "%s Error [%d] : Bus communication failed\r\n", api_name, rslt);
        }
        else if (rslt == BMP280_E_IMPLAUS_TEMP)
        {
            sprintf(buf, "%s Error [%d] : Invalid Temperature\r\n", api_name, rslt);
        }
        else if (rslt == BMP280_E_DEV_NOT_FOUND)
        {
            sprintf(buf, "%s Error [%d] : Device not found\r\n", api_name, rslt);
        }
        else
        {
            /* For more error codes refer "*_defs.h" */
            sprintf(buf, "%s Error [%d] : Unknown error code\r\n", api_name, rslt);
        }
    }

    transmit_line(uart, buf);
}
