// $Id$
// This file is part of SmallBASIC
//
// lowlevel device (OS) I/O
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include "str.h"
#include "var.h"
#include "device.h"
#include "osd.h"
#include "smbas.h"
#include "sberr.h"
#include "messages.h"

#if !defined(_PalmOS)
#include <signal.h>
#include <stdio.h>
#if defined(_UnixOS)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>           // struct timeval
#include <unistd.h>
extern char **environ;
#elif defined(_DOS)
#include <sys/time.h>           // struct timeval
#include <unistd.h>
#include <conio.h>
extern char **environ;
#elif defined(_Win32) || defined(__MINGW32__)
#include <windows.h>
#include <process.h>
#include <dir.h>
extern char **environ;
#endif
#endif

#ifndef IMPL_DEV_RUN

/**
 * run an external program
 */
#if defined(_Win32)
/**
 * Run a program in the Win32 environment and captures the stdout/stderr
 *
 * cmd     = the command line (ex: dir /w)
 * infile  = the stdin (can be NULL)
 * outfile = the stdout/stderr
 *
 * (like "rm -f < infile > outfile")
 *
 * Warning: you can't use DOS built-in commands (like dir, del, etc)
 * Warning: there is no way to take the real output (stdout/stderr as one file)
 *     I have test it (the same file for stdout & stderr) on Win95 box
 *     and I take the output of stdout at the top and the output of stderr at the bottom !!!
 */
int shellw2(const char *cmd, const char *inbuf, char **outbuf, int priority) {
  static char tmp_dir[1024];
  static int sh2_count;

  HANDLE new_stdin, new_stdout, new_stderr;
  int attr = 0, failed = 0, backg = 0, clen;
  DWORD exit_code;
  FILE *fp;

  STARTUPINFO si;
  SECURITY_ATTRIBUTES sap, sat;
  PROCESS_INFORMATION pip;

  char fname_in[1024], fname_out[1024], fname_err[1024];

  //
  if ((clen = strlen(cmd)) == 0)
    return -1;

  //
  if (cmd[clen - 1] == '&')
    backg = 1;

  // temporary directory
  if (tmp_dir[0] == '\0') {
    GetTempPath(1024, tmp_dir);
    if (tmp_dir[strlen(tmp_dir) - 1] == '\\')
      tmp_dir[strlen(tmp_dir) - 1] = '\0';
    if (strlen(tmp_dir) < 4) {
      mkdir("c:\\tmp");
      strcpy(tmp_dir, "c:\\tmp");
    }
  }

  // new handles - filenames
  sh2_count++;
  sprintf(fname_in, "%s\\shellw2.0x%X.%d.in", tmp_dir, getpid(), sh2_count);
  sprintf(fname_out, "%s\\shellw2.0x%X.%d.out", tmp_dir, getpid(), sh2_count);
  sprintf(fname_err, "%s\\shellw2.0x%X.%d.err", tmp_dir, getpid(), sh2_count);

  // fill the new stdin
  if (inbuf && !backg) {
    if ((fp = fopen(fname_in, "wt")) != NULL) {
      fwrite(inbuf, strlen(inbuf), 1, fp);
      fclose(fp);
    }
  }

  if (!backg) {
    // create the new standard handles
    new_stdin =
    CreateFile(fname_in, GENERIC_READ, 0, NULL, CREATE_ALWAYS,
               FILE_ATTRIBUTE_NORMAL, NULL);
    new_stdout =
    CreateFile(fname_out, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
               FILE_ATTRIBUTE_NORMAL, NULL);
    new_stderr =
    CreateFile(fname_err, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
               FILE_ATTRIBUTE_NORMAL, NULL);
  }

  // fill stupid structures

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  sap.nLength = sizeof(sap);
  sap.lpSecurityDescriptor = NULL;
  sap.bInheritHandle = TRUE;
  sat.nLength = sizeof(sat);
  sat.lpSecurityDescriptor = NULL;
  sat.bInheritHandle = TRUE;

  // create process attributes (priority, etc).
  attr |= priority;
  attr |= DETACHED_PROCESS;

  // show window args
  si.wShowWindow = SW_HIDE;
  si.dwFlags = STARTF_USESHOWWINDOW;

  // redirection standard handles
  if (!backg) {
    si.hStdInput = new_stdin;
    si.hStdOutput = new_stdout;
    si.hStdError = new_stderr;
    si.dwFlags = STARTF_USESTDHANDLES;
  }
  else {
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;
  }

  /**** RUN IT ****/
  if (CreateProcess(NULL, (char *)cmd,
                    &sap, &sat, TRUE, attr, NULL, NULL, &si, &pip)) {

    // wait the process
    if (!backg)
      while (WaitForSingleObject(pip.hProcess, 1000) == WAIT_TIMEOUT);

    // ok, the process is finished, close handle

    /**
       from GetExitCodeProcess():

       If the function succeeds, the return value is nonzero.
       If the function fails, the return value is zero. To get extended error information, call GetLastError.

       Remarks
       If the specified process has not terminated, the termination status returned is STILL_ACTIVE.
       If the process has terminated, the termination status returned may be one of the following:

       * The exit value specified in the ExitProcess or TerminateProcess function.
       * The return value from the main or WinMain function of the process.
       * The exception value for an unhandled exception that caused the process to terminate.

       */

    if (!backg)
      GetExitCodeProcess(pip.hProcess, &exit_code);
    CloseHandle(pip.hProcess);
  }
  else
    failed = 1;

  // close files
  if (!backg) {
    CloseHandle(new_stdin);
    CloseHandle(new_stdout);
    CloseHandle(new_stderr);
  }

  if (!failed) {
    // copy stdout & stderr to output file
    // don't use the ultrashit API ReadFile func
    if (outbuf && !backg) {
      *outbuf = NULL;
      if ((fp = fopen(fname_out, "rt")) != NULL) {
        char *buf;
        int size;

        size = filelength(fileno(fp));
        buf = (char *)tmp_alloc(size + 1);
        if (size)
          fread(buf, size, 1, fp);
        buf[size] = '\0';
        *outbuf = buf;
        fclose(fp);             // STD-C is beautiful.
        // You dont want to see this with Win32-API commands
      }
    }                           // outbuf
  }                             // !failed
  else if (outbuf)
    *outbuf = NULL;

  //
  if (!backg) {
    remove(fname_in);
    remove(fname_out);
    remove(fname_err);
  }

  if (!failed)
    return exit_code;
  return -1;
}

/**
 * w32 run process and capture stdout/stderr using pipes
 *
 * returns a newly allocated string with the result or NULL
 *
 * warning: if the cmd is a GUI process, the pw_shell will hang
 */
#define BUFSIZE 1024
char *pw_shell(const char *cmd) {
  HANDLE h_inppip, h_outpip, h_errpip, h_pid;
  char buf[BUFSIZE + 1], cv_buf[BUFSIZE + 1];
  char *result = NULL;
  int block_count = 0, bytes;

  SECURITY_ATTRIBUTES sa;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  memset(&sa, 0, sizeof(sa));
  sa.nLength = sizeof(sa);
  sa.bInheritHandle = TRUE;

  if (!CreatePipe(&h_inppip, &h_outpip, &sa, BUFSIZE))
    return NULL;                // failed

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

  if (CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {

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
      if (result)
        result = (char *)tmp_realloc(result, block_count * BUFSIZE + 1);
      else {
        result = (char *)tmp_alloc(BUFSIZE + 1);
        *result = '\0';
      }

      strcat(result, cv_buf);
    }

    //
    CloseHandle(pi.hProcess);
  }
  else
    result = NULL;              // could not run it


  // clean up
  CloseHandle(h_inppip);
  if (h_outpip)
    CloseHandle(h_outpip);
  if (h_errpip)
    CloseHandle(h_errpip);

  return result;
}
#undef BUFSIZE

#endif

/**
 * run a program (if retflg wait and return; otherwise just exec())
 */
int dev_run(const char *src, int retflg) {
#if defined(_PalmOS)
  LocalID lid;
  dword progid;
  word card;
  DmSearchStateType state;

  progid =
  ((dword) src[0] << 24) + ((dword) src[1] << 16) + ((dword) src[2] << 8) +
  (dword) src[3];
  if (DmGetNextDatabaseByTypeCreator
      (true, &state, 0x6170706C, progid, true, &card, &lid) == 0)
    return (SysUIAppSwitch(card, lid, sysAppLaunchCmdNormalLaunch, NULL) == 0);
  return 0;
#elif defined(_VTOS) || defined(_FRANKLIN_EBM)
  return 0;
#elif defined(_Win32)
  int r;
  char *out;

  r = ((out = pw_shell(src)) != NULL);
  if (r) {
    tmp_free(out);
  }

  if (r && !retflg) {
    exit(1);                    // ok, what to do now ?
  }
  return r;
#else
  if (retflg) {
    return (system(src) != -1);
  }
  else {
    //              execl(src, src, NULL);
    // call the shell if we want to behave same like system() function
    //  -c means the next argument is the command string to execute
    // this allow us to execute shell script too!
    char *src1;
#if defined(__CYGWIN__) || defined(__MINGW32__)
    char *cmdspec;
#endif

    src1 = malloc(strlen(src) + 3);
    if (src1 == NULL)
      exit(-1);                 // ok, what to do now ?
    memset(src1, '\0', strlen(src) + 3);
    *src1 = '"';
    strcat(src1, src);
    *(src1 + strlen(src) + 1) = '"';  // we need a doublequote around the
    // command
#if defined(_SDL)
    if (os_graphics) {
      dev_settextcolor(0, 0);
      dev_printf("\n");         // in SDL version a new line with black on
      // black color means no wait on quit
      osd_devrestore();         // we have to close the graphical screen before
                                //
      // call the program
    }
#endif
    execlp("sh", "sh", "-c", src1, NULL);
#if defined(__CYGWIN__) || defined(__MINGW32__)
    cmdspec = getenv("COMSPEC");
    if (cmdspec == NULL) {
      cmdspec = strdup("cmd");
    }
    execlp(cmdspec, cmdspec, "/c", src1, NULL); // might be we were cross
    // compiled under CYGWIN or
    // MINGW but running
    // in native windows environment which means no shell (sh) available
    // try to use the standard windows command interpreter
#endif
    exit(-1);                   // o.k. some error happens - what to do??? we
    // already closed the screen!!
  }
#endif
}

#endif // IMPL_DEV_RUN

#ifndef IMPL_DEV_ENV

/**
 * GNU Manual:
 *
 * The  putenv() function adds or changes the value of environment variables.  The argument string is of the form name=value.
 * If name does not already exist in the environment, then string is added to the environment.  If name does exist, then  the
 * value of name in the environment is changed to value.  The string pointed to by string becomes part of the environment, so
 * altering the string changes the environment.
 *
 * The putenv() function returns zero on success, or -1 if an error occurs.
 *
 * SmallBASIC:
 * If the value is zero-length then the variable must be deleted. (libc4,libc5 compatible version)
 */
int dev_putenv(const char *str) {
#if defined(_VTOS)
  return -1;
#elif defined(_PalmOS)
  return putenv(str);
#else
  char *p;

  p = strdup(str);              // no free()
  return putenv(p);
#endif
}

/**
 * GNU Manual:
 *
 * The  getenv() function searches the environment list for a string that matches the string pointed to by name. The strings
 * are of the form name = value.
 *
 * The getenv() function returns a pointer to the value in the environment, or NULL if there is no match.
 */
char *dev_getenv(const char *str) {
#if defined(_VTOS)
  return -1;
#else
  return getenv(str);
#endif
}

#if !defined(_FRANKLIN_EBM)

/**
 * returns the number of environment variables
 */
int dev_env_count() {
#if defined(_PalmOS)
  return dbt_count(env_table);
#elif defined(_VTOS)
  return 0;
#else
  int count = 0;

  while (environ[count]) {
    count++;
  }
  return count;
#endif
}

/**
 * returns the value of the n-th system's environment variable
 */
char *dev_getenv_n(int n) {
#if defined(_PalmOS)
  dbt_var_t nd;
  char *buf;
  char *retptr;

  if (n < dev_env_count()) {
    dbt_read(env_table, n, &nd, sizeof(nd));
    buf = tmp_alloc(nd.node_len);
    dbt_read(env_table, n, buf, nd.node_len);
    retptr = dev_getenv(buf + sizeof(nd));  // use getenv's static-buffer
    tmp_free(buf);
    return retptr;
  }
  return NULL;
#elif defined(_VTOS)
  return NULL;
#else
  int count = 0;

  while (environ[count]) {
    if (n == count) {
      return environ[count];
    }
    count++;
  }
  return NULL;
#endif
}

#endif // _FRANKLIN_EBM
#endif // IMPL_DEV_ENV
