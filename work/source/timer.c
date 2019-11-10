
#include "boards.h"
#include "nrf_drv_gpiote.h"
#include "app_timer.h"
#include "timer.h"


APP_TIMER_DEF(m_repeated_timer_id);     /**< Handler for repeated timer used to blink LED 1. */

static volatile bool m_timer_int_flag = false;


/**
 * @brief Timeout handler for the repeated timer.
 */
static void repeated_timer_handler(void* p_context)
{
    timer_setIntFlag(true);
}


/**
 * @brief Create timers.
 */
static void create_timers(void)
{
    ret_code_t err_code;

    err_code = app_timer_create(&m_repeated_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                repeated_timer_handler);
    APP_ERROR_CHECK(err_code);
}


/** 
 * @brief Function for initializing the timer interrupt
 */
void timer_init(void)
{
    APP_ERROR_CHECK(app_timer_init());
    create_timers();
}


/** 
 * @brief Function for starting the timer
 * 
 * @param[in] timeout           Timeout for the interrupt to fire [ms]
 */
void timer_start(uint32_t timeout)
{
    APP_ERROR_CHECK(app_timer_start(m_repeated_timer_id, APP_TIMER_TICKS(timeout), NULL));
}


/** 
 * @brief Function for stopping the timer
 */
void timer_stop(void)
{
    APP_ERROR_CHECK(app_timer_stop(m_repeated_timer_id));
}


/** 
 * @brief Function for getting the interrupt flag
 * 
 * @return      Boolean indicating the interupt flag state
 */
bool timer_getIntFlag(void)
{
    return m_timer_int_flag;
}


/** 
 * @brief Function for setting the interrupt flag
 * 
 * @param[in] timerIntFlag      Boolean indicating the interupt flag state
 */
void timer_setIntFlag(bool timerIntFlag)
{
    m_timer_int_flag = timerIntFlag;
}