/*
*	serial I/O, driver
*
*	Nicholas Christopoulos
*/

#if !defined(_sbfs_memo_h)
#define _sbfs_memo_h

#include "sys.h"
#include "device.h"

int		memo_mount(void)                                   SEC(BIO);
int		memo_umount(void)                                  SEC(BIO);
int		memo_open(dev_file_t *f)                           SEC(BIO);
int		memo_close(dev_file_t *f)                          SEC(BIO);
int		memo_write(dev_file_t *f, byte *data, dword size)  SEC(BIO);
int		memo_read(dev_file_t *f, byte *data, dword size)   SEC(BIO);
dword	memo_length(dev_file_t *f)                         SEC(BIO);
dword	memo_eof(dev_file_t *f)                            SEC(BIO);
int		memo_delete(const char *name)                      SEC(BIO);
int		memo_exist(const char *name)                       SEC(BIO);
dword   memo_seek(dev_file_t *f, dword offset)             SEC(BIO);
char_p_t	*memo_create_file_list(const char *wc, int *count) SEC(BIO);
int		memo_access(const char *name)                      SEC(BIO);
int		memo_fattr(const char *name)                      SEC(BIO);

#endif

