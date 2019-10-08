
#include "spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static uint8_t*         p_buffer;
static volatile bool    spi_xfer_done;


void spi_event_handler(nrf_drv_spi_evt_t const* p_event,
                                         void*  p_context)
{
    spi_xfer_done = true;

    if (&p_buffer[0] != 0)
    {
        NRF_LOG_INFO("Received: ");
        NRF_LOG_HEXDUMP_INFO(p_buffer, strlen((const char*)p_buffer));
    }
}

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


bool spi_transfer(const nrf_drv_spi_t* const spi_instance, 
                  const uint8_t* p_tx_buffer, uint8_t tx_buffer_length,
                  uint8_t* p_rx_buffer, uint8_t rx_buffer_length)
{
    memcpy(p_buffer, p_rx_buffer, rx_buffer_length);
    memset(p_buffer, 0, rx_buffer_length);
    spi_xfer_done = false;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, p_tx_buffer, tx_buffer_length, 
                                         p_buffer, rx_buffer_length));

    while (!spi_xfer_done)
    {
        __WFE();
    }

    return spi_xfer_done;
}

