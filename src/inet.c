/*
 * $Id: inet.c,v 1.11 2006/08/03 10:30:53 zeeb90au Exp $
 *   Network library (byte-stream sockets)
 *
 *   Nicholas Christopoulos
 */

#include "sys.h"
#include "inet.h"
#include "device.h"

#if defined(_UnixOS) && !defined(__MINGW32__)
#include <sys/ioctl.h>
#endif

#if defined(_PalmOS)
static word netlib;
#endif
#if defined(_Win32) || defined(_PalmOS) || defined(__MINGW32__)
static int inetlib_init = 0;
#endif

#ifndef socklen_t
#define socklen_t int
#endif

/*
 */
int net_init()
{
#if defined(_PalmOS)
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
#elif defined(_Win32) || defined(__MINGW32__)
  WSADATA wsadata;

  if (inetlib_init) {
    inetlib_init++;
    return 1;
  }

  if (WSAStartup(MAKEWORD(2, 0), &wsadata)) {
    return 0;
  }
  inetlib_init = 1;
  return 1;
#elif defined(_UnixOS)
  return 1;
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  SocketInit();
  return 0;
#else
  return 1;
#endif
#else
  return 0;
#endif
}

/*
 */
int net_close()
{
#if defined(_PalmOS)
  inetlib_init--;
  if (inetlib_init <= 0) {
    NetLibClose(netlib, 1);
    inetlib_init = 0;
  }
  return 1;
#elif defined(_Win32) || defined(__MINGW32__)
  inetlib_init--;
  if (inetlib_init <= 0) {
    WSACleanup();
    inetlib_init = 0;
  }
  return 1;
#elif defined(_UnixOS)
  return 1;
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  SocketClose();
  return 0;
#else
  return 1;
#endif
#else
  return 0;
#endif
}

/*
 *   sends a string to socket
 */
void net_print(socket_t s, const char *str)
{
#if defined(_PalmOS)
  Err err;

  NetLibSend(netlib,
             s, (UInt8 *) str, strlen(str),
             0 /* flags */ , 0, 0, 5 * SysTicksPerSecond(), &err);
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  write_s(s, (byte *) str, strlen(str));
#endif
#elif defined(_VTOS)
  // do nothing
#else
  send(s, str, strlen(str), 0);
#endif
}

/*
 *   sends a string to socket
 */
void net_printf(socket_t s, const char *fmt, ...)
{
  char buf[1025];
  va_list argp;

  va_start(argp, fmt);
#if defined(_PalmOS)
  StrVPrintF(buf, fmt, argp);
#elif defined(_DOS) || defined(_Win32) || defined(_VTOS)
  vsprintf(buf, fmt, argp);
#else
  vsnprintf(buf, 1024, fmt, argp);
#endif
  va_end(argp);
  // strcat(buff, "\r\n"); 
  net_print(s, buf);
}

/*
 *  read the specified number of bytes from the socket
 */
int net_read(socket_t s, char *buf, int size)
{
#if defined(_VTOS)
  buf[0] = '\0';
  return 0;
#elif defined(_PalmOS)
  Err err;
  int bytes = NetLibReceive(netlib,
                            s, (UInt8 *) buf, size,
                            0, 0, 0, 5 * SysTicksPerSecond(), &err);
  if (err) {
    return 0;
  }
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  return read_s(s, buf, size);
#else
  return = -1;
#endif
#else
  fd_set readfds;
  struct timeval tv;
  int rv;

  // clear the set
  FD_ZERO(&readfds);

  tv.tv_sec = 0;                // block at 1 second intervals
  tv.tv_usec = 250000;
  while (1) {
    FD_SET(s, &readfds);
    rv = select(s + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
      // an error occured
      return 0;
    }
    else if (rv == 0) {
      // timeout occured
      if (0 != dev_events(0)) {
        s = 0;
        break;
      }
    }
    else {
      // ready to read
      return recv(s, buf, size, 0);
    }
  }
  return 0;
#endif
}

/*
 *   read a string from a socket until a char from delim str found.
 */
int net_input(socket_t s, char *buf, int size, const char *delim)
{
#if defined(_VTOS)
  buf[0] = '\0';
  return 0;
#else

  int count = 0, bytes;
  char ch;
#if defined(_PalmOS)
  Err err;
#endif

  memset(buf, 0, size);
  while (count < size) {
#if defined(_PalmOS)
    bytes = NetLibReceive(netlib,
                          s, (UInt8 *) & ch, 1,
                          0 /* flags */ , 0, 0, 5 * SysTicksPerSecond(), &err);
    if (err) {
      return 0;
    }
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
    bytes = read_s(s, &ch, 1);
#else
    bytes = -1;
#endif
#else
    bytes = net_read(s, &ch, 1);
#endif
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
#endif
}

/*
 *   return true if there something waitting
 */
int net_peek(socket_t s)
{
#if defined(_VTOS)
  return 0;
#else

#if defined(_PalmOS)
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
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  int bytes;
  byte ch;

  bytes = read_s(s, &ch, 1);
  // put back... how ??? 
  return (bytes > 0);
#else
  return 0;
#endif
#elif defined(_Win32) || defined(__MINGW32__)
  unsigned long bytes;

  ioctlsocket(s, FIONREAD, &bytes);
  return (bytes > 0);
#else
  int bytes;

  ioctl(s, FIONREAD, &bytes);
  return (bytes > 0);
  /*
   * struct pollfd pfd;
   * 
   * pfd.fd = s; pfd.events = POLLIN|POLLERR; return (poll(&pfd, 1, 1) == 1); 
   */
#endif
#endif
}

/*
 *   connect to server and returns the socket
 */
socket_t net_connect(const char *server_name, int server_port)
{

#if defined(_PalmOS)
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
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  socket_t sock;
  dword inaddr;
  struct sockaddr_in ad;
  struct hostent *hp;

  net_init();

  memset(&ad, 0, sizeof(ad));
  ad.sin_family = AF_INET;

  if ((inaddr = inet_addr((char *)server_name)) == INADDR_NONE) {
    // hp = gethostbyname(server_name);
    // if (hp == NULL)
    return -1;
    // memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
  }
  else {
    memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
  }

  ad.sin_port = htons(server_port);
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock <= 0) {
    return sock;
  }
  if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
    return -1;
  }
  return sock;
#else
  return -1;
#endif
#elif defined(_VTOS)
  return -1;
#else
  socket_t sock;
  dword inaddr;
  struct sockaddr_in ad;
  struct hostent *hp;

  net_init();

  memset(&ad, 0, sizeof(ad));
  ad.sin_family = AF_INET;

  if ((inaddr = inet_addr(server_name)) == INADDR_NONE) {
    hp = gethostbyname(server_name);
    if (hp == NULL) {
      return -1;
    }
    memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
  }
  else {
    memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
  }

  ad.sin_port = htons(server_port);
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock <= 0) {
    return sock;
  }
  if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
    return -1;
  }
  return sock;
#endif
}

/*
 * listen for an incoming connection on the given port and 
 * returns the socket once a connection has been established
 */
socket_t net_listen(int server_port)
{
  int sock;
  struct sockaddr_in addr, remoteaddr;
  socklen_t remoteaddr_len;
  socket_t s;
  fd_set readfds;
  struct timeval tv;
  int rv;

  // more info about listen sockets:
  // http://beej.us/guide/bgnet/output/htmlsingle/bgnet.html#acceptman
  net_init();
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock <= 0) {
    return sock;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port); // clients connect to this port
  addr.sin_addr.s_addr = INADDR_ANY;  // autoselect IP address

  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  listen(sock, 1);              // set s up to be a server (listening) socket

  // clear the set
  FD_ZERO(&readfds);

  tv.tv_sec = 1;                // block at 1 second intervals
  tv.tv_usec = 0;
  while (1) {
    FD_SET(sock, &readfds);
    rv = select(sock + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
      s = 0;
      break;                    // an error occured
    }
    else if (rv == 0) {
      // timeout occured
      if (0 != dev_events(0)) {
        s = 0;
        break;
      }
    }
    else {
      // connection is ready
      s = accept(sock, (struct sockaddr *)&remoteaddr, &remoteaddr_len);
      break;
    }
  }

  close(sock);
  return s;
}

/*
 */
void net_disconnect(socket_t s)
{
#if defined(_PalmOS)
  Err err;
  NetLibSocketClose(netlib, s, 200, &err);
#elif defined(_Win32) || defined(__MINGW32__)
  closesocket(s);
#elif defined(_VTOS)
  // do nothing
#elif defined(_DOS)
#if defined(_DOSTCP_ENABLE)
  close_s(s);
#endif
#else
  close(s);
#endif
  net_close();
}

/* End of "$Id: inet.c,v 1.11 2006/08/03 10:30:53 zeeb90au Exp $". */
