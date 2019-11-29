
#include <string.h>
#include "boards.h"
#include "sdk_common.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "ble_srv_common.h"
#include "ble_tcs.h"


#define TC_MAX_PACKET_LENGTH  20

static volatile bool m_nrf_error_resources = false;
volatile uint32_t m_tc_data_size = 0;
volatile uint32_t m_tc_data_pos = 0;
uint8_t* m_tc_data;

static volatile bool m_tcs_activated_flag = false;
static volatile uint32_t m_tcs_timer_interval = 0;

/**@brief Function for updating the Thermocouple value per packet.
 *
 * @details The application calls this function when the thermocouple value should be updated.
 *          A packet of max 20 bytes will be send to the client.
 *       
 * @param[in]   p_tcs               Thermocouple Service structure.
 * @param[in]   p_tc_packet         Packet to be send. 
 * @param[in]   tc_packet_length    Length of the packet.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static ret_code_t ble_tcs_send_packet(ble_tcs_t* p_tcs, uint8_t* p_tc_packet, uint16_t tc_packet_length)
{
    ret_code_t err_code;
    ble_gatts_hvx_params_t hvx_params;

    if (m_nrf_error_resources)
    {
        return NRF_ERROR_RESOURCES;
    }

    VERIFY_PARAM_NOT_NULL(p_tcs);

    if (p_tcs->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if (tc_packet_length > TC_MAX_PACKET_LENGTH)
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle   = p_tcs->char_handles.value_handle;
    hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset   = 0;
    hvx_params.p_len    = &tc_packet_length;
    hvx_params.p_data   = p_tc_packet;

    err_code = sd_ble_gatts_hvx(p_tcs->conn_handle, &hvx_params);
    if (err_code == NRF_ERROR_RESOURCES)
    {
        m_nrf_error_resources = true;
    }

    return err_code;
}


/**@brief Function for pushing the Thermocouple data.
 *
 * @details The application calls this function when the thermcouple value should be updated.
 *          The data will be split in packets of 20 bytes and send to the client.
 *       
 * @param[in]   p_tcs           Thermocouple Service structure.
 * 
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static ret_code_t push_data_packets(ble_tcs_t* p_tcs)
{
    ret_code_t err_code = NRF_SUCCESS;
    uint32_t packet_length = TC_MAX_PACKET_LENGTH;
    uint32_t packet_size = 0;

    while(err_code == NRF_SUCCESS)
    {
        if ((m_tc_data_size - m_tc_data_pos) >= packet_length)
        {
            packet_size = packet_length;
        }
        else if ((m_tc_data_size - m_tc_data_pos) >= 0)
        {
            packet_size = (m_tc_data_size - m_tc_data_pos);
        }

        if (packet_size > 0)
        {
            err_code = ble_tcs_send_packet(p_tcs, &m_tc_data[m_tc_data_pos], packet_size);
            if (err_code == NRF_SUCCESS)
            {
                m_tc_data_pos += packet_size;
            }
            else
            {
                break;
            }
            
        }
        else
        {
            NRF_LOG_INFO("Data send successful\r\n")
            m_tc_data_size = 0;
            m_tc_data_pos = 0;
            break;
        }
    }
    return err_code;
}


/**@brief Function for updating the thermocouple value.
 *
 * @details The application calls this function when the thermcouple value should be updated. If
 *          notification has been enabled, the thermocouple value characteristic is sent to the client.
 *       
 * @param[in]   p_tcs           Thermocouple Service structure.
 * @param[in]   p_tc_data       Thermocouple data to send.
 * @param[in]   tc_data_length  Thermocouple data length.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
ret_code_t ble_tcs_thermocouple_level_update(ble_tcs_t* p_tcs, uint8_t* p_tc_data, uint16_t tc_data_length)
{   
    ret_code_t err_code;

    VERIFY_PARAM_NOT_NULL(p_tcs);
    VERIFY_PARAM_NOT_NULL(p_tc_data);

    if (p_tcs->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    m_tc_data_size = tc_data_length;
    m_tc_data = p_tc_data;

    err_code = push_data_packets(p_tcs);
    if (err_code == NRF_ERROR_RESOURCES) return NRF_SUCCESS;
    return err_code;
}


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_tcs       Thermocouple Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_tcs_t* p_tcs, ble_evt_t const* p_ble_evt)
{
    p_tcs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_tcs_evt_t evt;
    evt.evt_type = BLE_TCS_EVT_CONNECTED;
    p_tcs->evt_handler(p_tcs, &evt);
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_tcs       Thermocouple Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_tcs_t* p_tcs, ble_evt_t const* p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_tcs->conn_handle = BLE_CONN_HANDLE_INVALID;

    ble_tcs_evt_t evt;
    evt.evt_type = BLE_TCS_EVT_DISCONNECTED;
    p_tcs->evt_handler(p_tcs, &evt);
}


/**@brief Function for handling write events to the TC characteristic.
 *
 * @param[in]   p_tcs           Thermocouple Service structure.
 * @param[in]   p_evt_write     Write event received from the BLE stack.
 */
static void on_tc_cccd_write(ble_tcs_t* p_tcs, ble_gatts_evt_write_t const* p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        if (p_tcs->evt_handler != NULL)
        {
            ble_tcs_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_TCS_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_TCS_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_tcs->evt_handler(p_tcs, &evt);
        }
    }
}


/**@brief Function for handling the Write event.
 *
 * @param[in]   p_tcs           Thermocouple Service structure.
 * @param[in]   p_ble_evt       Event received from the BLE stack.
 */
static void on_write(ble_tcs_t* p_tcs, ble_evt_t const* p_ble_evt)
{
    ble_gatts_evt_write_t const* p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_tcs->char_handles.value_handle)
    {
        char receivedString[p_evt_write->len + 1];
        for (int i = 0; i < p_evt_write->len; i++) {
            receivedString[i] = p_evt_write->data[i];
        }
        receivedString[p_evt_write->len] = '\0';

        if (strcmp(receivedString, "Activate") == 0)
        {
            m_tcs_activated_flag = true;
        }
        
        if (strstr(receivedString, "TimerInterval") != NULL)
        {            
            char delim[] = "=";
            char *timer_interval = strtok(receivedString, delim);
            timer_interval = strtok(NULL, delim);
            sscanf(timer_interval, "%ld", &m_tcs_timer_interval);
        }
    }

    // Check if the tc value CCCD is written to.
    if (p_evt_write->handle == p_tcs->char_handles.cccd_handle)
    {
        on_tc_cccd_write(p_tcs, p_evt_write);
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Thermocouple Service.
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Thermocouple Service structure.
 */
void ble_tcs_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context)
{
    ble_tcs_t* p_tcs = (ble_tcs_t*) p_context;

    if (p_tcs == NULL || p_ble_evt == NULL)
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_tcs, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_tcs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_tcs, p_ble_evt);
            break;
        
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            {
                if (m_tc_data_size > 0)
                {
                    push_data_packets(p_tcs);
                }
                m_nrf_error_resources = false;
            }
            break;

        default:
            break;
    }
}


/**@brief Function for adding the TC Value characteristic.
 *
 * @param[in]   p_tcs        TC Service structure.
 * @param[in]   p_tcs_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static ret_code_t tc_char_add(ble_tcs_t* p_tcs, const ble_tcs_init_t* p_tcs_init)
{
    ret_code_t          err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;

    // Populate cccd_md
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc    = BLE_GATTS_VLOC_STACK;

    // Populate char_md
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read     = 1;
    char_md.char_props.write    = 1;
    char_md.char_props.notify   = 1;
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;

    // Populate attr_md
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm   = p_tcs_init->tc_value_char_attr_md.read_perm;
    attr_md.write_perm  = p_tcs_init->tc_value_char_attr_md.write_perm;
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth     = 0;
    attr_md.wr_auth     = 0;
    attr_md.vlen        = 1;

    // Populate char_uuid
    char_uuid.type = p_tcs->uuid_type;
    char_uuid.uuid = BLE_UUID_THERMOCOUPLE_CHAR;

    // Populate attr_char_value
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    attr_char_value.max_len     = TC_MAX_PACKET_LENGTH;
    attr_char_value.init_len    = sizeof(uint8_t);
    attr_char_value.init_offs   = 0;

    // Add characteristic
    err_code = sd_ble_gatts_characteristic_add(p_tcs->service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &p_tcs->char_handles);
    VERIFY_SUCCESS(err_code);

    return err_code;
}


/**@brief Function for initializing the TC Service.
 *
 * @param[out]  p_tcs        TC Service structure. This structure will have to be supplied by
 *                           the application. It will be initialized by this function, and will later
 *                           be used to identify this particular service instance.
 * @param[in]   p_tcs_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
ret_code_t ble_tcs_init(ble_tcs_t* p_tcs, const ble_tcs_init_t* p_tcs_init)
{
    ret_code_t  err_code;
    ble_uuid_t  service_uuid;

    VERIFY_PARAM_NOT_NULL(p_tcs);
    VERIFY_PARAM_NOT_NULL(p_tcs_init);

    // Initialize service structure
    p_tcs->evt_handler       = p_tcs_init->evt_handler;
    p_tcs->conn_handle       = BLE_CONN_HANDLE_INVALID;

    // Add service UUID
    ble_uuid128_t base_uuid = {BLE_UUID_THERMOCOUPLE_SERVICE_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_tcs->uuid_type);
    VERIFY_SUCCESS(err_code);

    service_uuid.type = p_tcs->uuid_type;
    service_uuid.uuid = BLE_UUID_THERMOCOUPLE_SERVICE;

    // Add service to BLE Stack's GATT table
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, 
                                        &service_uuid, 
                                        &p_tcs->service_handle);
    VERIFY_SUCCESS(err_code);

    // Add tc characteristic
    return tc_char_add(p_tcs, p_tcs_init);
}


/** 
 * @brief Function for getting the activated flag
 * 
 * @return      Boolean indicating the flag status
 */
bool ble_tcs_getActivatedFlag(void)
{
    return m_tcs_activated_flag;
}


/** 
 * @brief Function for setting the activated flag
 * 
 * @param[in]      Boolean indicating the flag status
 */
void ble_tcs_setActivatedFlag(bool tcs_activated_flag)
{
    m_tcs_activated_flag = tcs_activated_flag;
}


/** 
 * @brief Function for getting the timer interval
 * 
 * @return      Uint32_t representing the timer interval
 */
uint32_t ble_tcs_getTimerInterval(void)
{
    return m_tcs_timer_interval;
}


/** 
 * @brief Function for setting the timer interval
 * 
 * @param[in]      Uint32_t representing the timer interval
 */
void ble_tcs_setTimerInterval(uint32_t tcs_timer_interval)
{
    m_tcs_timer_interval = tcs_timer_interval;
}