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

/**
 * terminal speed
 * select the correct system constant
 */
long select_unix_serial_speed(long n) {
  switch (n) {
    case 300:
    return B300;
    case 600:
    return B600;
    case 1200:
    return B1200;
    case 2400:
    return B2400;
    case 4800:
    return B4800;
    case 9600:
    return B9600;
    case 19200:
    return B19200;
    case 38400:
    return B38400;
#ifdef B4000000
    // extra baud rates are not POSIX standard
    // but supported by Linux and FreeBSD
    case 57600:
    return B57600;
    case 115200:
    return B115200;
    case 230400:
    return B230400;
    case 460800:
    return B460800;
    case 500000:
    return B500000;
    case 921600:
    return B921600;
    case 1000000:
    return B1000000;
    case 1500000:
    return B1500000;
    case 2000000:
    return B2000000;
    case 2500000:
    return B2500000;
    case 3000000:
    return B3000000;
    case 3500000:
    return B3500000;
    case 4000000:
    return B4000000;
#endif
#ifdef B576000
    // Following baud rates are supported in Linux but
    // not defined in FreeBSD
    case 576000:
    return B576000;
    case 1152000:
    return B1152000;
#endif
  }
  return B9600;
}

int serial_open(dev_file_t *f) {
  if (strncmp(f->name, "COM", 3) == 0) {
    sprintf(f->name, "/dev/ttyS%d", f->port);
  }

  f->handle = open(f->name, O_RDWR | O_NOCTTY);
  if (f->handle < 0) {
    err_file((f->last_error = errno));
  }

  f->devspeed = select_unix_serial_speed(f->devspeed);

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

int serial_write(dev_file_t *f, byte *data, uint32_t size) {
  return stream_write(f, data, size);
}

int serial_read(dev_file_t *f, byte *data, uint32_t size) {
  return stream_read(f, data, size);
}

// Returns the number of the available data on serial port
uint32_t serial_length(dev_file_t *f) {
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

int serial_open(dev_file_t *f) {
  DCB dcb;
  HANDLE hCom;
  DWORD dwer;

  if (strncmp(f->name, "COM", 3) != 0) {
    rt_raise("SERIALFS: Linux serial port was specified. Use COM instead.");
  }

  // Bug when opening COM-Port > 9: https://support.microsoft.com/en-us/topic/howto-specify-serial-ports-larger-than-com9-db9078a5-b7b6-bf00-240f-f749ebfd913e
  sprintf(f->name, "\\\\.\\COM%d", f->port);
  
  hCom = CreateFile(f->name, GENERIC_READ | GENERIC_WRITE,
                    0, NULL, OPEN_EXISTING, 0, NULL);

  if (hCom == INVALID_HANDLE_VALUE) {
    dwer = GetLastError();
    if (dwer != 5) {
      rt_raise("SERIALFS: CreateFile() failed (Error %d)", dwer);
    } else {
      rt_raise("SERIALFS: ACCESS DENIED");
    }
    return 0;
  }

  if (!GetCommState(hCom, &dcb)) {
    rt_raise("SERIALFS: GetCommState() failed (Error %d)", GetLastError());
    return 0;
  }

  // Default settings from Putty
  dcb.fBinary = TRUE;                    
  dcb.fDtrControl = DTR_CONTROL_ENABLE;   // This is needed for Raspberry Pi PICO
  dcb.fDsrSensitivity = FALSE;
  dcb.fTXContinueOnXoff = FALSE;
  dcb.fOutX = FALSE;                      
  dcb.fInX = FALSE;                       
  dcb.fErrorChar = FALSE;
  dcb.fNull = FALSE;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;  
  dcb.fAbortOnError = FALSE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  // Settings SmallBASIC
  dcb.BaudRate = f->devspeed;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;

  if (!SetCommState(hCom, &dcb)) {
    rt_raise("SERIALFS: SetCommState() failed (Error %d)", GetLastError());
    return 0;
  }

  f->handle = (intptr_t)hCom;
  return 1;
}

int serial_close(dev_file_t *f) {
  CloseHandle((HANDLE) (intptr_t)f->handle);
  f->handle = -1;
  return 1;
}

int serial_write(dev_file_t *f, byte *data, uint32_t size) {
  DWORD bytes;
  f->last_error = !WriteFile((HANDLE)(intptr_t)f->handle, data, size, &bytes, NULL);
  return bytes;
}

int serial_read(dev_file_t *f, byte *data, uint32_t size) {
  DWORD bytes;
  f->last_error = !ReadFile((HANDLE)(intptr_t)f->handle, data, size, &bytes, NULL);
  return bytes;
}

uint32_t serial_length(dev_file_t *f) {
  COMSTAT cs;
  DWORD de = CE_BREAK;
  ClearCommError((HANDLE)(intptr_t)f->handle, &de, &cs);
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

int serial_write(dev_file_t *f, byte *data, uint32_t size) {
  return 0;
}

int serial_read(dev_file_t *f, byte *data, uint32_t size) {
  return 0;
}

uint32_t serial_length(dev_file_t *f) {
  return 0;
}

#endif

/*
 * Returns true (EOF) if the connection is broken
 */
uint32_t serial_eof(dev_file_t *f) {
  return f->last_error;
}
