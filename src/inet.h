/**
*	@file inet.h
*	Network library (byte-stream sockets)
*
*	@author	Nicholas Christopoulos
*/

/**
*	@defgroup net Network
*/

#if !defined(_inet_ndclib_h)
#define _inet_ndclib_h

#include "sys.h"
#if defined(_PalmOS)
    #include <NetMgr.h>

    typedef NetSocketRef     socket_t;
#else
	#include <string.h>
	#include <sys/types.h>
	#include <errno.h>

    #if defined(_Win32) || defined(__MINGW32__)
		#include <winsock2.h>	// @#@!$@#!$ it uses 'byte'
        #undef V_ARRAY /* defined in oleauto.h in mingw build */
	#elif defined(_UnixOS)
		#include <sys/socket.h>
		#include <netinet/in.h>
		#include <sys/param.h>
		#include <netdb.h>
		#include <arpa/inet.h>
	#elif defined(_DOS)
	//#define _DOSTCP_ENABLE
		#if defined(_DOSTCP_ENABLE)
		#include <netinet/in.h>
		#include <socket.h>
		#define INADDR_NONE    NULL
		#endif
	#endif

    typedef int     socket_t;
#endif

/**
*	@ingroup net
*
*	initialize network driver
*
*	@return non-zero on success
*/
int     net_init(void)                                              SEC(BIO);

/**
*	@ingroup net
*
*	deinitialize network driver
*
*	@return non-zero on success
*/
int     net_close(void)                                             SEC(BIO);

/**
*	@ingroup net
*
*	writes a string to a socket
*
*	@param s the socket
*	@param str the string
*/
void    net_print(socket_t s, const char *str)                      SEC(BIO);

/**
*	@ingroup net
*
*	writes a string to a socket using printf-style
*
*	@param s the socket
*	@param fmt the format
*	@param ... the format's parameters
*/
void    net_printf(socket_t s, const char *fmt, ...)                SEC(BIO);

/**
*	@ingroup net
*
*	reads a string from a socket until the size > from the size of the buffer
*	or until one characters of 'delim' string found
*
*	@note character \r will ignored
*
*	@param s the socket
*	@param buf a buffer to store the string
*	@param size the size of the buffer
*	@param delim the characters that terminates the string
*	@return the number of the bytes that read
*/
int		net_input(socket_t s, char *buf, int size, const char *delim)   SEC(BIO);

/**
*	@ingroup net
*
*	read the specified number of bytes from the socket
*
*	@param s the socket
*	@param buf a buffer to store the string
*	@param size the size of the buffer
*	@return the number of the bytes that read
*/
int		net_read(socket_t s, char *buf, int size)   SEC(BIO);

/**
*	@ingroup net
*
*	connect to a server
*
*	@param server_name the server's IP or domain-name
*	@param server_port the port to connect
*	@return on success the socket; otherwise -1
*/
socket_t  net_connect(const char *server_name, int server_port)     SEC(BIO);

/**
*	@ingroup net
*
*	disconnect
*
*	@param s the socket
*/
void	net_disconnect(socket_t s)                                  SEC(BIO);

/**
*	@ingroup net
*
*	returns true if something is waitting in input-buffer
*
*	@param s the socket
*	@return non-zero if something is waitting in input-buffer; otherwise returns 0
*/
int		net_peek(socket_t s)										SEC(BIO);

#endif


