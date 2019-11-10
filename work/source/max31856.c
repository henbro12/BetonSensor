
#include "max31856.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/** Member to hold the SPI instance */
static const nrf_drv_spi_t *m_spi;


/** 
 * @brief Function to wait for DRDY pin to assert, indicating conversion is completed
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
static max31856_status max31856_waitForDRDY() 
{
    uint8_t counter = 0;
    while (nrf_gpio_pin_read(DRDY) && counter < 50) 
    {
        nrf_delay_ms(10);
        counter++; 
    }

    return (counter >= 50) ? MAX31856_ERROR_DRDY : MAX31856_SUCCESS;
}


/** 
 * @brief Function to start a conversion and determine the themocouple temperature
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
static max31856_status max31856_startConversion()
{
    const uint8_t oneShot = CR0 | CR0_ONESHOT;
    const uint8_t tx_buffer[] = { WREGISTER_CR0, oneShot };
    static uint8_t rx_buffer[sizeof(tx_buffer)];
    
    bool success = spi_transfer(m_spi, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer));
    return success ? max31856_waitForDRDY() : MAX31856_ERROR_SPI;
}


/** 
 * @brief Function to set the MAX31856 registers
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
static max31856_status max31856_setRegisters()
{
    // { Write Register, Value }
    static uint8_t tx_buffer[][2] = 
    { 
        { WREGISTER_CR0,    CR0     },
        { WREGISTER_CR1,    CR1     }, 
        { WREGISTER_MASK,   MASK    },
        { WREGISTER_CJHF,   CJHF    },
        { WREGISTER_CJLF,   CJLF    },
        { WREGISTER_LTHFTH, LTHFTH  },
        { WREGISTER_LTHFTL, LTHFTL  },
        { WREGISTER_LTLFTH, LTLFTH  },
        { WREGISTER_LTLFTL, LTLFTL  },
        { WREGISTER_CJTO,   CJTO    }
    };

    bool success = true;
    for (int i = 0; i < (sizeof(tx_buffer) / sizeof(tx_buffer[0])); i++) 
    {
        static uint8_t rx_buffer[sizeof(tx_buffer[i])];
        success &= spi_transfer(m_spi, tx_buffer[i], sizeof(tx_buffer[i]), rx_buffer, sizeof(rx_buffer));
    }
    
    return success ? MAX31856_SUCCESS : MAX31856_ERROR_SPI;
}


/** 
 * @brief Function to check the MAX31856 registers
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
static max31856_status max31856_checkRegisters()
{
    // { Read Register, Expected Value }
    static uint8_t tx_buffer[][2] = 
    { 
        { RREGISTER_CR0,    CR0     },
        { RREGISTER_CR1,    CR1     }, 
        { RREGISTER_MASK,   MASK    },
        { RREGISTER_CJHF,   CJHF    },
        { RREGISTER_CJLF,   CJLF    },
        { RREGISTER_LTHFTH, LTHFTH  },
        { RREGISTER_LTHFTL, LTHFTL  },
        { RREGISTER_LTLFTH, LTLFTH  },
        { RREGISTER_LTLFTL, LTLFTL  },
        { RREGISTER_CJTO,   CJTO    }
    };

    bool success = true;
    for (int i = 0; i < (sizeof(tx_buffer) / sizeof(tx_buffer[0])); i++) 
    {
        static uint8_t rx_buffer[sizeof(tx_buffer[i])];
        success &= spi_transfer(m_spi, tx_buffer[i], sizeof(tx_buffer[i]) - 1, rx_buffer, sizeof(rx_buffer));

        success &= (tx_buffer[i][1] == rx_buffer[1]);
    }
    
    return success ? MAX31856_SUCCESS : MAX31856_ERROR_SPI;
}


/** 
 * @brief Function for initializing the MAX31856
 * 
 * @param[in] spi_instance          Instance of the spi interface to use
 * @param[out] max31856_status      Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_init(const nrf_drv_spi_t *const spi_instance)
{
    m_spi = spi_instance;
    max31856_status status = MAX31856_SUCCESS;

    status |= max31856_setRegisters(m_spi);
    status |= max31856_checkRegisters(m_spi);

    if (status != MAX31856_SUCCESS)
    {
        NRF_LOG_ERROR("MAX31856 ERROR: Failed to initialize\r\n");
        return status;
    }

    NRF_LOG_INFO("MAX31856 initialized\r\n");
    return status;
}


/** 
 * @brief Function to check the FAULT status registers
 * 
 * @param[out] fault_status     Error code to determine the FAULT if any
 */
fault_status max31856_checkFaultStatus()
{
    const uint8_t tx_buffer[] = { RREGISTER_SR };
    static uint8_t rx_buffer[sizeof(tx_buffer) + 1];

    bool success = spi_transfer(m_spi, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer));

    fault_status fault = APPROVED;
    if (success && (rx_buffer[1] != 0x00))
    {
        for (int bit = 0; bit < CHAR_BIT; bit++)
        {
            uint8_t bit_value = rx_buffer[1] & (1 << bit);
            if (bit_value > 0x00)
            {
                fault = bit;
                max31856_printFaultStatus(fault);
            }
        }    
    }
    else if (!success)
    {
        fault = SPI;
    }
    
    return fault;
}


/** 
 * @brief Function to reset the FAULT status registers
 * 
 * @param[out] max31856_status      Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_resetFaultStatus()
{
    const uint8_t fault_reset = CR0 | CR0_FAULTCLR;
    const uint8_t tx_buffer[] = { WREGISTER_CR0, fault_reset };
    static uint8_t rx_buffer[sizeof(tx_buffer)];

    bool success = spi_transfer(m_spi, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer));
    return success ? MAX31856_SUCCESS : MAX31856_ERROR_SPI;
}


/** 
 * @brief Function to print the FAULT error code
 * 
 * @param[in] fault     Fault status to be printed
 */
void max31856_printFaultStatus(fault_status fault)
{
    switch (fault)
    {
        case OPEN: NRF_LOG_ERROR("STATUS:\t Thermocouple Open-Circuit Fault"); break;
        case OVUV: NRF_LOG_ERROR("STATUS:\t Overvoltage or Undervoltage Input Fault"); break;
        case TCLOW: NRF_LOG_ERROR("STATUS:\t Thermocouple Temperature Low Fault"); break;
        case TCHIGH: NRF_LOG_ERROR("STATUS:\t Thermocouple Temperature High Fault"); break;
        case CJLOW: NRF_LOG_ERROR("STATUS:\t Cold-Junction Low Fault"); break;
        case CJHIGH: NRF_LOG_ERROR("STATUS:\t Cold-Junction High Fault"); break;
        case TCRANGE: NRF_LOG_ERROR("STATUS:\t Thermocouple Out-of-RANGE"); break;
        case CJRANGE: NRF_LOG_ERROR("STATUS:\t Cold-Junction Out-of-RANGE"); break;
        case UNKNOWN: NRF_LOG_ERROR("STATUS:\t UNKNOWN"); break;
        case APPROVED: NRF_LOG_INFO("STATUS:\t APPROVED"); break;
        default: NRF_LOG_ERROR("STATUS:\t FAULT not recognized"); break;
    }
}


/** 
 * @brief Function to read the cold junction temperature registers and convert it to degree celcius
 * 
 * @param[in] temperature   Pointer to a temperature instance for reading the cold junction value
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_getColdJunctionTemperature(float* temperature)
{
    const uint8_t tx_buffer[] = { RREGISTER_CJTH };
    static uint8_t rx_buffer[sizeof(tx_buffer) + 2];

    max31856_status status = max31856_startConversion();
    if (status == MAX31856_SUCCESS)
    {
        if (spi_transfer(m_spi, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer)))
        {
            uint16_t digitalTemperature = rx_buffer[1] << 8 | rx_buffer[2];
            *temperature = (digitalTemperature >> 2) * CJ_RESOLUTION;
            return status;
        }
        else
        {
            status = MAX31856_ERROR_SPI;
        }
        
    }

    NRF_LOG_ERROR("Failed to read Cold Junction Temperature");
    return status;
}


/** 
 * @brief Function to read the thermocouple temperature registers and convert it to degree celcius
 * 
 * @param[in] temperature   Pointer to a temperature instance for reading the thermocouple value
 * 
 * @return  Error code to determine the status of MAX31856 if any
 */
max31856_status max31856_getThermoCoupleTemperature(float* temperature)
{
    const uint8_t tx_buffer[] = { RREGISTER_LTCBH };
    static uint8_t rx_buffer[sizeof(tx_buffer) + 3];

    max31856_status status = max31856_startConversion();
    if (status == MAX31856_SUCCESS)
    {
        if (spi_transfer(m_spi, tx_buffer, sizeof(tx_buffer), rx_buffer, sizeof(rx_buffer)))
        {
            uint32_t digitalTemperature = rx_buffer[1] << 16 | rx_buffer[2] << 8 | rx_buffer[3];
            digitalTemperature |= ((digitalTemperature & 0x8000000) ? 0xFF000000 : 0x00000000);
            *temperature = (digitalTemperature >> 5) * TC_RESOLUTION;
            return status;
        }
        else
        {
            status = MAX31856_ERROR_SPI;
        }
    }

    NRF_LOG_ERROR("Failed to read Thermocouple Temperature");
    return status;
}


// TODO: Add interuptHandler for DRDY and FAULT pins
