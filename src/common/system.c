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
  HANDLE hPipeRead, hPipeWrite;

  SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
  // Pipe handles are inherited by child process.
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  // Create a pipe to get results from child's stdout.
  if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
    return 0;
  }

  STARTUPINFO si = { sizeof(STARTUPINFO) };
  si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.hStdOutput  = hPipeWrite;
  si.hStdError   = hPipeWrite;
  // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.
  si.wShowWindow = SW_HIDE;

  PROCESS_INFORMATION pi  = { 0 };
  if (!CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    return 0;
  }

  int processEnded = 0;
  while (!processEnded) {
    // Give some timeslice (50ms), so we won't waste 100% cpu.
    processEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

    // Even if process exited - we continue reading, if there is some data available over pipe.
    while (1) {
      char buf[BUFSIZE];
      DWORD numRead = 0;
      DWORD numAvail = 0;

      if (!PeekNamedPipe(hPipeRead, NULL, 0, NULL, &numAvail, NULL)) {
        break;
      }

      if (!numAvail) {
        // no data available
        break;
      }

      if (!ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, numAvail), &numRead, NULL) ||
          !numRead) {
        // child process may have ended
        break;
      }
      buf[numRead] = 0;
      v_strcat(r, buf);
    }
  }
  CloseHandle(hPipeWrite);
  CloseHandle(hPipeRead);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return 1;
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


