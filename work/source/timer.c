
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "app_timer.h"
#include "timer.h"


APP_TIMER_DEF(m_repeated_timer_id);     /**< Handler for repeated timer used to blink LED 1. */


/**
 * @brief Timeout handler for the repeated timer.
 */
static void repeated_timer_handler(void* p_context)
{
    nrf_drv_gpiote_out_toggle(LED_1);
}

/**
 * @brief Create timers.
 */
static void create_timers()
{
    ret_code_t err_code;

    err_code = app_timer_create(&m_repeated_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                repeated_timer_handler);
    APP_ERROR_CHECK(err_code);
}


void timer_init()
{
    app_timer_init();
    create_timers();
}

void timer_start()
{
    APP_ERROR_CHECK(app_timer_start(m_repeated_timer_id, APP_TIMER_TICKS(120000), NULL));
}

void timer_stop()
{
    APP_ERROR_CHECK(app_timer_stop(m_repeated_timer_id));
}