/*
 * stream-files, driver
 *
 * Nicholas Christopoulos
 */

#if !defined(_sbfs_stream_h)
#define _sbfs_stream_h

#include "sys.h"
#include "device.h"

int stream_open(dev_file_t * f) SEC(BIO);
int stream_close(dev_file_t * f) SEC(BIO);
int stream_write(dev_file_t * f, byte * data, dword size) SEC(BIO);
int stream_read(dev_file_t * f, byte * data, dword size) SEC(BIO);
dword stream_tell(dev_file_t * f) SEC(BIO);
dword stream_length(dev_file_t * f) SEC(BIO);
dword stream_seek(dev_file_t * f, dword offset) SEC(BIO);
int stream_eof(dev_file_t * f) SEC(BIO);

#endif
