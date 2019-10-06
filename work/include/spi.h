
#ifndef _spi_H__
#define _spi_H__

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define SPI_SCK_PIN     29
#define SPI_MISO_PIN    2
#define SPI_MOSI_PIN    47
#define SPI_SS_PIN      31

#define SPI_IRQ_PRIORITY    6                                // 0-7 -> 0,1,4,5 reserverd by softdevice
#define SPI_ORC             0xFF                             // Over-run charachter
#define SPI_FREQUENCY       NRF_DRV_SPI_FREQ_1M              // 125K-250K-500K-1M-2M-4M-8M
#define SPI_MODE            NRF_DRV_SPI_MODE_0               // 0-high-leading|1-high-trailing|2-low-leading|3-low-trailing   
#define SPI_BIT_ORDER       NRF_DRV_SPI_BIT_ORDER_MSB_FIRST  // MSB-LSB

static volatile bool        spi_xfer_done;

void spi_event_handler(nrf_drv_spi_evt_t const* p_event,
                                         void*  p_context);

void spi_init(const nrf_drv_spi_t *const spi_instance);

bool spi_transfer(const nrf_drv_spi_t *const spi_instance, 
                  const uint8_t* p_tx_buffer, uint8_t tx_buffer_length,
                  uint8_t* p_rx_buffer, uint8_t rx_buffer_length);

#endif // _spi_H__