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
	
	(long) f->handle = net_connect(server, port);
	if	( (long) f->handle <= 0 )	{
		f->handle = -1;
    f->drv_dw[0] = 0;
		//rt_raise("SOCL: CONNECTION ERROR");
		return 0;
		}

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


