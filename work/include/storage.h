
#ifndef _storage_H__
#define _storage_H__

#define RECORD_SIZE 4
#define MAX_RECORDS 144 ///< 1 dag, elke 10m

/** 
 * @brief Function for writing to the FDS
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * @param[in] p_write_data              Pointer to the data container
 * 
 * @param[out] ret_code_t               Return status code
 */
ret_code_t fds_write(uint32_t write_file_id, uint32_t write_record_key, const uint8_t* p_write_data);


/** 
 * @brief Function for reading from the FDS
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * @param[in] p_read_data               Pointer to the data container
 * 
 * @param[out] ret_code_t               Return status code
 */
ret_code_t fds_read(uint32_t read_file_id, uint32_t read_record_key, uint8_t (*p_read_data)[RECORD_SIZE]);


/** 
 * @brief Function for finding and deleting records within a file
 * 
 * @param[in] write_file_id             ID of the file to write
 * @param[in] write_record key          Key of the record to write to
 * 
 * @param[out] ret_code_t               Return status code
 */
ret_code_t fds_find_and_delete(uint32_t read_file_id, uint32_t read_record_key);


/** 
 * @brief Function for initializing the FDS
 * 
 * @param[out] ret_code_t               Return status code
 */
ret_code_t fds_storage_init(void);


/** 
 * @brief Function for getting the write flag
 * 
 * @param[out] bool                 Boolean indicating the write status
 */
bool fds_getWriteFlag(void);


/** 
 * @brief Function for setting the write flag
 * 
 * @param[in] fds_write_flag        Boolean indicating the write status
 */
void fds_setWriteFlag(bool fds_write_flag);


/** 
 * @brief Function for getting the number of found records
 * 
 * @param[out] unint16_t            16 bit initeger indicating the number of records found
 */
uint16_t fds_getNumberOfRecords(void);


/** 
 * @brief Function for setting the number of found records
 * 
 * @param[in] number_of_records     16 bit initeger indicating the number of records found
 */
void fds_setNumberOfRecords(uint16_t number_of_records);



#endif // _storage_H_