
#include "boards.h"
#include "sdk_errors.h"
#include "fds.h"
#include "storage.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static volatile bool m_fds_write_flag = false; 
static volatile bool m_fds_all_records_deleted_flag = false;

static volatile uint16_t m_number_of_records = 0;

static volatile uint32_t m_read_file_id = 0;
static volatile uint32_t m_read_record_key = 0;


/**
 * @brief   Event handler for the FDS.
 */
static void fds_evt_handler(fds_evt_t const* p_fds_evt)
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
                fds_setWriteFlag(true);
            }
            break;

        case FDS_EVT_DEL_RECORD:
            // NRF_LOG_INFO("FDS_EVT_DEL_RECORD");
            fds_find_and_delete(m_read_file_id, m_read_record_key);
            break;

        default:
            break;
    }
}


/** 
 * @brief Function for running the garbage collector
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
static ret_code_t fds_garbage_collector()
{
    // call the garbage collector to empty them, don't need to do this all the time, this is just for demonstration
    ret_code_t err_code = fds_gc();
    return (err_code != FDS_SUCCESS) ? err_code : NRF_SUCCESS;
}


/** 
 * @brief Function for writing to the FDS
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * @param[in] p_write_data              Pointer to the data container
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_write(uint32_t write_file_id, uint32_t write_record_key, const uint8_t* p_write_data)
{    
    uint8_t m_write_buffer[TC_DATA_SIZE] = {0};
	memcpy(m_write_buffer, p_write_data, sizeof(m_write_buffer));

    fds_record_t        record;
    fds_record_desc_t   record_desc;

    // Set up record
    record.file_id              = write_file_id;
    record.key                  = write_record_key;
    record.data.p_data          = &m_write_buffer;
    record.data.length_words    = sizeof(m_write_buffer)/sizeof(uint8_t);

    ret_code_t ret = fds_record_write(&record_desc, &record);
    if (ret != FDS_SUCCESS)
    {
        return ret;
    }

    NRF_LOG_INFO("Writing Record ID = %d\r\n", record_desc.record_id);
    return NRF_SUCCESS;
}


/** 
 * @brief Function for reading from the FDS
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * @param[out] p_read_data              Pointer to the data container
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_read(uint32_t read_file_id, uint32_t read_record_key, uint8_t (*p_read_data)[TC_DATA_SIZE])
{
    fds_flash_record_t  flash_record;
    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok = {0};

    uint8_t*    data;
    uint32_t    err_code;

    m_number_of_records = 0;

    // Loop until all records with the given key and file ID have been found
    while (fds_record_find(read_file_id, read_record_key, &record_desc, &ftok) == FDS_SUCCESS)
    {
        err_code = fds_record_open(&record_desc, &flash_record);
        if (err_code != FDS_SUCCESS)
        {
            return err_code;
        }

        //NRF_LOG_INFO("Found Record ID = %d", record_desc.record_id);
        data = (uint8_t*) flash_record.p_data;
        for (uint8_t i=0;i<flash_record.p_header->length_words;i++)
		{
			p_read_data[m_number_of_records][i] = data[i];
		}
        
        err_code = fds_record_close(&record_desc);
        if (err_code != FDS_SUCCESS)
        {
            return err_code;
        }
        m_number_of_records++;
    }
    return NRF_SUCCESS;
}


/** 
 * @brief Function for finding and deleting records within a file
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_find_and_delete(uint32_t read_file_id, uint32_t read_record_key)
{
    m_read_file_id = read_file_id;
    m_read_record_key = read_record_key;

    ret_code_t err_code = FDS_SUCCESS;
    ret_code_t ret_code;

    fds_record_desc_t   record_desc;
    fds_find_token_t    ftok;

    ftok.page   = 0;
    ftok.p_addr = NULL;

    // After this function, a FDS_EVT_DEL_RECORD event is triggered
    // This event will continuously trigger this function and it will find all records with same ID and rec key and mark them as deleted
    if ((ret_code = fds_record_find(read_file_id, read_record_key, &record_desc, &ftok)) == FDS_SUCCESS)
    {
        err_code = fds_record_delete(&record_desc);
        if (err_code != FDS_SUCCESS)
        {
            // TODO: handle this error properly
            NRF_LOG_ERROR("FDS_RECORD_DELETE ERROR: %d", err_code);
        }
        
        NRF_LOG_INFO("Deleted Record ID: %d", record_desc.record_id);
        m_fds_all_records_deleted_flag = false;
    }

    if (ret_code == FDS_ERR_NOT_FOUND)
    {
        ret_code = fds_garbage_collector();
        NRF_LOG_INFO("Deleted all records with file id 0x%x and record key 0x%x\r\n", read_file_id, read_record_key);
        
        m_fds_all_records_deleted_flag = true;
        return ret_code;
    }

    return (err_code != FDS_SUCCESS) ? err_code : NRF_SUCCESS;
}


/** 
 * @brief Function for initializing the FDS
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
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

    NRF_LOG_INFO("FDS initialized\r\n");
    return NRF_SUCCESS;
}


/** 
 * @brief Function for getting the write flag
 * 
 * @return      Boolean indicating the flag status
 */
bool fds_getWriteFlag(void)
{
    return m_fds_write_flag;
}


/** 
 * @brief Function for setting the write flag
 * 
 * @param[in] fds_write_flag        Boolean indicating the flag status
 */
void fds_setWriteFlag(bool fds_write_flag)
{
    m_fds_write_flag = fds_write_flag;
}


/** 
 * @brief Function for getting all records deleted flag
 * 
 * @return      Boolean indicating the flag status
 */
bool fds_getAllRecordsDeletedFlag(void)
{
    return m_fds_all_records_deleted_flag;
}


/** 
 * @brief Function for setting the all records deleted flag
 * 
 * @param[in] fds_all_records_deleted_flag        Boolean indicating the flag status
 */
void fds_setAllRecordsDeletedFlag(bool fds_all_records_deleted_flag)
{
    m_fds_all_records_deleted_flag = fds_all_records_deleted_flag;
}


/** 
 * @brief Function for getting the number of found records
 * 
 * @return      16 bit initeger indicating the number of records found
 */
uint16_t fds_getNumberOfRecords(void)
{
    return m_number_of_records;
}


/** 
 * @brief Function for setting the number of found records
 * 
 * @param[in] number_of_records     16 bit initeger indicating the number of records found
 */
void fds_setNumberOfRecords(uint16_t number_of_records)
{
    m_number_of_records = number_of_records;
}