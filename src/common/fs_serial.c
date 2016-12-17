// This file is part of SmallBASIC
//
// serial I/O, driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/fs_stream.h"
#include "common/fs_serial.h"

#if USE_TERM_IO
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

int serial_open(dev_file_t *f) {
  sprintf(f->name, "/dev/ttyS%d", f->port);

  f->handle = open(f->name, O_RDWR | O_NOCTTY);
  if (f->handle < 0) {
    err_file((f->last_error = errno));
  }
  // save current port settings
  tcgetattr(f->handle, &f->oldtio);
  bzero(&f->newtio, sizeof(f->newtio));
  f->newtio.c_cflag = f->devspeed | CRTSCTS | CS8 | CLOCAL | CREAD;
  f->newtio.c_iflag = IGNPAR;
  f->newtio.c_oflag = 0;

  // set input mode (non-canonical, no echo,...)
  f->newtio.c_lflag = 0;
  f->newtio.c_cc[VTIME] = 0; // inter-character timer unused
  f->newtio.c_cc[VMIN] = 1; // blocking read until 1 char received
  tcflush(f->handle, TCIFLUSH);
  tcsetattr(f->handle, TCSANOW, &f->newtio);
  return (f->handle >= 0);
}

int serial_close(dev_file_t *f) {
  tcsetattr(f->handle, TCSANOW, &f->oldtio);
  close(f->handle);
  f->handle = -1;
  return 1;
}

int serial_write(dev_file_t *f, byte *data, dword size) {
  return stream_write(f, data, size);
}

int serial_read(dev_file_t *f, byte *data, dword size) {
  return stream_read(f, data, size);
}

// Returns the number of the available data on serial port
dword serial_length(dev_file_t *f) {
  fd_set readfs;
  struct timeval tv;

  FD_ZERO(&readfs);
  FD_SET(f->handle, &readfs);

  tv.tv_usec = 250; // milliseconds
  tv.tv_sec = 0; // seconds

  select(f->handle + 1, &readfs, NULL, NULL, &tv);
  if (FD_ISSET(f->handle, &readfs)) {
    return 1;
  }
  return 0;
}

#elif defined(_Win32)
typedef int FileHand;

int serial_open(dev_file_t *f) {
  DCB dcb;
  HANDLE hCom;
  DWORD dwer;

  sprintf(f->name, "COM%d", f->port);

  hCom = CreateFile(f->name, GENERIC_READ | GENERIC_WRITE,
                    0, NULL, OPEN_EXISTING, 0, NULL);

  if (hCom == INVALID_HANDLE_VALUE) {
    dwer = GetLastError();
    if (dwer != 5) {
      rt_raise("SERIALFS: CreateFile() failed (%d)", dwer);
    } else {
      rt_raise("SERIALFS: ACCESS DENIED");
    }
    return 0;
  }

  if (!GetCommState(hCom, &dcb)) {
    rt_raise("SERIALFS: GetCommState() failed (%d)", GetLastError());
    return 0;
  }

  dcb.BaudRate = f->devspeed;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;

  if (!SetCommState(hCom, &dcb)) {
    rt_raise("SERIALFS: SetCommState() failed (%d)", GetLastError());
    return 0;
  }

  f->handle = (FileHand) hCom;
  return 1;
}

int serial_close(dev_file_t *f) {
  CloseHandle((HANDLE) f->handle);
  f->handle = -1;
  return 1;
}

int serial_write(dev_file_t *f, byte *data, dword size) {
  DWORD bytes;
  f->last_error = !WriteFile((HANDLE) f->handle, data, size, &bytes, NULL);
  return bytes;
}

int serial_read(dev_file_t *f, byte *data, dword size) {
  DWORD bytes;
  f->last_error = !ReadFile((HANDLE) f->handle, data, size, &bytes, NULL);
  return bytes;
}

dword serial_length(dev_file_t *f) {
  COMSTAT cs;
  DWORD de = CE_BREAK;
  ClearCommError((HANDLE) f->handle, &de, &cs);
  return cs.cbInQue;
}

#else

int serial_open(dev_file_t *f) {
  err_unsup();
  return 0;
}

int serial_close(dev_file_t *f) {
  return 0;
}

int serial_write(dev_file_t *f, byte *data, dword size) {
  return 0;
}

int serial_read(dev_file_t *f, byte *data, dword size) {
  return 0;
}

dword serial_length(dev_file_t *f) {
  return 0;
}

#endif

/*
 * Returns true (EOF) if the connection is broken
 */
dword serial_eof(dev_file_t *f) {
  return f->last_error;
}
