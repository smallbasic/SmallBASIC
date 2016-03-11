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
 * valid exit codes from eval
 */
code_t kw_eval_validexit[] = { 
  kwTYPE_EOC, 
  kwTYPE_LINE, 
  kwTYPE_SEP, 
  kwFILLED, 
  kwCOLOR, 
  kwUSE, 
  kwTO, 
  kwIN,
  kwSTEP,
  kwFORSEP,
  kwINPUTSEP,
  kwINPUT, 
  kwOUTPUTSEP, 
  kwAPPENDSEP,
  kwAS,
  kwUSING,
  kwTHEN,
  kwDO,
  kwBACKG,
  0
};

/*
 * functions without parameters
 */
fcode_t kw_noarg_func_table[] = { 
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

//
int kw_check(code_t *table, code_t code) {
  register int i;

  for (i = 0; table[i] != 0; i++) {
    if (code == table[i]) {
      return 1;
    }
  }
  return 0;
}

//
int kw_check_evexit(code_t code) {
  return kw_check(kw_eval_validexit, code);
}

/*
 */
int kw_getcmdname(code_t code, char *dest) {
  int i;
  int found = 0;

  *dest = '\0';
  for (i = 0; keyword_table[i].name[0] != '\0'; i++) {
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

/*
 */
int kw_getfuncname(fcode_t code, char *dest) {
  int i;
  int found = 0;

  *dest = '\0';
  for (i = 0; func_table[i].name[0] != '\0'; i++) {
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

/*
 */
int kw_getprocname(pcode_t code, char *dest) {
  int i;
  int found = 0;

  *dest = '\0';
  for (i = 0; proc_table[i].name[0] != '\0'; i++) {
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

/*
 */
int kw_noarg_func(fcode_t code) {
  int i;

  for (i = 0; kw_noarg_func_table[i] != 0; i++) {
    if (kw_noarg_func_table[i] == code) {
      return 1;
    }
  }
  return 0;
}

/*
 */
int kw_iscommand(const char *name) {
  int i;

  for (i = 0; keyword_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(name, keyword_table[i].name) == 0) {
      return -1;
    }
  }
  return 0;
}

/*
 */
int kw_isproc(const char *name) {
  int i;

  for (i = 0; proc_table[i].name[0] != '\0'; i++) {
    if (strcasecmp(name, proc_table[i].name) == 0) {
      return -1;
    }
  }
  return 0;
}
