// This file is part of SmallBASIC
//
// Network library (byte-stream sockets)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "inet.h"

#if defined(INET_UNSUP)
 int net_init(void) { return 0; }
 int net_close(void) { return 0; }
 void net_print(socket_t s, const char *str) {}
 void net_printf(socket_t s, const char *fmt, ...) {}
 int net_input(socket_t s, char *buf, int size, const char *delim) { return 0; }
 int net_read(socket_t s, char *buf, int size) { return 0; }
 socket_t net_connect(const char *server_name, int server_port) { return 0; }
 socket_t net_listen(int server_port) { return 0; }
 void net_disconnect(socket_t s) {}
 int net_peek(socket_t s) { return 0; }
#elif defined(_UnixOS)
 #include "inet2.c"
#endif

