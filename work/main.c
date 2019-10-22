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
#include "timer.h"


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
 * @brief Function for resetting the output voltage to 3V3,
 * reset board afterwards for the new configuration to take effect
 */
void reset_voltage_level()
{
    if (NRF_UICR->REGOUT0 != UICR_REGOUT0_VOUT_2V4) 
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
        NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_2V4;

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
    }
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
    /* Configure board. */
    reset_voltage_level();
    lfclk_request();
    bsp_board_init(BSP_INIT_LEDS);

    bsp_board_leds_on();

    timer_init();

    timer_start();

    while (true)
    {
        __WFI();
    }
}

/**
 *@}
 **/
