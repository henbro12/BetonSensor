
#ifndef _spi_H__
#define _spi_H__

#include "nrf_drv_spi.h"


/** SPI pin numbers */
#define SPI_SCK_PIN     29          ///< Spi-Clock pin number
#define SPI_MISO_PIN    2           ///< Master-In-Slave-Out pin number
#define SPI_MOSI_PIN    47          ///< Master-Out-Slave-In pin number
#define SPI_SS_PIN      31          ///< Spi-Select pin number

/** Extra details concerning SPI */
#define SPI_IRQ_PRIORITY    6                                ///< 0-7 -> 0,1,4,5 reserverd by softdevice
#define SPI_ORC             0xFF                             ///< Over-run charachter
#define SPI_FREQUENCY       NRF_DRV_SPI_FREQ_1M              ///< 125K-250K-500K-1M-2M-4M-8M
#define SPI_MODE            NRF_DRV_SPI_MODE_1               ///< 0-high-leading|1-high-trailing|2-low-leading|3-low-trailing   
#define SPI_BIT_ORDER       NRF_DRV_SPI_BIT_ORDER_MSB_FIRST  ///< MSB-LSB


/** 
 * @brief Function for initializing the SPI interface
 * 
 * @param[in] spi_instance          Instance of the spi interface to use
 */
void spi_init(const nrf_drv_spi_t *const spi_instance);


/** 
 * @brief Function for transmitting and receiving data over the SPI bus
 * 
 * @param[in] spi_instance          Instance of the spi interface to use
 * @param[in] p_tx_buffer           Buffer for holding the message to transmit
 * @param[in] tx_buffer_length      Length of the transmit buffer
 * @param[in] p_rx_buffer           Buffer for holding the received message
 * @param[in] rx_buffer_length      Length of the receive buffer
 * @param[out] bool                 Boolean to indicate if the SPI transfer was successful
 */
bool spi_transfer(const nrf_drv_spi_t *const spi_instance, 
                  const uint8_t* p_tx_buffer, uint8_t tx_buffer_length,
                  uint8_t* p_rx_buffer, uint8_t rx_buffer_length);


#endif // _spi_H__