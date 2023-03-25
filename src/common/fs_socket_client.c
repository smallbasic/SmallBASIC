// This file is part of SmallBASIC
//
// BSD sockets driver (byte-stream client)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/inet.h"
#include "common/device.h"
#include "common/fs_socket_client.h"
#include "common/sberr.h"
#include <time.h>

int sockcl_open(dev_file_t *f) {
  // open "SOCL:smallbasic.sf.net:80" as #1
  // open "SOCL:80" as #2
  f->drv_dw[0] = 1;
  char *p = strchr(f->name + 5, ':');
  if (!p) {
    int port = xstrtol(f->name + 5);
    f->handle = (int) net_listen(port);
  } else {
    *p = '\0';
    char server[255];
    strlcpy(server, f->name + 5, sizeof(server));
    *p = ':';
    int port = xstrtol(p + 1);
    f->handle = (int) net_connect(server, port);
  }

  if (f->handle <= 0) {
    f->handle = -1;
    f->drv_dw[0] = 0;
    return 0;
  }

  return 1;
}

//
// open a web server connection
//
int http_open(dev_file_t *f) {
  char host[250];
  char txbuf[1024];
  f->port = 0;

  // check for http://
  if (0 != strncasecmp(f->name, "http://", 7)) {
    rt_raise("HTTP: INVALID URL");
    return 0;
  }

  // check for end of host delimeter
  char *colon = strchr(f->name + 7, ':');
  char *slash = strchr(f->name + 7, '/');
  char *lastSlash;

  // saves the length of the path component in f->drv_dw[1]
  if (colon) {
    // http://host:port/resource or http://host:port
    if (slash) {
      *slash = 0;
      f->port = xstrtol(colon + 1);
      *slash = '/';
      lastSlash = strrchr(slash, '/');
      f->drv_dw[1] = lastSlash ? lastSlash - f->name : slash - f->name;
    } else {
      f->port = xstrtol(colon + 1);
      f->drv_dw[1] = strlen(f->name);
    }
    *colon = 0;
    strcpy(host, f->name + 7);
    *colon = ':';
  } else if (slash) {
    // http://host/resource or http://host/
    *slash = 0;
    strcpy(host, f->name + 7);
    *slash = '/';
    lastSlash = strrchr(slash, '/');
    f->drv_dw[1] = lastSlash ? lastSlash - f->name : slash - f->name;
  } else {
    // http://host
    strlcpy(host, f->name + 7, sizeof(host));
    f->drv_dw[1] = strlen(f->name);
  }

  f->drv_dw[0] = 1;
  if (f->port == 0) {
    f->port = 80;
  }

  socket_t s = net_connect(host, f->port);
  f->handle = (socket_t) s;

  if (f->handle <= 0) {
    f->handle = -1;
    f->drv_dw[0] = 0;
    f->port = 0;
    return 0;
  }

  sprintf(txbuf, "GET %s HTTP/1.0\r\n"
          "Host: %s\r\n"
          "Accept: */*\r\n"
          "Accept-Language: en-au\r\n"
          "User-Agent: SmallBASIC\r\n", slash ? slash : "/", host);
  if (f->drv_dw[2]) {
    // If-Modified-Since: Sun, 03 Apr 2005 04:45:47 GMT
    strcat(txbuf, "If-Modified-Since: ");
    strftime(txbuf + strlen(txbuf), 60, "%a, %d %b %Y %H:%M:%S %Z\r\n",
             localtime((time_t *) &f->drv_dw[2]));
  }
  strcat(txbuf, "\r\n");
  net_print(s, txbuf);
  return 1;
}

//
// read from a web server connection
//
int http_read(dev_file_t *f, var_t *var_p) {
  static const char *delim = "\r\n\r\n";
  char rxbuff[1024];
  int inContent = 0;
  int httpOK = 0;

  v_setint(var_p, 0);

  while (1) {
    int bytes = net_read(f->handle, (char *)rxbuff, sizeof(rxbuff));
    if (bytes == -1) {
      httpOK = 0;
      break;
    } else if (bytes == 0) {
      // no more data
      break;
    }
    // assumes http header < 1024 bytes
    if (inContent) {
      if (var_p->type == V_INT) {
        v_free(var_p);
        var_p->type = V_STR;
        var_p->v.p.length = bytes;
        var_p->v.p.ptr = malloc(var_p->v.p.length + 1);
        var_p->v.p.owner = 1;
        memcpy(var_p->v.p.ptr, rxbuff, var_p->v.p.length);
        var_p->v.p.ptr[var_p->v.p.length] = '\0';
      } else {
        var_p->v.p.ptr = realloc(var_p->v.p.ptr, var_p->v.p.length + bytes + 1);
        memcpy(var_p->v.p.ptr + var_p->v.p.length, rxbuff, bytes);
        var_p->v.p.length += bytes;
        var_p->v.p.ptr[var_p->v.p.length] = '\0';
      }
    } else {
      int i = 0;
      int countNL = 0;
      while (i < bytes && rxbuff[i] != 0 && countNL != 4) {
        // scan for CR + LF + CR + LF
        if (rxbuff[i] == delim[countNL]) {
          countNL++;
        } else {
          countNL = 0;
        }
        i++;
      }
      if (countNL == 4) {
        // found start of content
        if (i < bytes) {
          // copy remaining characters from rxbuff
          v_free(var_p);
          var_p->type = V_STR;
          var_p->v.p.length = bytes - i;
          var_p->v.p.ptr = malloc(var_p->v.p.length + 1);
          var_p->v.p.owner = 1;
          memcpy(var_p->v.p.ptr, rxbuff + i, var_p->v.p.length);
          var_p->v.p.ptr[var_p->v.p.length] = '\0';
        }
        inContent = 1;
      }
      // null terminate headers fragment
      rxbuff[i - 1] = '\0';
      if (strstr(rxbuff, "200 OK") != 0) {
        httpOK = 1;
      }
      char *location = strstr(rxbuff, "Location: ");
      if (location) {
        // handle redirection
        char *cr = strstr(location, "\r");
        if (cr) {
          *cr = '\0';
          sockcl_close(f);
          strlcpy(f->name, location + 10, sizeof(f->name));
          if (http_open(f) == 0) {
            httpOK = 0;
            break;
          }
          inContent = 0;
          v_setint(var_p, 0);
        }
      }
    }
  }
  return httpOK;
}

int sockcl_close(dev_file_t *f) {
  net_disconnect((socket_t) (long) f->handle);
  f->drv_dw[0] = 0;
  f->handle = -1;
  return 1;
}

//
// write to a socket
//
int sockcl_write(dev_file_t *f, byte *data, uint32_t size) {
  net_send((socket_t) (long) f->handle, (char *)data, size);
  return size;
}

//
// read from a socket
//
int sockcl_read(dev_file_t *f, byte *data, uint32_t size) {
  int result;
  if (f->handle != -1) {
    f->drv_dw[0] = (uint32_t) net_input((socket_t) (long) f->handle, (char *)data, size, NULL);
    result = (((long) f->drv_dw[0]) <= 0) ? 0 : (long) f->drv_dw[0];
  } else {
    err_network();
    data[0] = 0;
    result = 0;
  }
  return result;
}

//
// Returns true (EOF) if the connection is broken
//
int sockcl_eof(dev_file_t *f) {
  return (((long) f->drv_dw[0]) <= 0) ? 1 : 0;
}

//
// returns the size of data waiting in stream's queue
//
int sockcl_length(dev_file_t *f) {
  return net_peek((socket_t) (long) f->handle);
}
