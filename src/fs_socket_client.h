/*
*	BSD sockets driver (byte-stream client)
*
*	Nicholas Christopoulos
*/

#if !defined(_fs_socket_client_h)
#define _fs_socket_client_h
#include "inet.h"
#include "device.h"

int		sockcl_open(dev_file_t *f)                              SEC(BIO);
int		sockcl_close(dev_file_t *f)                             SEC(BIO);
int		sockcl_write(dev_file_t *f, byte *data, dword size)     SEC(BIO);
int		sockcl_read(dev_file_t *f, byte *data, dword size)      SEC(BIO);
int 	sockcl_eof(dev_file_t *f)                               SEC(BIO);
int		sockcl_length(dev_file_t *f)							SEC(BIO);

#endif

