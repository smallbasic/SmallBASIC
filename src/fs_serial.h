/*
*	serial I/O, driver
*
*	Nicholas Christopoulos
*/

#if !defined(_sbfs_serial_h)
#define _sbfs_serial_h

#include "sys.h"
#include "device.h"

int		serial_open(dev_file_t *f)                            SEC(BIO);
int		serial_close(dev_file_t *f)                           SEC(BIO);
int		serial_write(dev_file_t *f, byte *data, dword size)   SEC(BIO);
int		serial_read(dev_file_t *f, byte *data, dword size)    SEC(BIO);
dword	serial_length(dev_file_t *f)                          SEC(BIO);
dword	serial_eof(dev_file_t *f)                             SEC(BIO);

#endif

