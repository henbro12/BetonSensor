
#ifndef _timer_H__
#define _timer_H__


/** 
 * @brief Function for initializing the timer interrupt
 */
void timer_init(void);

/** 
 * @brief Function for starting the timer
 * 
 * @param[in] timeout           Timeout for the interrupt to fire [ms]
 */
void timer_start(uint32_t timeout);

/** 
 * @brief Function for stopping the timer
 */
void timer_stop(void);

/** 
 * @brief Function for getting the interrupt flag
 * 
 * @param[out] bool             Boolean indicating the interupt flag state
 */
bool timer_getIntFlag(void);

/** 
 * @brief Function for setting the interrupt flag
 * 
 * @param[in] timerIntFlag      Boolean indicating the interupt flag state
 */
void timer_setIntFlag(bool timerIntFlag);


#endif // _timer_H__