/*
*   BSD sockets driver (byte-stream client)
*
*   Nicholas Christopoulos
*/

/*
*   PALMOS: Avoid to initialize netlib at startup because it uses a lot of memory
*/

#include "inet.h"
#include "fs_socket_client.h"
#include "sberr.h"
#include <time.h>

void trace(const char *format, ...);
#if !defined(_PalmOS)
    #if defined(_VTOS)
    typedef FILE *  FileHand;
    #else
    typedef long    FileHand;
    #endif
#endif

int sockcl_open(dev_file_t *f) {
    #if defined(_VTOS)
    err_unsup();
    return 0;
    #else
    char    *p;
    int     port;
    char    server[129];

    p = strchr(f->name+5, ':');
    if  ( !p )  {
        rt_raise("SOCL: NO PORT IS SPECIFIED");
        return 0;
        }
    *p = '\0';
    strcpy(server, f->name+5);
    *p = ':';
    port = xstrtol(p+1);
    f->drv_dw[0] = 1;
    
    f->handle = (int)net_connect(server, port);
    if  (f->handle <= 0 )   {
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

    char host[250];
    char txbuf[1024];
    f->port = 0;

    // check for http://
    if (0 != strncasecmp(f->name, "http://", 7)) {
        rt_raise("HTTP: INVALID URL");
        return 0;
    }

    // check for end of host delimeter
    char* colon = strchr(f->name+7, ':');
    char* slash = strchr(f->name+7, '/');
    char* lastSlash;

    // saves the length of the path component in f->drv_dw[1]
    if (colon) {
        // http://host:port/resource or http://host:port
        if (slash) {
            *slash = 0;
            f->port = xstrtol(colon+1);
            *slash = '/';
            lastSlash = strrchr(slash, '/');
            f->drv_dw[1] = lastSlash ? lastSlash-f->name: slash-f->name;
        } else {
            f->port = xstrtol(colon+1);
            f->drv_dw[1] = strlen(f->name);
        }
        *colon = 0;
        strcpy(host, f->name+7);
        *colon = ':';
    } else if (slash) {
        // http://host/resource or http://host/
        *slash = 0;
        strcpy(host, f->name+7);
        *slash = '/';
        lastSlash = strrchr(slash, '/');
        f->drv_dw[1] = lastSlash ? lastSlash-f->name: slash-f->name;
    } else {
        // http://host
        strcpy(host, f->name+7);
        f->drv_dw[1] = strlen(f->name);
    }

    f->drv_dw[0] = 1;
    if (f->port == 0) {
        f->port = 80;
    }
  
    socket_t s = net_connect(host, f->port);
    f->handle = (socket_t)s;

    if (f->handle <= 0) {
        f->handle = -1;
        f->drv_dw[0] = 0;
        f->port = 0;
        return 0;
    }

    sprintf(txbuf, 
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Accept: */*\r\n"
            "Accept-Language: en-au\r\n"
            "User-Agent: SmallBASIC\r\n", 
            slash?slash:"/", host);
    if (f->drv_dw[2]) {
        // If-Modified-Since: Sun, 03 Apr 2005 04:45:47 GMT
        strcat(txbuf, "If-Modified-Since: ");
        strftime(txbuf+strlen(txbuf), 60, "%a, %d %b %Y %T %Z\r\n", 
                 localtime((time_t*)&f->drv_dw[2]));
    }
    strcat(txbuf, "\r\n");
    net_print(s, txbuf);
    return 1;
    #endif
}

int sockcl_close(dev_file_t *f) {
    net_disconnect((socket_t) (long) f->handle);
    f->drv_dw[0] = 0;
    f->handle = -1;
    return 1;
}

/*
*   write to a socket
*/
int sockcl_write(dev_file_t *f, byte *data, dword size) {
    net_print((socket_t) (long) f->handle, data);
    return size;
}

/*
*   read from a socket
*/
int sockcl_read(dev_file_t *f, byte *data, dword size) {
    f->drv_dw[0] = (dword) net_input((socket_t) (long) f->handle, data, size, NULL);
    return (((long) f->drv_dw[0]) <= 0) ? 0 : (long) f->drv_dw[0];
}

/*
*   Returns true (EOF) if the connection is broken
*/
int sockcl_eof(dev_file_t *f) {
  return (((long) f->drv_dw[0]) <= 0) ? 1 : 0;
}

/*
*   returns the size of the data which are waiting in stream's queue
*/
int sockcl_length(dev_file_t *f) {
    return net_peek((socket_t) (long) f->handle);
}


