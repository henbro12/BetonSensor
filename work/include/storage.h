
#ifndef _storage_H__
#define _storage_H__

ret_code_t fds_write(void);
ret_code_t fds_read(void);
ret_code_t fds_find_and_delete(void);
ret_code_t fds_storage_init(void);

bool fds_getWriteFlag(void);
void fds_setWriteFlag(bool fds_write_flag);

#endif // _storage_H_