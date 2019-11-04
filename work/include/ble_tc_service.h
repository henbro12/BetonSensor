
#ifndef _TC_SERVICE_H__
#define _TC_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

// UUID: 63CCxxxx-8BA2-4D48-aCC4-E5669638E060
#define BLE_UUID_TC_SERVICE_BASE   {0x60, 0xE0, 0x38, 0x96, 0x66, 0xE5, 0xC4, 0xAC, \
                                    0x48, 0x4D, 0xA2, 0x8B, 0x00, 0x00, 0xCC, 0x63}

#define BLE_UUID_TC_SERVICE         0x1400
#define BLE_UUID_TC_CHAR            0x1401


/**@brief   Macro for defining a ble_tc instance 
 * 
 * @param[in]   _name   Name of the instance
 * @hideinitializer
 */
#define BLE_TC_SERVICE_DEF(_name)                   \
static ble_tc_service_t _name;                      \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,     \
                     ble_tc_service_on_ble_evt,     \
                     &_name)


/**@brief   TC Service event type. */
typedef enum
{
    BLE_TC_SERVICE_EVT_NOTIFICATION_ENABLED,
    BLE_TC_SERVICE_EVT_NOTIFICATION_DISABLED,
    BLE_TC_SERVICE_EVT_DISCONNECTED,
    BLE_TC_SERVICE_EVT_CONNECTED
} ble_tc_service_evt_type_t;


/**@brief TC Service event. */
typedef struct
{
    ble_tc_service_evt_type_t   evt_type;
} ble_tc_service_evt_t;


/**@brief   Forward declaration of the ble_tc_t type. */
typedef struct ble_tc_service_s ble_tc_service_t;


/**@brief TC Service event handler type. */
typedef void (*ble_tc_service_evt_handler_t) (ble_tc_service_t* p_tc_service, ble_tc_service_evt_t* p_evt);


/**@brief   Custom Service init structure. This contains all options and data needed for
 *          initialization of the service. */
typedef struct
{
    ble_tc_service_evt_handler_t    evt_handler;                /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                         initial_tc_value;           /**< Initial tc value */
    ble_srv_cccd_security_mode_t    tc_value_char_attr_md;      /**< Initial security level for tc characteristics attribute */
} ble_tc_service_init_t;


/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_tc_service_s
{
    ble_tc_service_evt_handler_t    evt_handler;            /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                        conn_handle;            /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection) */
    uint16_t                        service_handle;         /**< Handle of Our Service (as provided by the BLE stack) */
    ble_gatts_char_handles_t        char_handles;           /**< Handles related to the value characteristic */
    uint8_t                         uuid_type;
};


/**@brief Function for adding the TC Value characteristic.
 *
 * @param[in]   p_tc_service        TC Service structure.
 * @param[in]   p_tc_service_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
ret_code_t ble_tc_service_init(ble_tc_service_t* p_tc_service, const ble_tc_service_init_t* p_tc_service_init);


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_tc_service_on_ble_evt(ble_evt_t const* p_ble_evt, void* p_context);


/**@brief Function for updating the custom value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *       
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
ret_code_t ble_tc_service_value_update(ble_tc_service_t* p_tc_service, uint8_t tc_value);


#endif // _TC_SERVICE_H__