
#ifndef _MAX31856_H__
#define _MAX31856_H__

#include "spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**
 * @brief MAX31856 Read Registers.
 */
#define RREGISTER_CR0        0x00   ///< Configuration Register 0 
#define RREGISTER_CR1        0x01   ///< Configuration Register 1
#define RREGISTER_MASK       0x02   ///< Fault Mask Register
#define RREGISTER_CJHF       0x03   ///< Cold-Junction High Fault Threshold Register
#define RREGISTER_CJLF       0x04   ///< Cold-Junction Low Fault Threshold Register
#define RREGISTER_LTHFTH     0x05   ///< Linearized Temperature High Fault Threshold Register, MSB
#define RREGISTER_LTHFTL     0x06   ///< Linearized Temperature High Fault Threshold Register, lSB
#define RREGISTER_LTLFTH     0x07   ///< Linearized Temperature Low Fault Threshold Register, MSB
#define RREGISTER_LTLFTL     0x08   ///< Linearized Temperature Low Fault Threshold Register, lSB
#define RREGISTER_CJTO       0x09   ///< Cold-Junction Temperature Offset Register
#define RREGISTER_CJTH       0x0A   ///< Cold-Junction Temperature Register, MSB
#define RREGISTER_CJTL       0x0B   ///< Cold-Junction Temperature Register, lSB
#define RREGISTER_LTCBH      0x0C   ///< Linearized TC Temperature, Byte 2
#define RREGISTER_LTCBM      0x0D   ///< Linearized TC Temperature, Byte 1
#define RREGISTER_LTCBL      0x0E   ///< Linearized TC Temperature, Byte 0
#define RREGISTER_SR         0x0F   ///< Fault Status Register

/**
 * @brief MAX31856 Write Registers.
 */
#define WREGISTER_CR0        0x80   ///< Configuration Register 0
#define WREGISTER_CR1        0x81   ///< Configuration Register 1
#define WREGISTER_MASK       0x82   ///< Fault Mask Register
#define WREGISTER_CJHF       0x83   ///< Cold-Junction High Fault Threshold Register
#define WREGISTER_CJLF       0x84   ///< Cold-Junction Low Fault Threshold Register
#define WREGISTER_LTHFTH     0x85   ///< Linearized Temperature High Fault Threshold Register, MSB
#define WREGISTER_LTHFTL     0x86   ///< Linearized Temperature High Fault Threshold Register, lSB
#define WREGISTER_LTLFTH     0x87   ///< Linearized Temperature low Fault Threshold Register, MSB
#define WREGISTER_LTLFTL     0x88   ///< Linearized Temperature Low Fault Threshold Register, LSB
#define WREGISTER_CJTO       0x89   ///< Cold-Junction Temperature Offset Register
#define WREGISTER_CJTH       0x0A   ///< Cold-Junction Temperature Register, MSB
#define WREGISTER_CJTL       0x0B   ///< Cold-Junction Temperature Register, LSB

/**
 * @brief MAX31856 Register Values.
 */
#define CR0         0x15    ///< Enable open-circuit detection - 50Hz
#define CR1         0x22    ///< 4 samples averaged - TC Type J
#define MASK        0xFC    ///< FAULT output only asserting on under-overvoltage and open-circuit
#define CJHF        0x7F    ///< DEFAULT
#define CJLF        0xC0    ///< DEFAULT
#define LTHFTH      0x7F    ///< DEFAULT
#define LTHFTL      0xFF    ///< DEFAULT
#define LTLFTH      0x80    ///< DEFAULT
#define LTLFTL      0x00    ///< DEFAULT
#define CJTO        0x00    ///< DEFAULT

#define FAULTCLR    0x02    ///< Fault Status Clear Bit
#define ONESHOT     0x64    ///< One-Shot Conversion Bit

#define DRDY        0x29    ///< DRDY Pin number

#define TC_RESOLUTION   0.0078125f  ///< Termocouple Temperature Resolution

#define CHAR_BIT    __CHAR_BIT__    ///< Returns number of bits in a char


const nrf_drv_spi_t *m_spi;

typedef enum
{
    MAX31856_SUCCESS,           ///< Successful command
    MAX31856_ERROR_SPI,         ///< SPI transfer failure
    MAX31856_ERROR_STATUS,      ///< Status Fault Detected
    MAX31856_ERROR_DRDY,        ///< DRDY pin error
    MAX31856_ERROR_FAULT,       ///< FAULT pin error
    MAX31856_ERROR_UNKNOWN      ///< UNKNOWN status
} max31856_status;

typedef enum
{
    OPEN,           ///< Thermocouple Open-Circuit Fault
    OVUV,           ///< Overvoltage or Undervoltage Input Fault
    TCLOW,          ///< Thermocouple Temperature Low Fault
    TCHIGH,         ///< Thermocouple Temperature High Fault
    CJLOW,          ///< Cold-Junction Low Fault
    CJHIGH,         ///< Cold-Junction High Fault
    TCRANGE,        ///< Thermocouple Out-of-Range
    CJRANGE,        ///< Cold-Junction Out-of-Range
    SPI,            ///< SPI transfer fault
    APPROVED,       ///< No status fault detected 
    UNKNOWN         ///< An unknown error has beed asserted
} fault_status;


max31856_status max31856_init(const nrf_drv_spi_t *const spi_instance);

max31856_status max31856_setRegisters();
max31856_status max31856_checkRegisters();

max31856_status max31856_resetFaultStatus();
fault_status max31856_checkFaultStatus();
void max31856_printFaultStatus(fault_status fault);

max31856_status max31856_startConversion();
max31856_status max31856_getTemperature(float* temperature);

#endif // _MAX31856_H__