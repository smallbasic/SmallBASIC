// This file is part of SmallBASIC
//
// Network library (byte-stream sockets)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#if !defined(INET_H)
#define INET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <sys/types.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef int socket_t;

/**
 * @ingroup net
 *
 * initialize network driver
 *
 * @return non-zero on success
 */
int net_init(void);

/**
 * @ingroup net
 *
 * deinitialize network driver
 *
 * @return non-zero on success
 */
int net_close(void);

/**
 * @ingroup net
 *
 * writes a NULL terminated string to a socket
 *
 * @param s the socket
 * @param str the string
 */
void net_print(socket_t s, const char *str);

/**
 * @ingroup net
 *
 * writes a string to a socket using printf-style
 *
 * @param s the socket
 * @param fmt the format
 * @param ... the format's parameters
 */
void net_printf(socket_t s, const char *fmt, ...);

/**
 * @ingroup net
 *
 * writes data to a socket
 *
 * @param s the socket
 * @param str the string
 */
void net_send(socket_t s, const char *str, size_t size);

/**
 * @ingroup net
 *
 * reads a string from a socket until the size > from the size of the buffer
 * or until one characters of 'delim' string found
 *
 * @note character \r will ignored
 *
 * @param s the socket
 * @param buf a buffer to store the string
 * @param size the size of the buffer
 * @param delim the characters that terminates the string
 * @return the number of the bytes that read
 */
int net_input(socket_t s, char *buf, int size, const char *delim);

/**
 * @ingroup net
 *
 * read the specified number of bytes from the socket
 *
 * @param s the socket
 * @param buf a buffer to store the string
 * @param size the size of the buffer
 * @return the number of the bytes that read
 */
int net_read(socket_t s, char *buf, int size);

/**
 * @ingroup net
 *
 * connect to a server
 *
 * @param server_name the server's IP or domain-name
 * @param server_port the port to connect
 * @return on success the socket; otherwise -1
 */
socket_t net_connect(const char *server_name, int server_port);

/**
 * @ingroup net
 *
 * listen on a port number like a server
 *
 * @param server_port the port to listen
 * @return on success the socket; otherwise -1
 */
socket_t net_listen(int server_port);

/**
 * @ingroup net
 *
 * disconnect
 *
 * @param s the socket
 */
void net_disconnect(socket_t s);

/**
 * @ingroup net
 *
 * returns true if something is waiting in input-buffer
 *
 * @param s the socket
 * @return non-zero if something is waiting in input-buffer; otherwise returns 0
 */
int net_peek(socket_t s);

#if defined(__cplusplus)
}
#endif

#endif
