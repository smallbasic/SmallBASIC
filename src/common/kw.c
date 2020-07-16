// This file is part of SmallBASIC
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sys.h"
#include "common/kw.h"
#include "common/var.h"
#include "common/scan.h"

/*
 * functions without parameters
 */
bid_t kw_noarg_func_table[] = {
  kwINKEY,
  kwTIME,
  kwDATE,
  kwTICKS,
  kwTIMER,
  kwPROGLINE,
  kwFREEFILE,
  kwXPOS,
  kwYPOS,
  kwRND,
  0
};

/*
 * valid exit codes from eval
 */
int kw_check_evexit(code_t code) {
  switch (code) {
  case kwTYPE_EOC:
  case kwTYPE_LINE:
  case kwTYPE_SEP:
  case kwFILLED:
  case kwCOLOR:
  case kwUSE:
  case kwTO:
  case kwIN:
  case kwSTEP:
  case kwFORSEP:
  case kwINPUTSEP:
  case kwINPUT:
  case kwOUTPUTSEP:
  case kwAPPENDSEP:
  case kwAS:
  case kwUSING:
  case kwTHEN:
  case kwDO:
  case kwBACKG:
    return 1;
  default:
    return 0;
  }
}

int kw_getcmdname(code_t code, char *dest) {
  int found = 0;

  *dest = '\0';
  for (int i = 0; keyword_table[i].name[0] != '\0'; i++) {
    if (code == keyword_table[i].code) {
      strcpy(dest, keyword_table[i].name);
      found++;
      break;
    }
  }
  if (!found) {
    sprintf(dest, "(%d)", (int) code);
  }
  return found;
}

int kw_getfuncname(bid_t code, char *dest) {
  int found = 0;

  *dest = '\0';
  for (int i = 0; func_table[i].name[0] != '\0'; i++) {
    if (code == func_table[i].fcode) {
      strcpy(dest, func_table[i].name);
      found++;
      break;
    }
  }
  if (!found) {
    sprintf(dest, "(%d)", code);
  }
  return found;
}

int kw_getprocname(bid_t code, char *dest) {
  int found = 0;

  *dest = '\0';
  for (int i = 0; proc_table[i].name[0] != '\0'; i++) {
    if (code == proc_table[i].pcode) {
      strcpy(dest, proc_table[i].name);
      found++;
      break;
    }
  }
  if (!found) {
    sprintf(dest, "(%d)", code);
  }
  return found;
}

int kw_noarg_func(bid_t code) {
  for (int i = 0; kw_noarg_func_table[i] != 0; i++) {
    if (kw_noarg_func_table[i] == code) {
      return 1;
    }
  }
  return 0;
}

