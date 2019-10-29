
#include "boards.h"
#include "sdk_errors.h"
#include "fds.h"
#include "storage.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static volatile uint8_t m_fds_write_flag = 0; 
#define FILE_ID_FDS     0x1111
#define REC_KEY_FDS     0x2222


void fds_evt_handler(fds_evt_t const* p_fds_evt)
{
    switch (p_fds_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_fds_evt->result != FDS_SUCCESS)
            {
                NRF_LOG_ERROR("FDS initialization failed")
            }
            break;
        case FDS_EVT_WRITE:
            if (p_fds_evt->result == FDS_SUCCESS)
            {
                m_fds_write_flag = 1;
            }
            break;
        default:
            break;
    }
}

ret_code_t fds_write(void)
{
    static uint8_t const    m_deadbeef[4] = {0x01, 0x02, 0x03, 0x04};
    
    fds_record_t        record;
    fds_record_desc_t   record_desc;

    // Set up record
    record.file_id              = FILE_ID_FDS;
    record.key                  = REC_KEY_FDS;
    record.data.p_data          = &m_deadbeef;
    record.data.length_words    = sizeof(m_deadbeef)/sizeof(uint8_t);

    ret_code_t ret = fds_record_write(&record_desc, &record);
    if (ret != FDS_SUCCESS)
    {
        return ret;
    }

    NRF_LOG_INFO("Writing Record ID = %d\r\n", record_desc.record_id);
    return NRF_SUCCESS;
}

ret_code_t fds_read(void)
{
    fds_flash_record_t  flash_record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok = {0};

    uint8_t*    data;
    uint32_t    err_code;

    NRF_LOG_INFO("Start searching...\r\n");

    // Loop until all records with the given key and file ID have been founc
    while (fds_record_find(FILE_ID_FDS, REC_KEY_FDS, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_open(&record_desc, &flash_record);
        if (err_code != FDS_SUCCESS)
        {
            return err_code;
        }

        NRF_LOG_INFO("Found Record ID = %d\r\n", record_desc.record_id);
        NRF_LOG_INFO("Data = ");
        data = (uint8_t*) flash_record.p_data;
        for (uint8_t i = 0; i < flash_record.p_header->length_words; i++)
        {
            NRF_LOG_INFO("0x%8x", data[i]);
        }
        NRF_LOG_INFO("\r\n");

        err_code = fds_record_close(&record_desc);
        if (err_code != FDS_SUCCESS)
        {
            return err_code;
        }
    }
    return NRF_SUCCESS;
}

ret_code_t fds_find_and_delete(void)
{
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;

    ftok.page   = 0;
    ftok.p_addr = NULL;

    // Loop and find records with same ID and rec key and mark them as deleted
    while (fds_record_find(FILE_ID_FDS, REC_KEY_FDS, &record_desc, &ftok) == FDS_SUCCESS)
    {
        fds_record_delete(&record_desc);
        NRF_LOG_INFO("Deleted Record ID = %d\r\n", record_desc.record_id);
    }

    // call the garbage collector to empty them, don't need to do this all the time, this is just for demonstration
    ret_code_t ret = fds_gc();
    if (ret != FDS_SUCCESS)
    {
        return ret;
    }
    return NRF_SUCCESS;
}

ret_code_t fds_storage_init(void)
{
    ret_code_t ret = fds_register(fds_evt_handler);
    if (ret != FDS_SUCCESS)
    {
        return ret;
    }

    ret = fds_init();
    if (ret != FDS_SUCCESS)
    {
        return ret;
    }

    return NRF_SUCCESS;
}

bool fds_getWriteFlag(void)
{
    return m_fds_write_flag;
}

void fds_setWriteFlag(bool fds_write_flag)
{
    m_fds_write_flag = fds_write_flag;
}