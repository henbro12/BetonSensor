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


#define SPI_INSTANCE    0
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);


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


/**
 * @brief Function for handling the MAX31856 timer interrupt
 */
static void max31856_int_handler(void)
{
    bsp_board_leds_off();
        
    max31856_checkFaultStatus();

    float coldJunctionTemperature = 0.0f;
    if (max31856_getColdJunctionTemperature(&coldJunctionTemperature) == MAX31856_SUCCESS)
    {
       NRF_LOG_INFO("Cold Junction Temperature: " NRF_LOG_FLOAT_MARKER "°C", NRF_LOG_FLOAT(coldJunctionTemperature));
    }

    nrf_delay_ms(500);

    float thermocoupleTemperature = 0.0f;
    if (max31856_getThermoCoupleTemperature(&thermocoupleTemperature) == MAX31856_SUCCESS)
    {
        NRF_LOG_INFO("Thermocouple Temperature: " NRF_LOG_FLOAT_MARKER "°C\r\n", NRF_LOG_FLOAT(thermocoupleTemperature));
    }

    bsp_board_leds_on();
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

    bsp_board_leds_on();
    NRF_LOG_FLUSH();


    APP_ERROR_CHECK(fds_storage_init());
    APP_ERROR_CHECK(fds_find_and_delete());
    APP_ERROR_CHECK(fds_write());
    while (fds_getWriteFlag() == 0);
    APP_ERROR_CHECK(fds_read());

    NRF_LOG_FLUSH();
    nrf_delay_ms(10000);

    timer_start(60000);

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
