
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
#define BLE_UUID_TC_CHAR_VALUE      0x1401


/**@brief   Macro for defining a ble_tc instance 
 * 
 * @param[in]   _name   Name of the instance
 * @hideinitializer
 */
#define BLE_TC_SERVICE_DEF(_name)   \
static ble_tc_service_t _name;      \


// Forward declaration of the ble_tc_t type.
typedef struct ble_tc_service_s ble_tc_service_t;

/**@brief   Custom Service init structure. This contains all options and data needed for
 *          initialization of the service.
 */
typedef struct
{
    uint8_t                         initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t    custom_value_char_attr_md;      /**< Initial security level for Custom characteristics attribute */
} ble_tc_service_init_t;


/**@brief Custom Service structure. This contains various status information for the service.
*/
struct ble_tc_service_s
{
    uint16_t                    conn_handle;            /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection) */
    uint16_t                    service_handle;         /**< Handle of Our Service (as provided by the BLE stack) */
    ble_gatts_char_handles_t    char_handles;           /**< Handles related to the value characteristic */
    uint8_t                     uuid_type;
};


/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
ret_code_t ble_tc_service_init(ble_tc_service_t* p_tc_service, const ble_tc_service_init_t* p_tc_service_init);



#endif // _TC_SERVICE_H__