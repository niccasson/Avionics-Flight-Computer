//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// pressure_sensor_bmp3.c
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
#include <stdlib.h>


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
static uint8_t bmp3_config(uint8_t filter, uint8_t os_pres,uint8_t os_temp, uint8_t odr);

static void delay_ms(uint32_t period_ms);

static int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

static int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

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
	spi2_init(hspi_ptr);

	//Initialize BMP3 Handler
	bmp3_ptr = malloc(sizeof(struct bmp3_dev));

	/* Set bmp3_sensor_ptr members to newly initialized handlers */
	bmp3_sensor_ptr->bmp_ptr = bmp3_ptr;
	bmp3_sensor_ptr->hspi_ptr = hspi_ptr;

	// Save static reference to bmp3_sensor_ptr for use in spi_reg_read/write wrapper functions
	// The spi_reg_read/write functions have function signatures defined by the BOSCH API which they conform to.
	// Unfortunately, a reference to the SPI connection is not in that signature, so I keep a static reference to it for use in said wrapper functions.
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

int8_t get_sensor_data(struct bmp3_dev *dev, struct bmp3_data* data)
{
    int8_t rslt;
    /* Variable used to select the sensor component */
    uint8_t sensor_comp;

    /* Sensor component selection */
    sensor_comp = BMP3_PRESS | BMP3_TEMP;
    /* Temperature and Pressure data are read and stored in the bmp3_data instance */
    rslt = bmp3_get_sensor_data(sensor_comp, data, dev);

    return rslt;
}

static uint8_t bmp3_config(uint8_t iir,uint8_t os_pres, uint8_t os_temp, uint8_t odr){
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
	dev->settings.odr_filter.iir_filter = iir;
	/* Assign the settings which needs to be set in the sensor */
	settings_sel = BMP3_PRESS_EN_SEL | BMP3_TEMP_EN_SEL | BMP3_PRESS_OS_SEL | BMP3_TEMP_OS_SEL | BMP3_ODR_SEL| BMP3_IIR_FILTER_SEL;
	rslt = bmp3_set_sensor_settings(settings_sel, dev);

	/* Set the power mode to normal mode */
	dev->settings.op_mode = BMP3_NORMAL_MODE;
	rslt = bmp3_set_op_mode(dev);

	return rslt;
}
void init_bmp(configData_t * configParams){

	bmp3_sensor* bmp3_sensor_ptr = malloc(sizeof(bmp3_sensor));
	int8_t rslt;
	rslt = init_bmp3_sensor(bmp3_sensor_ptr);
	if(rslt != 0){
		while(1){}
	}
	rslt = bmp3_config(configParams->values.iir_coef,configParams->values.pres_os, configParams->values.temp_os, configParams->values.bmp_odr);
	if(rslt != 0){
		while(1){}
	}
}

void calibrate_bmp(configData_t * configParams){

	bmp_data_struct dataStruct;
	get_sensor_data(static_bmp3_sensor->bmp_ptr, &dataStruct.data);
	configParams->values.ref_alt=0;
	configParams->values.ref_pres = (uint32_t)dataStruct.data.pressure/100;
}

void vTask_pressure_sensor_bmp3(void *pvParameters){

	PressureTaskParams * params = (PressureTaskParams *) pvParameters;
	QueueHandle_t bmp_queue = params->bmp388_queue;
	uart = params->huart;	//Get uart for printing to console
	configData_t * configParams = params->flightCompConfig;


	int8_t rslt;

	/* Variable used to store the compensated data */
	bmp_data_struct dataStruct;

	TickType_t prevTime;


	bmp3_sensor* bmp3_sensor_ptr = malloc(sizeof(bmp3_sensor));

	rslt = init_bmp3_sensor(bmp3_sensor_ptr);
	bmp3_print_rslt("init_bmp3_sensor", rslt);

    /* Configuration */
	//rslt = bmp3_config(BMP3_IIR_FILTER_COEFF_15,BMP3_OVERSAMPLING_4X, BMP3_OVERSAMPLING_4X, BMP3_ODR_50_HZ);
	rslt = bmp3_config(configParams->values.iir_coef,configParams->values.pres_os, configParams->values.temp_os, configParams->values.bmp_odr);
	if(rslt != 0){
		while(1){}
	}
	prevTime =xTaskGetTickCount();
	int i;
	for(i=0;i<3;i++){
		get_sensor_data(static_bmp3_sensor->bmp_ptr, &dataStruct.data);
		vTaskDelayUntil(&prevTime,configParams->values.data_rate);
	}

	if(!IS_IN_FLIGHT(configParams->values.flags)){
		get_sensor_data(static_bmp3_sensor->bmp_ptr, &dataStruct.data);
		configParams->values.ref_pres = dataStruct.data.pressure/100;
	}
    while(1){

    	get_sensor_data(static_bmp3_sensor->bmp_ptr, &dataStruct.data);
    	dataStruct.time_ticks = xTaskGetTickCount();

    	xQueueSend(bmp_queue,&dataStruct,1);

    	//sprintf(buf, "Pressure: %ld [Pa] at time: %d", (uint32_t)dataStruct.data.pressure,dataStruct.time_ticks);
    	//sprintf(buf, "P %d",dataStruct.time_ticks);
    	//transmit_line(uart, buf);

    	//sprintf(buf, "Temperature: %ld [0.01 C]", (int32_t)dataStruct.data.temperature);
    	//transmit_line(uart, buf);

    	vTaskDelayUntil(&prevTime,configParams->values.data_rate);
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
	//HAL_Delay(period_ms);
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

	spi_send(*(static_bmp3_sensor->hspi_ptr), &reg_addr, 1, reg_data, length, TIMEOUT);

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

    spi_receive( *(static_bmp3_sensor->hspi_ptr), &reg_addr, 1, reg_data, length, TIMEOUT);

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
void bmp3_print_rslt(const char api_name[], int8_t rslt)
{
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
        else if (rslt == BMP3_E_INVALID_ODR_OSR_SETTINGS)
		{
			sprintf(error_msg, "Invalid ODR OSR settings");
		}
        else if (rslt == BMP3_E_CMD_EXEC_FAILED)
		{
			sprintf(error_msg, "Command execution failed");
		}
        else if (rslt == BMP3_E_CONFIGURATION_ERR)
		{
			sprintf(error_msg, "Configuration error");
		}
        else if (rslt == BMP3_E_INVALID_LEN)
		{
			sprintf(error_msg, "Invalid length");
		}
        else if (rslt == BMP3_E_COMM_FAIL)
		{
			sprintf(error_msg, "Communication failure");
		}
        else if (rslt == BMP3_E_FIFO_WATERMARK_NOT_REACHED)
		{
			sprintf(error_msg, "FIFO Watermark not reached");
		}
        else
        {
            //For more error codes refer "bmp3_defs.h"
        	sprintf(error_msg, "Unknown error code");
        }
        sprintf(buf, "\r\nERROR [%d] %s : %s\r\n", rslt, api_name, error_msg);
    }

    transmit(uart, buf);
}
