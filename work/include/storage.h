
#ifndef _storage_H__
#define _storage_H__

#define TC_DATA_SIZE        sizeof(float)   // Size of float (temperature)
#define MAX_RECORD_SIZE     144             // 1 day, every 10 minutes
#define MAX_NUMBER_OF_DAYS  30              // Maximum number of days the application will run

#define WORD                4               // Number of bytes in a word

/** 
 * @brief Function for writing to the FDS
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * @param[in] p_write_data              Pointer to the data container
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_write(uint32_t write_file_id, uint32_t write_record_key, uint8_t* p_write_data, uint32_t data_length);


/** 
 * @brief Function for reading from the FDS
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * @param[out] p_read_data              Pointer to the data container
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_read(uint32_t read_file_id, uint32_t read_record_key, uint8_t (*p_read_data)[MAX_RECORD_SIZE * TC_DATA_SIZE]);


/** 
 * @brief Function for finding and deleting records within a file
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_find_and_delete(uint32_t read_file_id, uint32_t read_record_key);


/** 
 * @brief Function for initializing the FDS
 * 
 * @return      NRF_SUCCESS if successful, else error code
 */
ret_code_t fds_storage_init(void);


/** 
 * @brief Function for getting the write flag
 * 
 * @return      Boolean indicating the write status
 */
bool fds_getWriteFlag(void);


/** 
 * @brief Function for setting the write flag
 * 
 * @param[in] fds_write_flag        Boolean indicating the write status
 */
void fds_setWriteFlag(bool fds_write_flag);


/** 
 * @brief Function for getting all records deleted flag
 * 
 * @return      Boolean indicating the flag status
 */
bool fds_getAllRecordsDeletedFlag(void);


/** 
 * @brief Function for setting the all records deleted flag
 * 
 * @param[in] fds_all_records_deleted_flag        Boolean indicating the flag status
 */
void fds_setAllRecordsDeletedFlag(bool fds_all_records_deleted_flag);


/** 
 * @brief Function for getting the number of found records
 * 
 * @return      16 bit initeger indicating the number of records found
 */
uint16_t fds_getNumberOfRecords(void);


/** 
 * @brief Function for setting the number of found records
 * 
 * @param[in] number_of_records     16 bit initeger indicating the number of records found
 */
void fds_setNumberOfRecords(uint16_t number_of_records);



#endif // _storage_H_