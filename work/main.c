/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
#include "boards.h"

#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_tcs.h"

#include "battery_voltage.h"
#include "max31856.h"
#include "timer.h"
#include "storage.h"

#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define DEVICE_NAME                     "Procrete"                              /**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME_LEN                 strlen(DEVICE_NAME)                     /**< Name length of device. Will define the length of the advertising data. */

#define MANUFACTURER_NAME               "ProwareTechnologies"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define HARDWARE_REV                    "1.0"                                   /**< Hardware revision. Will be passed to Device Information Service. */
#define FIRMAWARE_REV                   "1.0"                                   /**< Firmware revision. Will be passed to Device Information Service. */
#define SOFTWARE_REV                    "1.0"                                   /**< Software revision. Will be passed to Device Information Service. */

#define APP_ADV_INTERVAL                300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION                0                                       /**< The advertising duration in units of 10 milliseconds (0 -> always on). */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                      /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SPI_INSTANCE    0

#define FDS_FILE_ID     0x3185
#define FDS_REC_KEY     0x0001

#define BLE_TX_POWER    8

NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */

BLE_BAS_DEF(m_bas);
BLE_TCS_DEF(m_tcs);

bool tcs_update_flag = false;
bool bas_update_flag = false;

bool m_app_finished_flag = false;
bool m_app_activated_flag = false;

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_THERMOCOUPLE_SERVICE, BLE_UUID_TYPE_VENDOR_BEGIN},
    {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE},
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}
};

typedef enum
{
    e_cold_junction,
    e_thermocouple
} temperature_type;

union
{
    float float_val;
    unsigned char bytes_val[sizeof(float)];
} floatAsBytes;


static uint8_t m_tc_buffer_local[MAX_RECORD_SIZE * TC_DATA_SIZE] = {0};
static uint8_t m_tc_buffer_fds[MAX_RECORD_SIZE * TC_DATA_SIZE * MAX_NUMBER_OF_DAYS] = {0};

static uint8_t m_number_of_measurements = 0;
static uint16_t m_total_number_of_measurements = 0;


static void advertising_start(bool erase_bonds);


/**
 * @brief Function for converting a float to bytes
 * 
 * @param[in] m_float    Float to be converted to bytes
 * 
 * @return  Bytes that form a float value
 */
unsigned char* float2bytes(float m_float)
{
    floatAsBytes.float_val = m_float;
    return floatAsBytes.bytes_val;
}


/**
 * @brief Function for converting bytes to a float
 * 
 * @param[in] m_bytes    Bytes that form a float (4 bytes)
 * 
 * @return  Float value
 */
float bytes2float(unsigned char m_bytes[sizeof(float)])
{
    memcpy(floatAsBytes.bytes_val, m_bytes, sizeof(float));
    return floatAsBytes.float_val;
}


/**
 * @brief Function for handling the writing the thermocouple buffer to FDS
 */
static void write_tc_buffer_too_fds(void)
{
    ret_code_t ret_code;
    
    if (m_number_of_measurements > 0) 
    {
        NRF_LOG_INFO("Writing thermocouple buffer to FDS...");
        ret_code = fds_write(FDS_FILE_ID, FDS_REC_KEY, m_tc_buffer_local, (m_number_of_measurements * TC_DATA_SIZE));
    
        if (ret_code == FDS_ERR_RECORD_TOO_LARGE)
        {
            NRF_LOG_INFO("FDS_ERR_RECORD_TOO_LARGE");
        }
        else if (ret_code == FDS_ERR_NO_SPACE_IN_FLASH)
        {
            NRF_LOG_INFO("FDS_ERR_NO_SPACE_IN_FLASH");
        }
        else
        {
            APP_ERROR_CHECK(ret_code);

            while (!fds_getWriteFlag());
            fds_setWriteFlag(false);
        }

        m_number_of_measurements = 0;
        memcpy(m_tc_buffer_local, 0, sizeof(m_tc_buffer_local));
    }
}


/**
 * @brief Function for handling the MAX31856 timer interrupt
 */
static void max31856_int_handler(temperature_type type)
{
    bsp_board_led_on(BSP_BOARD_LED_2);
    
    // TODO: Handle error
    if (max31856_checkFaultStatus() != APPROVED)
    {
        max31856_resetFaultStatus();
    }

    if (type == e_cold_junction)
    {
        float cold_junction_temperature = 0.0f;
        unsigned char* cold_junction_temperature_bytes;

        if (max31856_getColdJunctionTemperature(&cold_junction_temperature) == MAX31856_SUCCESS)
        {
            NRF_LOG_INFO("Measurement: %d", m_number_of_measurements + 1);
            NRF_LOG_INFO("Cold Junction Temperature: " NRF_LOG_FLOAT_MARKER "°C\r\n", NRF_LOG_FLOAT(cold_junction_temperature));
            cold_junction_temperature_bytes = float2bytes(cold_junction_temperature);

            if (cold_junction_temperature != 0.0f)
            {
                memcpy(&m_tc_buffer_local[m_number_of_measurements * TC_DATA_SIZE], cold_junction_temperature_bytes, sizeof(cold_junction_temperature_bytes));
                
                m_number_of_measurements++;
                m_total_number_of_measurements++;
            }
        }
    }
    else if (type == e_thermocouple)
    {
        float thermocouple_temperature = 0.0f;
        unsigned char* thermocouple_temperature_tytes;

        if (max31856_getThermoCoupleTemperature(&thermocouple_temperature) == MAX31856_SUCCESS)
        {
            NRF_LOG_INFO("Measurement: %d", m_number_of_measurements + 1);
            NRF_LOG_INFO("Thermocouple Temperature: " NRF_LOG_FLOAT_MARKER "°C\r\n", NRF_LOG_FLOAT(thermocouple_temperature));
            thermocouple_temperature_tytes = float2bytes(thermocouple_temperature);

            if (thermocouple_temperature != 0.0f) {
                memcpy(&m_tc_buffer_local[m_number_of_measurements * TC_DATA_SIZE], thermocouple_temperature_tytes, sizeof(thermocouple_temperature_tytes));

                m_number_of_measurements++;
                m_total_number_of_measurements++;
            }
        }
    }
    else
    {
        NRF_LOG_ERROR("ERROR: UNKNOWN TEMPERATURE TYPE");
    }

    bsp_board_led_off(BSP_BOARD_LED_2);
} 


/**
 * @brief Function for reading the FDS records.
 * 
 * @param[out]  p_tc_buffer     Buffer to hold the thermocouple data.
 */
void read_records() 
{
    static uint8_t read_data[MAX_NUMBER_OF_DAYS][MAX_RECORD_SIZE * TC_DATA_SIZE] = {0};
    APP_ERROR_CHECK(fds_read(FDS_FILE_ID, FDS_REC_KEY, read_data));

    NRF_LOG_INFO("Total number of measurements: %d", m_total_number_of_measurements);
    NRF_LOG_INFO("Total number of records: %d", fds_getNumberOfRecords());

    int k = 0;
    for (int i = 0; i < fds_getNumberOfRecords(); i++)
    {
        for (int j = 0; j < MAX_RECORD_SIZE * TC_DATA_SIZE; j++)
        {
            m_tc_buffer_fds[k] = read_data[i][j];
            k++;

            // if (k % 4 == 0)
            // {
            //     float cold_junction_temperature = bytes2float(read_data[i]);
            //     NRF_LOG_INFO("Measurement %d, temperature: " NRF_LOG_FLOAT_MARKER "°C", i+1, NRF_LOG_FLOAT(cold_junction_temperature));
            // }
        }
    }
    //NRF_LOG_HEXDUMP_INFO(m_tc_buffer_fds, m_total_number_of_measurements * TC_DATA_SIZE);
    NRF_LOG_INFO("\t*** END OF RECORDS ***\r\n");
}



/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            advertising_start(false);
            break;

        default:
            break;
    }
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
       err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
       APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module. 
*/
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void battery_level_update(void)
{
    ret_code_t err_code;
    
    uint8_t  battery_level;
    uint16_t vbatt;

    battery_voltage_get(&vbatt);
    battery_level = battery_level_in_percent(vbatt);
    NRF_LOG_INFO("ADC result in percent: %d", battery_level);

    err_code = ble_bas_battery_level_update(&m_bas, battery_level, BLE_CONN_HANDLE_ALL);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
       )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for performing thermocouple measurement and updating the Thermocouple characteristic
 *        in Thermocouple Service.
 */
static void thermocouple_level_update(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Reading thermocouple data log from FDS...")
    read_records();

    uint8_t end_bytes[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

    uint16_t number_of_records      = fds_getNumberOfRecords();
    uint32_t tc_buffer_fds_length   = number_of_records * MAX_RECORD_SIZE * TC_DATA_SIZE;
    uint16_t tc_buffer_local_length = m_number_of_measurements * TC_DATA_SIZE;
    uint32_t total_number_of_bytes  = tc_buffer_fds_length + tc_buffer_local_length + sizeof(end_bytes);

    uint8_t* p_tc_buffer = malloc(total_number_of_bytes);
    memcpy(p_tc_buffer, m_tc_buffer_fds, tc_buffer_fds_length);
    memcpy(&p_tc_buffer[tc_buffer_fds_length], m_tc_buffer_local, tc_buffer_local_length);
    memcpy(&p_tc_buffer[tc_buffer_fds_length + tc_buffer_local_length], end_bytes, sizeof(end_bytes));

    NRF_LOG_INFO("Sending Thermocouple data...");

    err_code = ble_tcs_thermocouple_level_update(&m_tcs, p_tc_buffer, total_number_of_bytes);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != NRF_ERROR_RESOURCES) &&
        (err_code != NRF_ERROR_BUSY) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
    {
        APP_ERROR_HANDLER(err_code);
    }

    free(p_tc_buffer);
}


/**@brief Function for handling the TC Service events.
 * @details This function will be called for all TC Service events which are passed to
 *          the application.
 *
 * @param[in]   p_tcs   TC Service structure.
 * @param[in]   p_evt          Event received from the TC Service.
 *
 */
static void on_tcs_evt(ble_tcs_t* p_tcs,
                       ble_tcs_evt_t* p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_TCS_EVT_CONNECTED:
            NRF_LOG_INFO("BLE_TCS_EVT_CONNECTED");
            break;
        
        case BLE_TCS_EVT_DISCONNECTED:
            NRF_LOG_INFO("BLE_TCS_EVT_DISCONNECTED");
            break;

        case BLE_TCS_EVT_NOTIFICATION_ENABLED:
            NRF_LOG_INFO("BLE_TCS_EVT_NOTIFICATION_ENABLED\r\n");
            tcs_update_flag = true;
            break;
        
        case BLE_TCS_EVT_NOTIFICATION_DISABLED:
            NRF_LOG_INFO("BLE_TCS_EVT_NOTIFICATION_DISABLED\r\n");
            tcs_update_flag = false;
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t              err_code;
    nrf_ble_qwr_init_t      qwr_init         = {0};
    ble_tcs_init_t          tcs_init         = {0};
    ble_dis_init_t          dis_init         = {0};
    ble_bas_init_t          bas_init         = {0};

    // Initialize Queued Write Module
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);


    // Initialize Thermocouple Service.
    memset(&tcs_init, 0, sizeof(tcs_init));

    // Set peer permissions for tc service
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&tcs_init.tc_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&tcs_init.tc_value_char_attr_md.write_perm);

    // Set the TC event handler
    tcs_init.evt_handler = on_tcs_evt;

    err_code = ble_tcs_init(&m_tcs, &tcs_init);
    APP_ERROR_CHECK(err_code);


    // Initialize Battery Service.
    memset(&bas_init, 0, sizeof(bas_init));

    bas_init.evt_handler            = NULL;
    bas_init.support_notification   = false;
    bas_init.p_report_ref           = NULL;
    bas_init.initial_batt_level     = 100;

    bas_init.bl_rd_sec          = SEC_OPEN;
    bas_init.bl_cccd_wr_sec     = SEC_OPEN;
    bas_init.bl_report_rd_sec   = SEC_OPEN;

    err_code = ble_bas_init(&m_bas, &bas_init);
    APP_ERROR_CHECK(err_code);


    // Initialize Device Information Service.
    memset(&dis_init, 0, (sizeof(dis_init)));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char*)MANUFACTURER_NAME);
    //ble_srv_ascii_to_utf8(&dis_init.serial_num_str, (char*)SERIAL_NUM);
    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, (char*)HARDWARE_REV);
    ble_srv_ascii_to_utf8(&dis_init.fw_rev_str, (char*)FIRMAWARE_REV);
    ble_srv_ascii_to_utf8(&dis_init.sw_rev_str, (char*)SOFTWARE_REV);

    dis_init.dis_char_rd_sec = SEC_OPEN;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising\r\n");
            break;

        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("Advertising idle state\r\n");
            break;

        default:
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected\r\n");
            bsp_board_led_on(BSP_BOARD_LED_0);
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected\r\n");
            bsp_board_led_off(BSP_BOARD_LED_0);

            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_CONN, m_conn_handle, BLE_TX_POWER);
            APP_ERROR_CHECK(err_code);
            
            bas_update_flag = true;
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    ble_advdata_manuf_data_t            manuf_data;         /**< Advertising data -> MAX. 31 bytes */
    uint8_t data[]                      = "Proware";        /**< Extra data to advertise */
    manuf_data.company_identifier       = 0xFFFF;           /**< COMPANY ID -> ACHIEVED BY BECOMING A BLUETOOTH SIG MEMBER */
    manuf_data.data.p_data              = data;
    manuf_data.data.size                = sizeof(data);
    init.advdata.p_manuf_specific_data  = &manuf_data;

    init.advdata.name_type               = BLE_ADVDATA_SHORT_NAME;
    init.advdata.short_name_len          = DEVICE_NAME_LEN;
    init.advdata.include_appearance      = false;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


/**@brief Function for initializing leds.
 */
static void leds_init()
{
    bsp_board_init(BSP_INIT_LEDS);
    bsp_board_led_on(BSP_BOARD_LED_0);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the TX power level.
 */
static void tx_power_init(void)
{
    ret_code_t err_code;
    err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_advertising.adv_handle, BLE_TX_POWER);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_SCAN_INIT, BLE_CONN_HANDLE_INVALID, BLE_TX_POWER);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the DCDC switching regulator.
 */
static void dcdc_init(void)
{
    ret_code_t err_code;
    err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();     /**< Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event */
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
// static void sleep_mode_enter(void)
// {
//     ret_code_t err_code;
//     err_code = sd_power_system_off();
//     APP_ERROR_CHECK(err_code);
// }


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds = true;

    // Initialize.
    log_init();
    NRF_LOG_INFO("*****************************************************\r\n\n");
    NRF_LOG_INFO("\r\n\n\n\t*** CONCRETE SENSOR ***\r\n");
    NRF_LOG_FLUSH();

    timer_init();
    leds_init();
    battery_voltage_init();
    power_management_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    tx_power_init();
    dcdc_init();
    conn_params_init();
    peer_manager_init();

    spi_init(&spi);
    max31856_init(&spi);
    
    APP_ERROR_CHECK(fds_storage_init());
    
    fds_find_and_delete(FDS_FILE_ID, FDS_REC_KEY);
    while(!fds_getAllRecordsDeletedFlag());

    //sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, );
    advertising_start(erase_bonds);
    //sleep_mode_enter();

    NRF_LOG_FLUSH();

    while (true)
    {
        idle_state_handle();

        if (ble_tcs_getActivatedFlag())
        {
            const uint32_t timer_interval = ble_tcs_getTimerInterval(); 
            if (timer_interval != 0)
            {
                ble_tcs_setActivatedFlag(false);
                m_app_activated_flag = true;

                NRF_LOG_INFO("\r\n\n\n\t*** STARTING APPLICATION ***\r\n");
                NRF_LOG_INFO("Running application with Thermocouple timer interval of %dms\r\n", timer_interval);

                timer_start(timer_interval * 1000);
                max31856_int_handler(e_cold_junction);
            }
        }


        if (m_app_activated_flag)
        {
            if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
            {
                if (bas_update_flag)
                {
                    battery_level_update();
                    bas_update_flag = false;
                }

                if (tcs_update_flag)
                {
                    thermocouple_level_update();
                    tcs_update_flag = false;
                }
            }

            if (fds_getNumberOfRecords() < MAX_NUMBER_OF_DAYS)
            {
                if (timer_getIntFlag())
                {
                    max31856_int_handler(e_cold_junction);
                    timer_setIntFlag(false);
                }

                if (m_number_of_measurements >= MAX_RECORD_SIZE)
                {
                    write_tc_buffer_too_fds();
                }
            }
            else if (!m_app_finished_flag)
            {
                NRF_LOG_INFO("\r\n\n\n\t*** APPLICATION FINISHED ***\r\n");
                m_app_finished_flag = true;
            }
        }
    }
}


/**
 * @}
 */
