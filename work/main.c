/** @file
 *
 * @defgroup template main.c
 * @{
 * @ingroup template_henbro
 * @brief Template Application main file.
 *
 * This file contains the source code for a sample application to blink LEDs.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "sdk_errors.h"
#include "nrf_drv_clock.h"

#include "max31856.h"
#include "timer.h"
#include "storage.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define SIZE_OF_FLOAT   4

#define SPI_INSTANCE    0
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

#define FILE_ID_FDS     0x3185
#define REC_KEY_FDS     0x0001

union
{
    float float_val;
    unsigned char bytes_val[SIZE_OF_FLOAT];
} floatAsBytes;

/**
 * @brief Function starting the internal LFCLK oscillator.
 *
 * @details This is needed by RTC1 which is used by the Application Timer
 *          (When SoftDevice is enabled the LFCLK is always running and this is not needed).
 */
static void lfclk_request(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}


/**
 * @brief Function for initializing the nrf log module
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**
 * @brief Function for the Power manager
 */
// static void power_manage(void)
// {
//     ret_code_t err_code = sd_app_evt_wait();
//     APP_ERROR_CHECK(err_code);
// }


unsigned char* float2bytes(float m_float)
{
    floatAsBytes.float_val = m_float;
    return floatAsBytes.bytes_val;
}

float bytes2float(unsigned char m_bytes[SIZE_OF_FLOAT])
{
    memcpy(floatAsBytes.bytes_val, m_bytes, SIZE_OF_FLOAT);
    return floatAsBytes.float_val;
}

/**
 * @brief Function for handling the MAX31856 timer interrupt
 */
static void max31856_int_handler(void)
{
    bsp_board_leds_off();
        
    max31856_checkFaultStatus();

    float coldJunctionTemperature = 0.0f;
    unsigned char* coldJunctionTemperatureBytes;
    if (max31856_getColdJunctionTemperature(&coldJunctionTemperature) == MAX31856_SUCCESS)
    {
        NRF_LOG_INFO("Cold Junction Temperature: " NRF_LOG_FLOAT_MARKER "°C", NRF_LOG_FLOAT(coldJunctionTemperature));
        coldJunctionTemperatureBytes = float2bytes(coldJunctionTemperature);
        
        APP_ERROR_CHECK(fds_write(FILE_ID_FDS, REC_KEY_FDS, coldJunctionTemperatureBytes));
        while (!fds_getWriteFlag());
    }

    nrf_delay_ms(500);

    float thermocoupleTemperature = 0.0f;
    if (max31856_getThermoCoupleTemperature(&thermocoupleTemperature) == MAX31856_SUCCESS)
    {
        NRF_LOG_INFO("Thermocouple Temperature: " NRF_LOG_FLOAT_MARKER "°C\r\n", NRF_LOG_FLOAT(thermocoupleTemperature));
    }

    bsp_board_leds_on();
} 

void read_records(void) 
{
    static uint8_t read_data[MAX_RECORDS][RECORD_SIZE] = {0};
    APP_ERROR_CHECK(fds_read(FILE_ID_FDS, REC_KEY_FDS, read_data));

    for (int i = 0; i < fds_getNumberOfRecords(); i++)
    {
        float coldJunctionTemperature = bytes2float(read_data[i]);
        NRF_LOG_INFO("Record %d, temperature: " NRF_LOG_FLOAT_MARKER "°C", i, NRF_LOG_FLOAT(coldJunctionTemperature));
    }
    NRF_LOG_INFO("\t*** END OF RECORDS ***\r\n");
}

/**
 * @brief Function for application main entry
 */
int main(void)
{
    /* Configure board. */
    lfclk_request();
    bsp_board_init(BSP_INIT_LEDS);

    log_init();
    NRF_LOG_INFO("\r\n\n\n\t*** CONCRETE SENSOR ***\r\n")

    timer_init();
    spi_init(&spi);
    max31856_init(&spi);

    APP_ERROR_CHECK(fds_storage_init());
    //APP_ERROR_CHECK(fds_find_and_delete(FILE_ID_FDS, REC_KEY_FDS));

    bsp_board_leds_on();
    NRF_LOG_FLUSH();

    read_records();
    timer_start(600000);

    while(true)
    {
        if (timer_getIntFlag())
        {
            timer_setIntFlag(false);
            max31856_int_handler();
        }
        NRF_LOG_FLUSH();


        __WFE();
        __SEV();
        __WFE();
    }
}


/**
 *@}
 **/
