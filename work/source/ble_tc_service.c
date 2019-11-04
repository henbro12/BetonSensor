
#include <string.h>
#include "boards.h"
#include "sdk_common.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "ble_srv_common.h"
#include "ble_tc_service.h"



ret_code_t ble_tc_service_init(ble_tc_service_t* p_tc_service, const ble_tc_service_init_t* p_tc_service_init)
{
    if (p_tc_service == NULL || p_tc_service_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    ret_code_t  err_code;
    ble_uuid_t  service_uuid;

    // Initialize service structure
    p_tc_service->conn_handle       = BLE_CONN_HANDLE_INVALID;

    // Add service UUID
    ble_uuid128_t base_uuid = {BLE_UUID_TC_SERVICE_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_tc_service->uuid_type);
    VERIFY_SUCCESS(err_code);

    service_uuid.type = p_tc_service->uuid_type;
    service_uuid.uuid = BLE_UUID_TC_SERVICE;

    // Add service to BLE Stack's GATT table
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, 
                                        &service_uuid, 
                                        &p_tc_service->service_handle);
    VERIFY_SUCCESS(err_code);

    return err_code;
}
