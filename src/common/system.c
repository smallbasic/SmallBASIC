// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/device.h"

#include <stdio.h>
#include <time.h>

#if defined(_UnixOS)
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(_Win32)
#include <windows.h>
#include <process.h>
#include <dir.h>
#endif

extern char **environ;

#define BUFSIZE 1024

#if defined(_Win32)

/**
 * w32 run process and capture stdout/stderr using pipes
 *
 * returns a newly allocated string with the result or NULL
 *
 * warning: if the cmd is a GUI process, the pw_shell will hang
 */
char *pw_shell(const char *cmd) {
  HANDLE h_inppip, h_outpip, h_errpip, h_pid;
  char buf[BUFSIZE + 1], cv_buf[BUFSIZE + 1];
  char *result = NULL;
  int block_count = 0;
  DWORD bytes;

  SECURITY_ATTRIBUTES sa;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  memset(&sa, 0, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  if (!CreatePipe(&h_inppip, &h_outpip, &sa, BUFSIZE)) {
    return NULL;
  }

  h_pid = GetCurrentProcess();

  DuplicateHandle(h_pid, h_inppip, h_pid, &h_inppip, 0, FALSE,
                  DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
  DuplicateHandle(h_pid, h_outpip, h_pid, &h_errpip, 0, TRUE, DUPLICATE_SAME_ACCESS);
  
  // run
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.wShowWindow = SW_HIDE;

  si.hStdOutput = h_outpip;
  si.hStdError = h_errpip;

  if (CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
    // close streams
    CloseHandle(pi.hThread);
    CloseHandle(h_outpip);
    CloseHandle(h_errpip);
    h_errpip = h_outpip = NULL;

    // read stdout/err
    while (ReadFile(h_inppip, buf, BUFSIZE, &bytes, NULL)) {
      buf[bytes] = '\0';
      memset(cv_buf, 0, BUFSIZE + 1);
      OemToCharBuff(buf, cv_buf, bytes);
      block_count++;
      if (result) {
        result = (char *)realloc(result, block_count * BUFSIZE + 1);
      } else {
        result = (char *)malloc(BUFSIZE + 1);
        *result = '\0';
      }
      strcat(result, cv_buf);
    }

    CloseHandle(pi.hProcess);
  }
  else {
    result = NULL;              // could not run it
  }

  // clean up
  CloseHandle(h_inppip);
  if (h_outpip) {
    CloseHandle(h_outpip);
  }
  if (h_errpip) {
    CloseHandle(h_errpip);
  }
  return result;
}
#endif

/**
 * run a program (if retflg wait and return; otherwise just exec())
 */
int dev_run(const char *src, int retflg) {
#if defined(_Win32)
  int r;
  char *out;

  r = ((out = pw_shell(src)) != NULL);
  if (r) {
    free(out);
  }

  if (r && !retflg) {
    exit(1);                    // ok, what to do now ?
  }
  return r;
#else
  if (retflg) {
    return (system(src) != -1);
  } else {
    //              execl(src, src, NULL);
    // call the shell if we want to behave same like system() function
    //  -c means the next argument is the command string to execute
    // this allow us to execute shell script too!
    char *src1;
    src1 = malloc(strlen(src) + 3);
    if (src1 == NULL) {
      exit(-1);                 // ok, what to do now ?
    }
    memset(src1, '\0', strlen(src) + 3);
    *src1 = '"';
    strcat(src1, src);
    *(src1 + strlen(src) + 1) = '"';  // we need a doublequote around the command
    execlp("sh", "sh", "-c", src1, NULL);
    exit(-1);
    // o.k. some error happens - what to do??? we already closed the screen!!
  }
#endif
}

#ifndef IMPL_DEV_ENV

/**
 * The  putenv() function adds or changes the value of environment variables.  The argument string 
 * is of the form name=value. If name does not already exist in the environment, then string is added
 * to the environment.  If name does exist, then the value of name in the environment is changed 
 * to value.  The string pointed to by string becomes part of the environment, so
 * altering the string changes the environment.
 *
 * The putenv() function returns zero on success, or -1 if an error occurs.
 *
 * If the value is zero-length then the variable must be deleted. (libc4,libc5 compatible version)
 */
int dev_putenv(const char *str) {
  char *p = strdup(str); // no free()
  return putenv(p);
}

/**
 * The  getenv() function searches the environment list for a string that matches the string pointed 
 * to by name. The strings are of the form name = value.
 *
 * The getenv() function returns a pointer to the value in the environment, or NULL if there is no match.
 */
char *dev_getenv(const char *str) {
  return getenv(str);
}

/**
 * returns the number of environment variables
 */
int dev_env_count() {
  int count = 0;
  while (environ[count]) {
    count++;
  }
  return count;
}

/**
 * returns the value of the n-th system's environment variable
 */
char *dev_getenv_n(int n) {
  int count = 0;
  while (environ[count]) {
    if (n == count) {
      return environ[count];
    }
    count++;
  }
  return NULL;
}

#endif // IMPL_DEV_ENV

dword dev_get_millisecond_count(void) {
#if defined(__MACH__)
  struct timeval t;
  gettimeofday(&t, NULL);
  return (dword) (1000L * t.tv_sec + (t.tv_usec / 1000.0));
#elif defined(_Win32)
  return GetTickCount();
#else
  struct timespec t;
  t.tv_sec = t.tv_nsec = 0;
  if (0 == clock_gettime(CLOCK_MONOTONIC, &t)) {
    return (dword) (1000L * t.tv_sec + (t.tv_nsec / 1e6));
  } else {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (dword) (1000L * now.tv_sec + (now.tv_usec / 1000.0));
  }
#endif
}


