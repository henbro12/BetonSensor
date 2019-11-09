
#ifndef _BLE_TCS_H__
#define _BLE_TCS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

// UUID: 63CCxxxx-8BA2-4D48-aCC4-E5669638E060
#define BLE_UUID_THERMOCOUPLE_SERVICE_BASE     {0x60, 0xE0, 0x38, 0x96, 0x66, 0xE5, 0xC4, 0xAC, \
                                                0x48, 0x4D, 0xA2, 0x8B, 0x00, 0x00, 0xCC, 0x63}

#define BLE_UUID_THERMOCOUPLE_SERVICE           0x1400
#define BLE_UUID_THERMOCOUPLE_CHAR              0x1401


/**@brief   Macro for defining a ble_tcs instance 
 * 
 * @param[in]   _name   Name of the instance
 * @hideinitializer
 */
#define BLE_TCS_DEF(_name)                          \
static ble_tcs_t _name;                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,     \
                     ble_tcs_on_ble_evt,            \
                     &_name)


/**@brief   TC Service event type. */
typedef enum
{
    BLE_TCS_EVT_NOTIFICATION_ENABLED,
    BLE_TCS_EVT_NOTIFICATION_DISABLED,
    BLE_TCS_EVT_DISCONNECTED,
    BLE_TCS_EVT_CONNECTED
} ble_tcs_evt_type_t;


/**@brief TC Service event. */
typedef struct
{
    ble_tcs_evt_type_t   evt_type;
} ble_tcs_evt_t;


/**@brief   Forward declaration of the ble_tcs_t type. */
typedef struct ble_tcs_s ble_tcs_t;


/**@brief TC Service event handler type. */
typedef void (*ble_tcs_evt_handler_t) (ble_tcs_t* p_tcs, ble_tcs_evt_t* p_evt);


/**@brief   Custom Service init structure. This contains all options and data needed for
 *          initialization of the service. */
typedef struct
{
    ble_tcs_evt_handler_t           evt_handler;                /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                         initial_tc_value;           /**< Initial tc value */
    ble_srv_cccd_security_mode_t    tc_value_char_attr_md;      /**< Initial security level for tc characteristics attribute */
} ble_tcs_init_t;


/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_tcs_s
{
    ble_tcs_evt_handler_t           evt_handler;            /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                        conn_handle;            /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection) */
    uint16_t                        service_handle;         /**< Handle of Our Service (as provided by the BLE stack) */
    ble_gatts_char_handles_t        char_handles;           /**< Handles related to the value characteristic */
    uint8_t                         uuid_type;
};


/**@brief Function for adding the TC Value characteristic.
 *
 * @param[in]   p_tcs        TC Service structure.
 * @param[in]   p_tcs_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
ret_code_t ble_tcs_init(ble_tcs_t* p_tcs, const ble_tcs_init_t* p_tcs_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Thermocouple Service.
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Thermocouple Service structure.
 */
void ble_tcs_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context);


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
ret_code_t ble_tcs_thermocouple_level_update(ble_tcs_t* p_tcs, uint8_t* p_tc_data, uint16_t tc_data_length);


#endif // _BLE_TCS_H__