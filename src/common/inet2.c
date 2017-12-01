// This file is part of SmallBASIC
//
// Network library (byte-stream sockets)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/inet.h"
#include "common/device.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(_Win32)
static int inetlib_init = 0;
#else
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#endif

// the length of time (usec) to block waiting for an event
#define BLOCK_INTERVAL 250000

/**
 * prepare to use the network
 */
int net_init() {
#if defined(_Win32)
  if (!inetlib_init) {
    inetlib_init = 1;
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 0), &wsadata)) {
      return 0;
    }
    return 1;
  }
#else
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, 0);
#endif
  return 1;
}

/**
 * stop using the network
 */
int net_close() {
#if defined(_Win32)
  if (inetlib_init) {
    WSACleanup();
    inetlib_init = 0;
  }
#endif
  return 1;
}

/**
 * sends a string to socket
 */
void net_print(socket_t s, const char *str) {
  send(s, str, strlen(str), 0);
}

void net_send(socket_t s, const char *str, size_t size) {
  send(s, str, size, 0);
}

/**
 * sends a string to socket
 */
void net_printf(socket_t s, const char *fmt, ...) {
  char buf[1025];
  va_list argp;

  va_start(argp, fmt);
  vsnprintf(buf, sizeof(buf), fmt, argp);
  va_end(argp);
  net_print(s, buf);
}

/**
 * read the specified number of bytes from the socket
 */
int net_read(socket_t s, char *buf, int size) {
  fd_set readfds;
  struct timeval tv;
  int rv;

  // clear the set
  FD_ZERO(&readfds);

  while (1) {
    FD_SET(s, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = BLOCK_INTERVAL;

    rv = select(s + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
      // an error occured
      return 0;
    } else if (rv == 0) {
      // timeout occured
      if (0 != dev_events(0)) {
        s = 0;
        break;
      }
    } else {
      // ready to read
      return recv(s, buf, size, 0);
    }
  }
  return 0;
}

/**
 * read a string from a socket until a char from delim str found.
 */
int net_input(socket_t s, char *buf, int size, const char *delim) {
  // wait for remote input without eating cpu
  fd_set readfds;
  struct timeval tv;
  char ch;
  int count = 0;

  // clear the set
  FD_ZERO(&readfds);

  while (1) {
    tv.tv_sec = 0;
    tv.tv_usec = BLOCK_INTERVAL;        // time is reset in select() call in linux
    FD_SET(s, &readfds);

    int rv = select(s + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
      return 0;                 // an error occured
    } else if (rv == 0) {
      // timeout occured - check for program break
      if (0 != dev_events(0)) {
        return 0;
      }
    } else if (FD_ISSET(s, &readfds)) {
      // ready for reading
      break;
    }
  }

  FD_ZERO(&readfds);

  memset(buf, 0, size);
  while (count < size) {
    int bytes = net_read(s, &ch, 1);
    if (bytes <= 0) {
      return count;             // no more data
    } else {
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
int net_peek(socket_t s) {
#if defined(_Win32)
  unsigned long bytes;

  ioctlsocket(s, FIONREAD, &bytes);
  return (bytes > 0);
#else
  int bytes;

  ioctl(s, FIONREAD, &bytes);
  return (bytes > 0);
#endif
}

/**
 * connect to server and returns the socket
 */
socket_t net_connect(const char *server_name, int server_port) {
  socket_t sock;
  uint32_t inaddr;
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
  } else {
    memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
  }

  ad.sin_port = htons(server_port);
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock <= 0) {
    return sock;
  }
  if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
    net_disconnect(sock);
    return -1;
  }
  return sock;
}

/**
 * listen for an incoming connection on the given port and 
 * returns the socket once a connection has been established
 */
socket_t net_listen(int server_port) {
  int listener;
  struct sockaddr_in addr, remoteaddr;
  socket_t s;
  fd_set readfds;
  struct timeval tv;
  int rv;
  int yes = 1;

  // more info about listen sockets:
  // http://beej.us/guide/bgnet/output/htmlsingle/bgnet.html#acceptman
  net_init();
  listener = socket(PF_INET, SOCK_STREAM, 0);
  if (listener <= 0) {
    return listener;
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);   // clients connect to this port
  addr.sin_addr.s_addr = INADDR_ANY;    // autoselect IP address
  memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));  

  // prevent address already in use bind errors
  if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1) {
    return -1;
  }
  // set s up to be a server (listening) socket
  if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    net_disconnect(listener);
    return -1;
  }

  if (listen(listener, 1) == -1) {
    net_disconnect(listener);
    return -1;
  }
  // clear the set
  FD_ZERO(&readfds);

  while (1) {
    tv.tv_sec = 0;              // block at 1 second intervals
    tv.tv_usec = 500000;        // time is reset in select() call in linux
    FD_SET(listener, &readfds);

    rv = select(listener + 1, &readfds, NULL, NULL, &tv);
    if (rv == -1) {
      s = 0;
      break;                    // an error occured
    } else if (rv == 0) {
      // timeout occured - check for program break
      if (0 != dev_events(0)) {
        s = 0;
        break;
      }
    } else if (FD_ISSET(listener, &readfds)) {
      // connection is ready
#if defined(_Win32)
      int remoteaddr_len = sizeof(remoteaddr);
#else
      socklen_t remoteaddr_len = sizeof(remoteaddr);
#endif
      s = accept(listener, (struct sockaddr *)&remoteaddr, &remoteaddr_len);
      break;
    }
  }

  FD_ZERO(&readfds);
  net_disconnect(listener);

  return s;
}

/**
 * disconnect the given network connection
 */
void net_disconnect(socket_t s) {
#if defined(_Win32)
  closesocket(s);
#else
  close(s);
#endif
}

