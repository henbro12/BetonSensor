
#include "spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static volatile bool    spi_xfer_done;


/** 
 * @brief Function for handling the SPI event
 * 
 * @param[in] p_event          Pointer to the SPI event instance
 * @param[in] p_context        Pointer to the SPI context instance
 */
static void spi_event_handler(nrf_drv_spi_evt_t const* p_event,
                                         void*  p_context)
{
    UNUSED_PARAMETER(p_event);
    UNUSED_PARAMETER(p_context);

    spi_xfer_done = true;
}


/** 
 * @brief Function for initializing the SPI interface
 * 
 * @param[out] spi_instance          Instance of the spi interface to use
 */
void spi_init(const nrf_drv_spi_t* const spi_instance)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin       = SPI_SS_PIN;
    spi_config.miso_pin     = SPI_MISO_PIN;
    spi_config.mosi_pin     = SPI_MOSI_PIN;
    spi_config.sck_pin      = SPI_SCK_PIN;
    spi_config.irq_priority = SPI_IRQ_PRIORITY;
    spi_config.orc          = SPI_ORC;
    spi_config.frequency    = SPI_FREQUENCY;
    spi_config.mode         = SPI_MODE;
    spi_config.bit_order    = SPI_BIT_ORDER;
    APP_ERROR_CHECK(nrf_drv_spi_init(spi_instance, &spi_config, spi_event_handler, NULL));

    NRF_LOG_INFO("SPI initialized\r\n");
}


/** 
 * @brief Function for transmitting and receiving data over the SPI bus
 * 
 * @param[in]  spi_instance          Instance of the spi interface to use
 * @param[in]  p_tx_buffer           Buffer for holding the message to transmit
 * @param[in]  tx_buffer_length      Length of the transmit buffer
 * @param[out] p_rx_buffer           Buffer for holding the received message
 * @param[in]  rx_buffer_length      Length of the receive buffer
 * 
 * @return      Boolean to indicate if the SPI transfer was successful
 */
bool spi_transfer(const nrf_drv_spi_t* const spi_instance, 
                  const uint8_t* p_tx_buffer, uint8_t tx_buffer_length,
                  uint8_t* p_rx_buffer, uint8_t rx_buffer_length)
{
    memset(p_rx_buffer, 0, rx_buffer_length);
    spi_xfer_done = false;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, p_tx_buffer, tx_buffer_length, 
                                         p_rx_buffer, rx_buffer_length));

    while (!spi_xfer_done)
    {
        __WFE();
    }

    return spi_xfer_done;
}

