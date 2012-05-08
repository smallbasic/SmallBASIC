// This file is part of SmallBASIC
//
// IrDA I/O, driver
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/device.h"
#include "common/pproc.h"
#include "common/match.h"

#include <errno.h>
#include <dirent.h> 

#if USE_TERM_IO
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#endif

typedef int FileHand;

#include "common/fs_stream.h"
#include "common/fs_irda.h"

#if defined(__CYGWIN__) || defined(__MINGW32__)
#include <windows.h>
#endif

/*
 */
int irda_open(dev_file_t * f) {
#if defined(_PalmOS)
  // /////////////////////////////////////////////////////////////////////////////////////////
  // PalmOS
  SysLibFind(irLibName, &f->libHandle);
  if (f->devspeed > 57600)
  f->devspeed = irOpenOptSpeed115200;
  else if (f->devspeed < 57600)
  f->devspeed = irOpenOptSpeed9600;
  else
  f->devspeed = irOpenOptSpeed57600;

  f->last_error = IrOpen(f->libHandle, f->devspeed);
  f->handle = 1;
  if (f->last_error)
  rt_raise("IROPEN() ERROR %d", f->last_error);

  return (f->last_error == 0);

#elif USE_TERM_IO
  // /////////////////////////////////////////////////////////////////////////////////////////
  // Unix
  sprintf(f->name, "/dev/irda%d", f->port);

  f->handle = open(f->name, O_RDWR | O_NOCTTY);
  if (f->handle < 0)
  err_file((f->last_error = errno));

  tcgetattr(f->handle, &f->oldtio); /* save current port settings */
  bzero(&f->newtio, sizeof(f->newtio));
  f->newtio.c_cflag = f->devspeed | CRTSCTS | CS8 | CLOCAL | CREAD;
  f->newtio.c_iflag = IGNPAR;
  f->newtio.c_oflag = 0;

  /*
   * set input mode (non-canonical, no echo,...) 
   */
  f->newtio.c_lflag = 0;
  f->newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  f->newtio.c_cc[VMIN] = 1; /* blocking read until 1 char received */
  tcflush(f->handle, TCIFLUSH);
  tcsetattr(f->handle, TCSANOW, &f->newtio);
  return (f->handle >= 0);

#elif defined(_DOS)
  // /////////////////////////////////////////////////////////////////////////////////////////
  // DOS
  sprintf(f->name, "COM%d", f->port);

  f->handle = open(f->name, O_RDWR | O_NOCTTY);
  if (f->handle < 0)
  err_file((f->last_error = errno));
  return (f->handle >= 0);
#elif defined(_Win32) || defined(__CYGWIN__) || defined(__MINGW32__)
  // /////////////////////////////////////////////////////////////////////////////////////////
  // Win32
  DCB dcb;
  HANDLE hCom;
  DWORD dwer;

  sprintf(f->name, "COM%d", f->port);

  hCom = CreateFile(f->name, GENERIC_READ | GENERIC_WRITE,
      0, NULL, OPEN_EXISTING, 0, NULL);

  if (hCom == INVALID_HANDLE_VALUE) {
    dwer = GetLastError();
    if (dwer != 5)
    rt_raise("SERIALFS: CreateFile() failed (%d)", dwer);
    else
    rt_raise("SERIALFS: ACCESS DENIED");
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

  f->handle = (int)hCom;
  return 1;
#else
  // /////////////////////////////////////////////////////////////////////////////////////////
  // ERROR
  err_unsup();
  return 0;                     // failed
#endif
}

/*
 */
int irda_close(dev_file_t * f) {
#if defined(_PalmOS)
  f->last_error = IrClose(f->libHandle);
  f->handle = -1;
  if (f->last_error)
  rt_raise("IRCLOSE() ERROR %d", f->last_error);
  return (f->last_error == 0);

#elif USE_TERM_IO
  tcsetattr(f->handle, TCSANOW, &f->oldtio);
  close(f->handle);
  f->handle = -1;
  return 1;

#elif defined(_DOS)
  close(f->handle);
  f->handle = -1;
  return 1;

#elif defined(_Win32) || defined(__CYGWIN__) || defined(__MINGW32__)
  CloseHandle((HANDLE) f->handle);
  f->handle = -1;
  return 1;

#else
  return 0;
#endif
}

/*
 */
int irda_write(dev_file_t * f, byte * data, dword size) {
#if defined(_PalmOS)
//      SerSend(f->libHandle, data, size, &f->last_error);
  if (f->last_error)
  rt_raise("SERSEND() ERROR %d", f->last_error);
  return (f->last_error == 0);

#elif defined(_UnixOS) || defined(_DOS)
  return stream_write(f, data, size);
#elif defined(_Win32)
  DWORD bytes;

  f->last_error = !WriteFile((HANDLE) f->handle, data, size, &bytes, NULL);
  return bytes;
#else
  return 0;
#endif
}

/*
 */
int irda_read(dev_file_t * f, byte * data, dword size) {
#if defined(_PalmOS)
  dword num;
  int ev;

  do {
//              f->last_error = SerReceiveCheck(f->libHandle, &num);
    ev = dev_events(0);

    if (f->last_error || prog_error || ev < 0) {
      IrClose(f->libHandle);
      f->handle = -1;
      break;
    }
    if (num >= size)
    break;
  }while (1);

//      if      ( num >= size )
//              f->last_error = SerReceive10(f->libHandle, data, size, -1);

  if (f->last_error)
  rt_raise("SERRECEIVE10() ERROR %d", f->last_error);

  return (f->last_error == 0);

#elif defined(_UnixOS) || defined(_DOS)
  return stream_read(f, data, size);
#elif defined(_Win32)
  DWORD bytes;

  f->last_error = !ReadFile((HANDLE) f->handle, data, size, &bytes, NULL);
  return bytes;
#else
  return 0;
#endif
}

/*
 *	Returns the number of the available data on serial port
 */
dword irda_length(dev_file_t * f) {
#if defined(_PalmOS)
  dword num;

//      f->last_error = SerReceiveCheck(f->libHandle, &num);
  if (f->last_error) {
    IrClose(f->libHandle);
    f->handle = -1;
    num = 0;
  }
  return num;

  // /////////////////////////////////////////////////////////////////////////////////////////
#elif defined(_UnixOS) || defined(_DOS)
  fd_set readfs;
  struct timeval tv;
  int res;

  FD_ZERO(&readfs);
  FD_SET(f->handle, &readfs);

  tv.tv_usec = 250; /* milliseconds */
  tv.tv_sec = 0; /* seconds */

  res = select(f->handle + 1, &readfs, NULL, NULL, &tv);
  if (FD_ISSET(f->handle, &readfs))
  return 1;

  return 0;
#elif defined(_Win32)
  COMSTAT cs;
  DWORD de = CE_BREAK;

  ClearCommError((HANDLE) f->handle, &de, &cs);
  return cs.cbInQue;
#else
  return 0;
#endif
}

/*
 *	Returns true (EOF) if the connection is broken
 */
dword irda_eof(dev_file_t * f) {
#if defined(_PalmOS)
//      Boolean a, b;
//      return (SerGetStatus(f->libHandle, &a, &b) != 0);
  return 0;
#else
  return f->last_error;
#endif
}
