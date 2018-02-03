// This file is part of SmallBASIC
//
// SmallBASIC, main() console versions
// @see rev 1.12 for original version
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "common/sbapp.h"

// global filename (its needed for CTRL+C signal - delete temporary)
char g_file[OS_PATHNAME_SIZE + 1];

// decompile handling
int decomp;
void dump_bytecode(FILE *output);
int sbasic_compile(const char *file);
int sbasic_exec_prepare(const char *file);
int exec_close(int tid);

/*
 * remove temporary files
 *
 * its called by atexit() only if the
 * source file it had been created in /tmp
 */
void remove_temp_file(void) {
  unlink(g_file);
}

/*
 * Generic help-page
 */
void show_help() {
  printf("usage: sbasic [options] source [--] [program parameters]\n");
  printf("-c      program passed in as a string\n");
  printf("-m      [mod1,mod2,...]\n");
  printf("        load all or the specified modules\n");
  printf("-pkw    print all keywords \n");
  printf("        (for creating editor color-syntax macros)\n");
  printf("-s      decompiler\n");
  printf("-u      set unit path\n");
  printf("-v      verbose\n");
  printf("-x      output compiled SBX file\n");
  printf("\nExamples:\n\tsbasic -h | less\n\tsbasic -h-input\n");
}

void show_brief_help() {
  fprintf(stdout, "SmallBASIC version %s - use -h for help\n",  SB_STR_VER);
  fprintf(stdout, "https://smallbasic.sourceforge.io\n\n");
}

/*
 * handles the command "sbasic -pkw"
 */
void print_keywords() {
  int j;

  printf("SmallBASIC keywords table\n");
  printf("::':#:rem:\"\n");     // ted's format
  printf("$$$-remarks\n");
  printf("'\n");
  printf("REM\n");

  // operators
  printf("$$$-operators\n");
  printf("() \"\"\n");
  printf("%s\n", "+ - * / \\ % ^");
  printf("%s\n", "= <= =< >= => <> != !");
  printf("%s\n", "&& & || | ~");
  for (j = 0; opr_table[j].name[0] != '\0'; j++) {
    printf("%s\n", opr_table[j].name);
  }

  // print keywords
  printf("$$$-keywords\n");
  for (j = 0; keyword_table[j].name[0] != '\0'; j++) {
    if (keyword_table[j].name[0] != '$') {
      printf("%s\n", keyword_table[j].name);
    }
  }

  // special separators
  for (j = 0; spopr_table[j].name[0] != '\0'; j++) {
    printf("%s\n", spopr_table[j].name);
  }

  // functions
  printf("$$$-functions\n");
  for (j = 0; func_table[j].name[0] != '\0'; j++) {
    printf("%s\n", func_table[j].name);
  }

  // procedures
  printf("$$$-procedures\n");
  for (j = 0; proc_table[j].name[0] != '\0'; j++) {
    printf("%s\n", proc_table[j].name);
  }

  exit(1);
}

/*
 * setup to run a program passed from the command line
 */
int setup_file(const char *program) {
  static const char *ENV_VARS[] = {
    "TMP", "TEMP", "TMPDIR", "HOME", "APPDATA", ""
  };

  for (int i = 0; ENV_VARS[i][0] != '\0'; i++) {
    const char *tmp = getenv(ENV_VARS[i]);
    if (tmp && access(tmp, R_OK) == 0) {
      strcpy(g_file, tmp);
      strcat(g_file, "/sbasic.tmp");
      break;
    }
  }

  FILE *fp = fopen(g_file, "wb");
  if (fp) {
    //atexit(remove_temp_file);
    fputs(program, fp);
    fputc('\n', fp);
    fclose(fp);
    return 1;
  } else {
    fprintf(stderr, "file not writeable - %s\n", g_file);
    return 0;
  }
}

/*
 * process command-line parameters
 */
int process_options(int argc, char *argv[]) {
  int opt_ihavename = 0;
  int opt_nomore = 0;
  decomp = 0;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (opt_nomore) {
        if (strlen(opt_command) + strlen(argv[i]) + 2 < OPT_CMD_SZ) {
          // +1 for space +1 for the trailing zero
          strcat(opt_command, " ");
          strcat(opt_command, argv[i]);
        } else {
          fprintf(stderr, "command too long (%s)\n", argv[i]);
          return 0;
        }
      } else {
        switch (argv[i][1]) {
        case '-':
          // the following parameters are going to script (COMMAND$)
          opt_nomore = 1;
          break;

        case 's':
          // decompile
          decomp = 1;
          opt_nosave = 1;
          break;

        case 'c':
          if (i + 1 < argc) {
            opt_ihavename = 1;
            if (!setup_file(argv[i + 1])) {
              // failed to setup file
              return 0;
            }
          } else {
            fprintf(stderr, "argument expected for -c option\n");
            show_brief_help();
            return 0;
          }
          break;

        case 'u':
          dev_setenv("UNITPATH", &argv[i][2]);
          break;

        case 'v':
          // verbose check
          if (!opt_quiet) {
            // -v -v
            opt_verbose = 1;
          }
          opt_quiet = 0;
          break;

        case 'i':
          opt_ide = IDE_EXTERNAL;
          break;

        case 'x':
          opt_nosave = 0;
          break;

        case 'p':
          if (strcmp(argv[i] + 1, "pkw") == 0) {
            print_keywords();
          }
          break;

        case 'm':
          // load run-time modules
          opt_loadmod = 1;
          if (i + 1 < argc) {
            strcpy(opt_modlist, argv[++i]);
          }
          break;

        case 'q':
          opt_quiet = 1;
          break;

        case 'h':
          // print command-line parameters
          show_brief_help();
          show_help();
          return 0;

        default:
          fprintf(stderr, "unknown option: %s\n", argv[i]);
          show_brief_help();
          return 0;
        };
      }
    } else {
      // no - switch
      /// this is the filename or script-parameters
      if (opt_ihavename == 0) {
        strcpy(g_file, argv[i]);
        if (access(g_file, F_OK)) {
          strcat(g_file, ".bas");
          if (access(g_file, F_OK)) {
            fprintf(stderr, "file not accessible - %s\n", g_file);
            return 0;
          }
        }
        if (access(g_file, R_OK)) {
          fprintf(stderr, "file not readable - %s\n", g_file);
          return 0;
        }
        opt_ihavename = 1;
      } else {
        if (strlen(opt_command) + strlen(argv[i]) + 2 < OPT_CMD_SZ) {
          // +1 for space +1 for the trailing zero
          strcat(opt_command, " ");
          strcat(opt_command, argv[i]);
        } else {
          fprintf(stderr, "command too long (%s)\n", argv[i]);
          return 0;
        }
      }
    }
  }
  // success
  return 1;
}

#if defined(__GNUC__) && !defined(__MACH__) && !defined(_Win32)
// for analysing excessive malloc calls using kdbg
extern void *__libc_malloc(size_t size);
void *malloc(size_t size) {
  return __libc_malloc(size);
}
#endif

void print_taskinfo(FILE *output) {
  int prev_tid = 0;

  fprintf(output, "\n* task list:\n");
  for (int i = 0; i < count_tasks(); i++) {
    prev_tid = activate_task(i);
    fprintf(output, "  id %d, child of %d, file %s, status %d\n",
            ctask->tid, ctask->parent, ctask->file, ctask->status);
  }
  activate_task(prev_tid);
}

void print_bytecode(int tid, FILE *output) {
  int prev_tid = activate_task(tid);
  fprintf(stderr, "%d %d %d\n", prev_tid, tid, count_tasks());

  for (int i = 0; i < count_tasks(); i++) {
    activate_task(i);
    if (ctask->parent == tid) {
      // do the same for the childs
      print_bytecode(ctask->tid, output);
      exit(1);
    }
  }

  activate_task(tid);
  fprintf(output, "\n* task: %d/%d (%s)\n", ctask->tid, count_tasks(), prog_file);
  dump_bytecode(output);
  activate_task(prev_tid);
}

void decompile() {
  init_tasks();
  unit_mgr_init();
  sblmgr_init(0, NULL);
  if (sbasic_compile(g_file)) {
    int exec_tid = sbasic_exec_prepare(g_file);
    print_taskinfo(stdout);
    print_bytecode(exec_tid, stdout);
    exec_close(exec_tid);
  }
  unit_mgr_close();
  sblmgr_close();
  destroy_tasks();
}

/*
 * program entry point
 */
int main(int argc, char *argv[]) {
  opt_graphics = 0;
  opt_quiet = 1;
  opt_ide = 0;
  opt_nosave = 1;
  opt_pref_width = opt_pref_height = 0;
  opt_verbose = 0;
  opt_file_permitted = 1;
  opt_autolocal = 0;
  opt_command[0] = 0;
  opt_modlist[0] = 0;
  g_file[0] = '\0';

  if (process_options(argc, argv)) {
    if (g_file[0]) {
      char prev_cwd[OS_PATHNAME_SIZE + 1];
      prev_cwd[0] = 0;
      getcwd(prev_cwd, sizeof(prev_cwd) - 1);
      if (decomp) {
        decompile();
      } else {
        sbasic_main(g_file);
      }
      chdir(prev_cwd);
    } else {
      show_brief_help();
    }
  }
  return gsb_last_error;
}
