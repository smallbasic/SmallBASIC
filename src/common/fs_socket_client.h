// This file is part of SmallBASIC
//
// BSD sockets driver (byte-stream client)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(_fs_socket_client_h)
#define _fs_socket_client_h

int sockcl_open(dev_file_t * f) SEC(BIO);
int sockcl_close(dev_file_t * f) SEC(BIO);
int sockcl_write(dev_file_t * f, byte * data, dword size)
SEC(BIO);
int sockcl_read(dev_file_t * f, byte * data, dword size)
SEC(BIO);
int sockcl_eof(dev_file_t * f) SEC(BIO);
int sockcl_length(dev_file_t * f) SEC(BIO);
int http_open(dev_file_t * f) SEC(BIO);
int http_read(dev_file_t * f, var_t * var_p, int type)
SEC(BIO);

#endif
