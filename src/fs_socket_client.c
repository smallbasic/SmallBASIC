/*
*	BSD sockets driver (byte-stream client)
*
*	Nicholas Christopoulos
*/

/*
*	PALMOS:	Avoid to initialize netlib at startup because it uses a lot of memory
*/

#include "inet.h"
#include "fs_socket_client.h"
#include "sberr.h"

#if !defined(_PalmOS)
	#if defined(_VTOS)
	typedef FILE *	FileHand;
	#else
	typedef long	FileHand;
	#endif
#endif

int		sockcl_open(dev_file_t *f)
{
	#if defined(_VTOS)
	err_unsup();
	return 0;
	#else
	char	*p;
	int		port;
	char	server[129];

	p = strchr(f->name+5, ':');
	if	( !p )	{
		rt_raise("SOCL: NO PORT IS SPECIFIED");
		return 0;
		}
	*p = '\0';
	strcpy(server, f->name+5);
	*p = ':';
	port = xstrtol(p+1);
	f->drv_dw[0] = 1;
	
	f->handle = (int)net_connect(server, port);
	if	(f->handle <= 0 )	{
		f->handle = -1;
        f->drv_dw[0] = 0;
		//rt_raise("SOCL: CONNECTION ERROR");
		return 0;
    }

	return 1;
	#endif
}

int http_open(dev_file_t *f) {
	#if defined(_VTOS)
	err_unsup();
	return 0;
	#else

	int port = 0;
	char host[250];

    // check for http://
    if (0 != strncmpi(f->name, "http://", 7)) {
        rt_raise("HTTP: INVALID URL");
        return 0;
    }

    // check for end of host delimeter
	char* colon = strchr(f->name+7, ':');
    char* slash = strchr(f->name+7, '/');

    if (colon) {
        // http://host:port/resource or http://host:port
        if (slash) {
            *slash = 0;
            port = xstrtol(colon+1);
            *slash = '/';
        } else {
            port = xstrtol(colon+1);
        }

        *colon = 0;
        strcpy(host, f->name+7);
        *colon = ':';
    } else if (slash) {
        // http://host/resource or http://host/
        *slash = 0;
        strcpy(host, f->name+7);
        *slash = '/';
    } else {
        // http://host
        strcpy(host, f->name+7);
    }

    f->drv_dw[0] = 1;
    if (port == 0) {
        port = 80;
    }
  
    f->handle = (int)net_connect(host, port);
    if (f->handle <= 0) {
        f->handle = -1;
        f->drv_dw[0] = 0;
        return 0;
    }

	net_print((socket_t)(long)f->handle, "GET ");
    net_print((socket_t)(long)f->handle, slash?slash:"/");
	net_print((socket_t)(long)f->handle, "\n");
    
	return 1;
	#endif
}

int		sockcl_close(dev_file_t *f)
{
	net_disconnect((socket_t) (long) f->handle);
	f->drv_dw[0] = 0;
	f->handle = -1;
	return 1;
}

/*
*	write to a socket
*/
int		sockcl_write(dev_file_t *f, byte *data, dword size)
{
	net_print((socket_t) (long) f->handle, data);
	return size;
}

/*
*	read from a socket
*/
int		sockcl_read(dev_file_t *f, byte *data, dword size)
{
	f->drv_dw[0] = (dword) net_input((socket_t) (long) f->handle, data, size, NULL);
	return (((long) f->drv_dw[0]) <= 0) ? 0 : 1;
}

/*
*	Returns true (EOF) if the connection is broken
*/
int		sockcl_eof(dev_file_t *f)
{
  return (((long) f->drv_dw[0]) <= 0) ? 1 : 0;
}

/*
*	returns the size of the data which are waiting in stream's queue
*/
int		sockcl_length(dev_file_t *f)
{
	return net_peek((socket_t) (long) f->handle);
}


