// This file is part of SmallBASIC
//
// SmallBASIC help subsystem (it is used from console, -h option)
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sys.h"
#include <ctype.h>
#include "doc/guide.c"

/*
*	returns the index of the 'command' in the help data
*/
int help_find(const char *command, int start) {
  int i;
  char *ukey, *u;
  const char *p;

  ukey = u = (char *)malloc(strlen(command) + 1);

  /*
   * prepare 'command' 
   */
  p = command;
  while (*p)
    *u++ = toupper(*p++);
  *u = '\0';

  /*
   * search 
   */
  for (i = start; help_data[i].code; i++) {
    if (strcmp(help_data[i].code, ukey) == 0) {
      free(ukey);
      return i;
    }
  }

  free(ukey);
  return -1;
}

/*
*	get description of 'command'
*/
const char *help_getinfo(const char *command, int start) {
  int idx;

  idx = help_find(command, start);
  if (idx != -1)
    return help_data[idx].descr;
  return NULL;
}

/*
*	get syntax of 'command'
*/
const char *help_getsyntax(const char *command, int start) {
  int idx;

  idx = help_find(command, start);
  if (idx != -1)
    return help_data[idx].syntax;
  return NULL;
}

/*
*	prints to stdout the help text of a command
*/
void help_printinfo(const char *command) {
  int idx, count = 0;

  idx = help_find(command, 0);
  while (idx != -1) {
    printf("HELP SUBSYSTEM: %s %s\n", help_data[idx].type, help_data[idx].code);
    printf("SYNTAX:\n\t%s\n\n", help_data[idx].syntax);
    printf("DESCRIPTION:\n%s\n", help_data[idx].descr);
    idx = help_find(command, idx + 1);
    count++;
  }

  if (count == 0)
    printf("HELP SUBSYSTEM: There is no info for '%s'\n", command);
}

/*
*	gets all available info about command
*	returns a newly allocated string (malloc()) or NULL
*/
char *help_getallinfo(const char *command) {
  int idx, count = 0, size = 0, newsize;
  char *str = NULL, *buf;

  idx = help_find(command, 0);
  while (idx != -1) {
    newsize = strlen(help_data[idx].type) + strlen(help_data[idx].code)
      + strlen(help_data[idx].syntax)
      + strlen(help_data[idx].descr)
      + 64;                     /* add some more for headers */
    buf = (char *)malloc(newsize);

    sprintf(buf,
            "HELP SUBSYSTEM: %s %s\nSYNTAX:\n\t%s\n\nDESCRIPTION:\n%s\n",
            help_data[idx].type, help_data[idx].code, help_data[idx].syntax, help_data[idx].descr);

    if (!str) {
      str = (char *)malloc(size + newsize);
      *str = '\0';
    } else
      str = (char *)realloc(str, size + newsize);
    strcat(str, buf);
    size += newsize;

    // next
    idx = help_find(command, idx + 1);
    count++;
  }

  return str;
}
