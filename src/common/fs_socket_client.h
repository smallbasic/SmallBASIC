// This file is part of SmallBASIC
//
// BSD sockets driver (byte-stream client)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(FS_SOCKET_CLIENT_H)
#define FS_SOCKET_CLIENT_H

#if defined(__cplusplus)
extern "C" {
#endif

int sockcl_open(dev_file_t *f);
int sockcl_close(dev_file_t *f);
int sockcl_write(dev_file_t *f, byte *data, uint32_t size);
int sockcl_read(dev_file_t *f, byte *data, uint32_t size);
int sockcl_eof(dev_file_t *f);
int sockcl_length(dev_file_t *f);
int http_open(dev_file_t *f);
int http_read(dev_file_t *f, var_t *var_p);

#if defined(__cplusplus)
}
#endif

#endif
