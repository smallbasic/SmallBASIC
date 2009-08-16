// $Id: inet.c 681 2009-08-14 12:59:38Z zeeb90au $
// This file is part of SmallBASIC
//
// Network library (byte-stream sockets)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "inet.h"
#include "device.h"

static word netlib;
static int inetlib_init = 0;

/**
 * prepare to use the network
 */
int net_init()
{
  Err err;
  word ifer = 0;

  if (inetlib_init) {
    inetlib_init++;
    return 1;
  }

  err = SysLibFind("Net.lib", &netlib);
  if (err) {
    return 0;
  }

  err = NetLibOpen(netlib, &ifer);
  if (ifer) {
    NetLibClose(netlib, 1);
    return 0;
  }

  if (err) {
    return 0;
  }
  inetlib_init = 1;
  return 1;
}

/**
 * stop using the network
 */
int net_close()
{
  inetlib_init--;
  if (inetlib_init <= 0) {
    NetLibClose(netlib, 1);
    inetlib_init = 0;
  }
  return 1;
}

/**
 * sends a string to socket
 */
void net_print(socket_t s, const char *str)
{
  Err err;

  NetLibSend(netlib,
             s, (UInt8 *) str, strlen(str),
             0 /* flags */ , 0, 0, 5 * SysTicksPerSecond(), &err);
}

/**
 * sends a string to socket
 */
void net_printf(socket_t s, const char *fmt, ...)
{
  char buf[1025];
  va_list argp;

  va_start(argp, fmt);
  StrVPrintF(buf, fmt, argp);
  va_end(argp);
  net_print(s, buf);
}

/**
 * read the specified number of bytes from the socket
 */
int net_read(socket_t s, char *buf, int size)
{
  Err err;
  int bytes = NetLibReceive(netlib,
                            s, (UInt8 *) buf, size,
                            0, 0, 0, 5 * SysTicksPerSecond(), &err);
  if (err) {
    return 0;
  }
}

/**
 * read a string from a socket until a char from delim str found.
 */
int net_input(socket_t s, char* buf, int size, const char* delim)
{
  Err err;

  memset(buf, 0, size);
  while (count < size) {
    bytes = NetLibReceive(netlib,
                          s, (UInt8 *) & ch, 1,
                          0 /* flags */ , 0, 0, 5 * SysTicksPerSecond(), &err);
    if (err) {
      return 0;
    }
    bytes = net_read(s, &ch, 1);
    if (bytes <= 0) {
      return count;             // no more data
    }
    else {
      if (ch == 0) {
        return count;
      }
      if (delim) {
        if ((strchr(delim, ch) != NULL)) {
          return count;         // delimiter found
        }
      }
      if (ch != '\015') {       // ignore it
        buf[count] = ch;
        count += bytes;         // actually ++
      }
    }
  }

  return count;
}

/**
 * return true if there something waiting
 */
int net_peek(socket_t s)
{
  int bytes;
  byte ch;
  Err err;

  bytes = NetLibReceive(netlib,
                        s, (UInt8 *) & ch, 1,
                        netIOFlagPeek, 0, 0, SysTicksPerSecond(), &err);
  if (err) {
    return 0;
  }
  return (bytes > 0);
}

/**
 * connect to server and returns the socket
 */
socket_t net_connect(const char *server_name, int server_port)
{
  socket_t sock;
  NetIPAddr inaddr;
  Err err;
  NetHostInfoBufType hibt;
  NetSocketAddrINType addr;

  net_init();

  if ((inaddr = NetLibAddrAToIN(netlib, (char *)server_name)) == -1) {
    NetLibGetHostByName(netlib, (char *)server_name, &hibt, 5 * SysTicksPerSecond(),
                        &err);
    if (err != 0) {
      return -1;
    }
    else {
      addr.addr = NetHToNL(hibt.address[0]);
      // memcpy(&addr.addr, hp->addrListP, hp->addrLen);
    }

  }
  memcpy(&addr.addr, &inaddr, sizeof(inaddr));
  addr.port = server_port;
  sock = NetLibSocketOpen(netlib, netSocketAddrINET,
                          netSocketTypeStream, 0, 5 * SysTicksPerSecond(), &err);

  if (sock <= 0) {
    return sock;
  }
  if (NetLibSocketConnect(netlib, sock, (NetSocketAddrType *) & addr,
                          sizeof(addr), 5 * SysTicksPerSecond(), &err) < 0) {
    return -1;
  }
  return sock;
}

/**
 * listen for an incoming connection on the given port and 
 * returns the socket once a connection has been established
 */
socket_t net_listen(int server_port)
{
  return -1;
}

/**
 * disconnect the given network connection
 */
void net_disconnect(socket_t s)
{
  Err err;
  NetLibSocketClose(netlib, s, 200, &err);
  net_close();
}

/* End of "$Id: inet.c 681 2009-08-14 12:59:38Z zeeb90au $". */
