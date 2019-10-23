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
 * @brief Function for application main entry
 */
int main(void)
{
    nrf_delay_ms(500);

    /* Configure board. */
    lfclk_request();
    bsp_board_init(BSP_INIT_LEDS);

    log_init();
    NRF_LOG_INFO("\r\n\n\n\t*** CONCRETE SENSOR ***\r\n")

    spi_init(&spi);
    max31856_init(&spi);

    //nrf_gpio_cfg_input(DRDY, NRF_GPIO_PIN_PULLUP);

    bsp_board_leds_on();
    NRF_LOG_FLUSH();

    while(true)
    {
        max31856_checkFaultStatus();

        float coldJunctionTemperature = 0.0f;
        if (max31856_getColdJunctionTemperature(&coldJunctionTemperature) == MAX31856_SUCCESS)
        {
            NRF_LOG_INFO("Cold Junction Temperature: " NRF_LOG_FLOAT_MARKER "°C", NRF_LOG_FLOAT(coldJunctionTemperature));
        }

        float thermocoupleTemperature = 0.0f;
        if (max31856_getThermoCoupleTemperature(&thermocoupleTemperature) == MAX31856_SUCCESS)
        {
            NRF_LOG_INFO("Thermocouple Temperature: " NRF_LOG_FLOAT_MARKER "°C\r\n", NRF_LOG_FLOAT(thermocoupleTemperature));
        }

        NRF_LOG_FLUSH();
        nrf_delay_ms(5000);
        bsp_board_leds_off();
        nrf_delay_ms(500);
        bsp_board_leds_on();
    }
}


/**
 *@}
 **/
