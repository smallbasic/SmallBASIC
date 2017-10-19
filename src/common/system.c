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
 * warning: if the cmd is a GUI process, the shell will hang
 */
int shell(const char *cmd, var_t *r) {
  SECURITY_ATTRIBUTES sa;
  PROCESS_INFORMATION pi;
  HANDLE h_inppip, h_outpip, h_errpip;
  int result = 0;

  memset(&sa, 0, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  if (!CreatePipe(&h_inppip, &h_outpip, &sa, BUFSIZE)) {
    return 0;
  }

  HANDLE h_pid = GetCurrentProcess();
  DuplicateHandle(h_pid, h_inppip, h_pid, &h_inppip, 0, FALSE,
                  DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
  DuplicateHandle(h_pid, h_outpip, h_pid, &h_errpip, 0, TRUE, DUPLICATE_SAME_ACCESS);

  STARTUPINFO si;
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

    DWORD bytes;
    char buf[BUFSIZE + 1];
    char cv_buf[BUFSIZE + 1];

    // read stdout/err
    while (ReadFile(h_inppip, buf, BUFSIZE - 1, &bytes, NULL)) {
      buf[bytes] = '\0';
      memset(cv_buf, 0, BUFSIZE + 1);
      OemToCharBuff(buf, cv_buf, bytes);
      v_strcat(r, cv_buf);
      result = 1;
    }
    CloseHandle(pi.hProcess);
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

int dev_run(const char *cmd, var_t *r, int wait) {
  int result = 1;
  if (r != NULL) {
    v_zerostr(r);
    result = shell(cmd, r);
  } else if (wait) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    if (CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
      WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      result = 0;
    }
  } else {
    HWND hwnd = GetActiveWindow();
    ShellExecute(hwnd, "open", cmd, 0, 0, SW_SHOWNORMAL);
  }
  return result;
}

#else
int dev_run(const char *cmd, var_t *r, int wait) {
  int result = 1;
  if (r != NULL) {
    v_zerostr(r);
    FILE *fin = popen(cmd, "r");
    if (fin) {
      while (!feof(fin)) {
        char buf[BUFSIZE + 1];
        int bytes = fread(buf, 1, BUFSIZE, fin);
        buf[bytes] = '\0';
        v_strcat(r, buf);
      }
      pclose(fin);
    } else {
      result = 0;
    }
  } else if (wait) {
    result = (system(cmd) != -1);
  }
  else if (fork() == 0) {
    // exec separate process
    int size = strlen(cmd) + 3;
    char *src1 =  malloc(size);
    if (src1 != NULL) {
      memset(src1, '\0', size);
      // double quote the command
      *src1 = '"';
      strcat(src1, cmd);
      *(src1 + strlen(cmd) + 1) = '"';
      // -c means the next argument is the command string to execute
      // this allow us to execute shell script
      execlp("sh", "sh", "-c", src1, NULL);
    }
    exit(-1);
  }
  return result;
}
#endif

#if !defined(IMPL_DEV_ENV)

int dev_setenv(const char *key, const char *value) {
#ifdef __MINGW32__
  // use leaky putenv
  unsigned size = snprintf(NULL, 0, "%s=%s", key, value) + 1;
  char *buf = malloc(size);
  buf[0] = '\0';
  snprintf(buf, size, "%s=%s", key, value);
  return putenv(buf);
#else
  return setenv(key, value, 1);
#endif
}

/**
 * The  getenv() function searches the environment list for a string that matches the string pointed
 * to by name. The strings are of the form name = value.
 *
 * The getenv() function returns a pointer to the value in the environment, or NULL if there is no match.
 */
const char *dev_getenv(const char *str) {
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
const char *dev_getenv_n(int n) {
  int count = 0;
  while (environ[count]) {
    if (n == count) {
      return environ[count];
    }
    count++;
  }
  return NULL;
}
#endif

uint32_t dev_get_millisecond_count(void) {
#if defined(__MACH__)
  struct timeval t;
  gettimeofday(&t, NULL);
  return (uint32_t) (1000L * t.tv_sec + (t.tv_usec / 1000.0));
#elif defined(_Win32)
  return GetTickCount();
#else
  struct timespec t;
  t.tv_sec = t.tv_nsec = 0;
  if (0 == clock_gettime(CLOCK_MONOTONIC, &t)) {
    return (uint32_t) (1000L * t.tv_sec + (t.tv_nsec / 1e6));
  } else {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (uint32_t) (1000L * now.tv_sec + (now.tv_usec / 1000.0));
  }
#endif
}


