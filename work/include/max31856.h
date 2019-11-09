
#ifndef _MAX31856_H__
#define _MAX31856_H__

#include "spi.h"


/** MAX31856 Read Registers */
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

/** MAX31856 Write Registers */
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

/** MAX31856 Register Values */
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

/** Specific bits in a register */
#define CR0_AUTOCONVERT 0x80    ///< Conversion Mode Bit
#define CR0_ONESHOT     0x40    ///< One-Shot Conversion Bit
#define CR0_FAULTCLR    0x02    ///< Fault Status Clear Bit

/** Pin numbers */
#define DRDY        0x29    ///< DRDY Pin number
#define FAULT       0x27    ///< FAULT Pin number

/** Temperature Resolutions */
#define TC_RESOLUTION   0.0078125f  ///< Termocouple Temperature Resolution
#define CJ_RESOLUTION   0.015625f   ///< Cold Junction  Temperature Resolution

/** Extra defines */
#define CHAR_BIT    __CHAR_BIT__    ///< Returns number of bits in a char


/** 
 * @brief Typedef Enum for defining the MAX31856 status
 */
typedef enum
{
    MAX31856_SUCCESS,           ///< Successful command
    MAX31856_ERROR_SPI,         ///< SPI transfer failure
    MAX31856_ERROR_STATUS,      ///< Status Fault Detected
    MAX31856_ERROR_DRDY,        ///< DRDY pin error
    MAX31856_ERROR_FAULT,       ///< FAULT pin error
    MAX31856_ERROR_UNKNOWN      ///< UNKNOWN status
} max31856_status;

/** 
 * @brief Typedef Enum for defining the MAX31856 FAULT status
 */
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


/** 
 * @brief Function for initializing the MAX31856
 * 
 * @param[in] spi_instance          Instance of the spi interface to use
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_init(const nrf_drv_spi_t *const spi_instance);


/** 
 * @brief Function to check the FAULT status registers
 * 
 * @return  Error code to determine the FAULT if any
 */
fault_status max31856_checkFaultStatus();


/** 
 * @brief Function to reset the FAULT status registers
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_resetFaultStatus();


/** 
 * @brief Function to print the FAULT error code
 * 
 * @param[in] fault                 Fault status to be printed
 */
void max31856_printFaultStatus(fault_status fault);


/** 
 * @brief Function to read the cold junction temperature registers and convert it to degree celcius
 * 
 * @param[in] temperature           Pointer to a temperature instance for reading the cold junction value
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_getColdJunctionTemperature(float* temperature);


/** 
 * @brief Function to read the thermocouple temperature registers and convert it to degree celcius
 * 
 * @param[in] temperature           Pointer to a temperature instance for reading the thermocouple value
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_getThermoCoupleTemperature(float* temperature);


#endif // _MAX31856_H__