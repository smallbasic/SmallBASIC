// This file is part of SmallBASIC
//
// decompiler
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sbapp.h"

/**
 *
 * decompiler
 *
 */
void dump_bytecode(FILE * output) {
  int i, c, b, d, h, l, j;
  long lng;
  int len, new_ip;
  char cmd[33];
  FILE *fp;
  char_p_t *source = NULL;
  int lcount = 0, lsize = 0;

  // ///////////////
  // read source
  if (strstr(prog_file, ".bas")) {
    fp = fopen(prog_file, "rt");
    if (fp) {
      char buf[512];

      lsize = 1024;
      source = tmp_alloc(lsize * sizeof(char_p_t));
      lcount = 0;
      while (fgets(buf, 511, fp)) {
        source[lcount] = tmp_alloc(strlen(buf) + 1);
        strcpy(source[lcount], buf);
        lcount++;
        if (lcount >= lsize) {
          lsize += 1024;
          source = tmp_realloc(source, lsize * sizeof(char_p_t));
        }
      }
      fclose(fp);
    }
  }

  // ///////////////
  fprintf(output, "  program size   = %d\n", (int) prog_length);
  fprintf(output, "  variable-count = %d\n", (int) prog_varcount);
  fprintf(output, "  label-count    = %d\n", (int) prog_labcount);
  fprintf(output, "  lib-count      = %d\n", (int) prog_libcount);
  fprintf(output, "  symbol-count   = %d\n", (int) prog_symcount);

  // //////////////////
  // print label-table
  if (prog_labcount) {
    fprintf(output, "* label-table:\n");
    for (i = 0; i < prog_labcount; i++) {
      fprintf(output, "  %d = 0x%X (%d)\n", i, (int) tlab[i].ip, (int) tlab[i].ip);
    }
  } else {
    fprintf(output, "* no label-table\n");
  }

  // //////////////////
  // print libs
  if (prog_libcount) {
    fprintf(output, "* linked-libraries-table:\n");
    for (i = 0; i < prog_libcount; i++) {
      fprintf(output, "  %s: type '%s', lid 0x%0X\n", prog_libtable[i].lib,
          ((prog_libtable[i].type == 1) ? "unit" : "c-module"),
          ((prog_libtable[i].type ==
                  1) ? prog_libtable[i].id | UID_UNIT_BIT : prog_libtable[i].id)
          );
        }
      }
      else {
        fprintf(output, "* no linked-libraries\n");
      }

      // //////////////////
      // print symbol-table
  if (prog_symcount) {
    fprintf(output, "* import-symbol-table:\n");
    for (i = 0; i < prog_symcount; i++) {
      fprintf(output, "  %s: type %s, lid 0x%0X, sid %d, vid %d - tid %d, eidx %d\n", prog_symtable[i].symbol,
          (prog_symtable[i].type == stt_variable) ? "variable" : ((prog_symtable[i].type ==
                  stt_procedure) ? "procedure" :
              "function"), prog_symtable[i].lib_id,
          prog_symtable[i].sym_id, prog_symtable[i].var_id,
          prog_symtable[i].task_id, prog_symtable[i].exp_idx);
        }
      }
      else {
        fprintf(output, "* no import-symbol-table\n");
      }

      // //////////////////
      // print export symbol-table
  if (prog_expcount) {
    fprintf(output, "* export-symbol-table:\n");
    for (i = 0; i < prog_expcount; i++) {
      fprintf(output, "  %s: type %s, address %d, related variable id %d\n", prog_exptable[i].symbol,
          (prog_exptable[i].type == stt_variable) ? "variable" : ((prog_exptable[i].type ==
                  stt_procedure) ? "procedure" :
              "function"), prog_exptable[i].address,
          prog_exptable[i].vid);
        }
      }
      else {
        fprintf(output, "* no export-symbol-table\n");
      }

      // //////////////////
      // print code
  fprintf(output, "* byte-code:\n");
  do {
    b = code_getnext();
    h = b >> 4;
    l = b & 0xF;

    fprintf(output, "  %08d: %c%c %c; ", (int) prog_ip - 1, "0123456789ABCDEF"[h], "0123456789ABCDEF"[l],
        (b >= 32) ? b : 42);

    switch (b) {
    case kwTYPE_LINE:
      c = code_getaddr();
      if (source && lcount >= c) {
        fprintf(output, "line: %d\n%d: ", c, c);
        fprintf(output, "%s", source[c - 1]);
      } else {
        fprintf(output, "line: %d", c);
      }
      break;
    case kwTYPE_EVPUSH:
      fprintf(output, "push (r)esult");
      break;
    case kwTYPE_EVPOP:
      fprintf(output, "pop (l)eft");
      break;
    case kwTYPE_EOC:
      fprintf(output, "end-of-command");
      break;
    case kwTYPE_CALLF:
      // call build-in function
      lng = code_getaddr();
      kw_getfuncname(lng, cmd);
      fprintf(output, "%s; (build-in function %ld)", cmd, lng);
      break;
    case kwTYPE_CALLP:
      // call build-in procedure
      lng = code_getaddr();
      kw_getprocname(lng, cmd);
      fprintf(output, "%s; (build-in procedure %ld)", cmd, lng);
      break;
    case kwTYPE_CALLEXTF:
    case kwTYPE_CALLEXTP:      // [lib][index]
      // call external (module) function/procedure
      c = code_getaddr();
      d = code_getaddr();
      if (b == kwTYPE_CALLEXTF) {
        if (c & UID_UNIT_BIT) {
          fprintf(output, "call external (unit) function; lid 0x%X, sid %d", c, d);
        } else {
          sblmgr_getfuncname(c, prog_symtable[d].exp_idx, cmd);
          fprintf(output, "call external (c-lib) function; lid 0x%X, sid %d (%s)", c, d, cmd);
        }
      } else {
        if (c & UID_UNIT_BIT) {
          fprintf(output, "call external (unit) procedure; lid 0x%X, sid %d", c, d);
        } else {
          sblmgr_getprocname(c, prog_symtable[d].exp_idx, cmd);
          fprintf(output, "call external (c-lib) procedure; lid 0x%X, sid %d (%s)", c, d, cmd);
        }
      }
      break;
    case kwPROC:
      fprintf(output, "=== user defined procedure begin");
      break;
    case kwFUNC:
      fprintf(output, "=== user defined function begin");
      break;
    case kwFILEPRINT:
      fprintf(output, "file print");
      break;
    case kwFILEINPUT:
      fprintf(output, "file input");
      break;
    case kwLINEINPUT:
      fprintf(output, "line input");
      break;
    case kwFORSEP:
      fprintf(output, "for-sep");
      break;
    case kwLOOPSEP:
      fprintf(output, "loop-sep");
      break;
    case kwINPUTSEP:
      fprintf(output, "input-sep");
      break;
    case kwAPPEND:
      fprintf(output, "append");
      break;
    case kwCODEARRAY:
      fprintf(output, "array");
      break;
    case kwTYPE_PARAM:
    case kwTYPE_CRVAR:
      c = code_getnext();
      if (b == kwTYPE_PARAM) {
        fprintf(output, "parameter %d: ", c);
      } else {
        fprintf(output, "local variable %d: ", c);
      }

      d = 0;
      for (j = 0; j < c; j++) {
        if (b == kwTYPE_PARAM) {
          d = code_getnext();
          if (d & 0x80) {
            fprintf(output, "&");
          }
        }
        fprintf(output, "%d ", code_getaddr());
      }
      break;
    case kwUSE:
      fprintf(output, "use %d ", code_getaddr());
      fprintf(output, "exit %d", code_getaddr());
      break;
    case kwTYPE_RET:
      fprintf(output, "=== return; user-defined proc/func end");
      break;
    case kwTYPE_CALL_UDP:
      // call user-defined-procedure
      fprintf(output, "call user-defined procedure %d", code_getaddr());
      code_skipaddr();
      break;
    case kwTYPE_CALL_UDF:
      // call user-defined-function
      fprintf(output, "call user-defined function %d", code_getaddr());
      fprintf(output, ", return-variable: %d", code_getaddr());
      break;
    case kwEXIT:
      fprintf(output, "exit ");
      c = code_getnext();
      switch (c) {
      case kwFORSEP:
        fprintf(output, "for");
        break;
      case kwLOOPSEP:
        fprintf(output, "loop");
        break;
      case kwPROCSEP:
        fprintf(output, "procedure");
        break;
      case kwFUNCSEP:
        fprintf(output, "function");
        break;
      default:
        fprintf(output, "last-stack-node (undefined)");
      }
      ;

      break;

    default:
      for (c = 0; keyword_table[c].name[0] != '\0'; c++) {
        if (b == keyword_table[c].code) {
          fprintf(output, "%s ", keyword_table[c].name);
          break;
        }
      }

      switch (b) {
      case kwTYPE_SEP:
        c = code_getnext();
        fprintf(output, "\'%c\' ", (c >= 32) ? c : '?');
        break;
      case kwTYPE_LOGOPR:
      case kwTYPE_CMPOPR:
      case kwTYPE_ADDOPR:
      case kwTYPE_MULOPR:
      case kwTYPE_POWOPR:
      case kwTYPE_UNROPR:
        c = code_getnext();
        fprintf(output, "data %d \'%c\' ", c, (c >= 32) ? c : '?');
        break;
      case kwTYPE_LEVEL_BEGIN:
        fprintf(output, "(");
        break;
      case kwTYPE_LEVEL_END:
        fprintf(output, ")");
        break;
      case kwONJMP:
        new_ip = code_getaddr();
        code_skipaddr();
        b = code_getnext();
        c = code_getnext();
        fprintf(output, "on %s count %d\n", ((b == kwGOTO) ? "goto" : "gosub"), c);
        fprintf(output, "      next address %d\n", new_ip);
        for (j = 0; j < c; j++) {
          fprintf(output, "       jump to %d\n", code_getaddr());
        }
        fprintf(output, "on %s expression:", ((b == kwGOTO) ? "goto" : "gosub"));
        break;
        case kwGOTO:
        new_ip = code_getaddr();
        fprintf(output, "; jump to %d ", new_ip);
        c = code_getnext();
        fprintf(output, "; pop %d stack nodes", c);
        break;
        case kwOPTION:
        c = code_getnext();
        new_ip = code_getaddr();
        fprintf(output, " option (%d) = %d", c, new_ip);
        break;
        case kwGOSUB:
        c = code_getaddr();
        fprintf(output, "id %d ", c);
        new_ip = tlab[c].ip;
        fprintf(output, "; jump to %d ", new_ip);
        break;
        case kwLET:
        break;
        case kwRESTORE:
        case kwTYPE_LINE:
        case kwLABEL:
        case kwTYPE_VAR:
        fprintf(output, "id %d ", code_getaddr());
        break;
        case kwTYPE_INT:
        lng = code_getint();
        fprintf(output, "value (int) %d (0x%X) ", (int)lng, (int)lng);
        break;
        case kwTYPE_NUM:
        fprintf(output, "value (real) " VAR_NUM_FMT " ", code_getreal());
        break;
        case kwTYPE_STR:
        len = code_getstrlen();
        fprintf(output, "\"");
        for (j = 0; j < len; j++) {
          fprintf(output, "%c", prog_source[prog_ip + j]);
        }
        fprintf(output, "\"");
        prog_ip += len;
        break;
        case kwIF:
        case kwFOR:
        case kwWHILE:
        case kwREPEAT:
        case kwELSE:
        case kwELIF:
        case kwENDIF:
        case kwNEXT:
        case kwWEND:
        case kwUNTIL:
        fprintf(output, "address1 (true?) %d, address2 (false?) %d ", code_getaddr(),
            code_getaddr());
        break;
      }
    }

    fprintf(output, "\n");
  } while (prog_ip < prog_length);

  // ///////////////
  // delete source
  if (source) {
    for (i = 0; i < lcount; i++) {
      tmp_free(source[i]);
    }
    tmp_free(source);
  }
}
