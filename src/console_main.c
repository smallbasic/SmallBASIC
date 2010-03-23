// $Id$
// This file is part of SmallBASIC
//
// SmallBASIC, main() console versions
// @see rev 1.12 for original version
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

#include "sbapp.h"

#ifdef INTERACTIVE_CONSOLE
#include "interactive_mode.h"
#ifndef HELP_SUBSYS
#define HELP_SUBSYS
#endif
#endif

#ifdef HELP_SUBSYS
#include "help_subsys.h"
#endif

#if defined(_SDL) || defined(_Win32)
// sb prefix used to avoid name conflict with SDL
#define MAIN_FUNC sb_console_main
#else
#define MAIN_FUNC main
#endif

// global filename (its needed for CTRL+C signal - delete temporary)
char g_file[OS_PATHNAME_SIZE + 1];

/*
 * remove temporary files
 *
 * its called by atexit() only if the
 * source file it had been created in /tmp
 */
void remove_temp_file(void)
{
  unlink(g_file);
}

/*
 * sets the BASDIR environment variable from the input file and
 * current working directory. BASDIR indicates the directory
 * location of the running program
 */
void set_bas_dir(const char *cwd, const char *bas_file)
{
  char bas_dir[OS_PATHNAME_SIZE + 10];
  int path_len = strrchr(bas_file, OS_DIRSEP) - bas_file;
  
  bas_dir[0] = 0;
  strcat(bas_dir, "BASDIR=");
  
  if (bas_file[0] == OS_DIRSEP) {
    // full path
    strncat(bas_dir, bas_file, path_len + 1);
  }
  else if (path_len > 0) {
    // relative path
    // append the non file part of bas_file to cwd
    strcat(bas_dir, cwd);
    strcat(bas_dir, "/");
    strncat(bas_dir, bas_file, path_len + 1);
  }
  else {
    // in current dir
    strcat(bas_dir, cwd);
    strcat(bas_dir, "/");
  }
  dev_putenv(bas_dir);
}

/*
 *   Generic help-page
 */
void show_help()
{
  printf("usage: sbasic [options] source [--] [program parameters]\n");
  printf("-c      syntax check (compile only)\n");
  printf("-g      enable graphics\n");
  printf("-g[<width>x<height>[x<bpp>]]\n");
  printf("        enable graphics & setup the graphics mode (depends on driver)\n");
  printf("-m [mod1,mod2,...]\n");
  printf("        load all or the specified modules\n");
  printf("-pkw    print all keywords \n");
  printf("        (for creating editor color-syntax macros)\n");
  printf("-s      decompiler\n");
  printf("-u      set unit path\n");
  printf("-v      verbose\n");
  printf("-x      output compiled SBX file\n");
  // -i for ide
  /*
   * fprintf(stderr, "\ncharset (default: utf8)\n");
   * fprintf(stderr, "-j enable sjis\n");
   * fprintf(stderr, "-b enable big5\n");
   * fprintf(stderr, "-m enable generic
   * multibyte\n"); fprintf(stderr, "-u enable
   * unicode!\n");
   */
  printf("\nExamples:\n\tsbasic -h | less\n\tsbasic -h-input\n");
}

/*
 * handles the command "sbasic -pkw"
 */
void print_keywords()
{
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

  // external procedures
  // ....
  exit(1);
}

/*
 * process command-line parameters
 */
int process_options(int argc, char *argv[])
{
  int i;
  int opt_ihavename = 0;
  int opt_nomore = 0;
  char buff[128];

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (opt_nomore) {
        if (strlen(opt_command) + strlen(argv[i]) + 2 < OPT_CMD_SZ) {
          // +1 for space +1 for the trailing zero
          strcat(opt_command, " ");
          strcat(opt_command, argv[i]);
        }
        else {
          fprintf(stderr, "Too long command line! (%s)\n", argv[i]);
          return 1;
        }
      }
      else {
        switch (argv[i][1]) {
        case '-':
          // the following parameters are going to script
          // (COMMAND$)
          opt_nomore = 1;
          break;

        case 's':
          // decompile
          opt_decomp++;
          break;

        case 'c':
          // syntax check
          opt_syntaxcheck++;
          break;

        case 'u':
          sprintf(buff, "UNITPATH=%s", &argv[i][2]);
          dev_putenv(buff);
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

        case 'g':
          // run in graphics mode
          opt_graphics = 2;
          if ((argv[i][2] >= '1') && (argv[i][2] <= '9')) {
            // setup graphics mode
            char *mode = &argv[i][2];
            sprintf(buff, "SBGRAF=%s", mode);
            dev_putenv(buff);
            comp_preproc_grmode(mode);
            opt_graphics = 2;
          }
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
          // shutup
          opt_quiet = 1;
          break;

        case 'h':
          // print command-line parameters
          fprintf(stdout,
                  "SmallBASIC version %s - kw:%d, pc:%d, fc:%d, ae:%d\n",
                  SB_STR_VER, kwNULL, (kwNULLPROC - kwCLS) + 1,
                  (kwNULLFUNC - kwASC) + 1, (int)(65536 / sizeof(var_t)));
          fprintf(stdout, "http://smallbasic.sourceforge.net\n\n");

          if (argv[i][2] == '-' || argv[i][2] == 'x') {
            /*
             *   search for command, or print all doc
             */
            if (argv[i][2] == '-') {
#ifdef HELP_SUBSYS
              char *command = argv[i] + 3;
              help_printinfo(command);
#endif
            }
            else if (argv[i][2] == 'x') {
              // print all
              // printf("%s\n", help_text);
              ;
            }
          }
          else {
            show_help();
          }
          return 1;

        default:
          fprintf(stderr, "unknown option: %s\n", argv[i]);
          return 1;
        };
      }
    }
    else {
      // no - switch
      // this is the filename or script-parameters
      if (opt_ihavename == 0) {
        strcpy(g_file, argv[i]);
        if (access(g_file, F_OK)) {
          strcat(g_file, ".bas");
          if (access(g_file, F_OK)) {
            fprintf(stderr, "file not accessible - %s\n", g_file);
            return 1;
          }
        }
        if (access(g_file, R_OK)) {
          fprintf(stderr, "file not readable - %s\n", g_file);
          return 1;
        }
        opt_ihavename = 1;
      }
      else {
        if (strlen(opt_command) + strlen(argv[i]) + 2 < OPT_CMD_SZ) {
          // +1 for space +1 for the trailing zero
          strcat(opt_command, " ");
          strcat(opt_command, argv[i]);
        }
        else {
          fprintf(stderr, "Too long command line! (%s)\n", argv[i]);
          return 1;
        }
      }
    }
  }

  // initialization
  if (strlen(g_file) == 0) {
    // stdin
    if (isatty(STDIN_FILENO)) {
      // check if it is a terminal.
      opt_interactive = 1;
    }

#if defined(_Win32) || defined(_DOS)
    char *slash = strchr(argv[0], OS_DIRSEP);
    if (slash) {
      strcpy(g_file, argv[0]);
      slash = strrchr(g_file, OS_DIRSEP);
      *slash = OS_DIRSEP;
      *(slash + 1) = '\0';
      strcat(g_file, "sbasic.tmp");
    }
    else {
      sprintf(g_file, "sbasic.tmp");
    }
#elif defined(_UnixOS)
    sprintf(g_file, "%ctmp%csb%d.bas", OS_DIRSEP, OS_DIRSEP, getpid());
#else
    sprintf(g_file, "sb%d.bas", getpid());  // for minimal GNU systems like 
    // MINGW
#endif

    // its a temporary and it must be deleted
    atexit(remove_temp_file);

    if (opt_interactive) {
      // get it from console
#ifdef INTERACTIVE_CONSOLE
#ifndef HAVE_C_MALLOC
      memmgr_init();
#endif
      interactive_mode(g_file);
#endif
    }
    else {
      // get it from stdin
      FILE *fp = fopen(g_file, "wb");
      int c;
      if (fp) {
        while ((c = fgetc(stdin)) != EOF) {
          fputc(c, fp);
        }
        fclose(fp);
      }
      else {
        fprintf(stderr, "file not writeable - %s\n", g_file);
        return 1;
      }
    }
  }
  return 0;
}

/*
 * program entry point
 */
int MAIN_FUNC(int argc, char *argv[])
{
  char prev_cwd[OS_PATHNAME_SIZE + 1];

#ifdef _SDL
  opt_graphics = 2;             // we need to set default options here for SDL
#else
  opt_graphics = 0;
#endif
  opt_quiet = 1;
  opt_ide = 0;
  opt_nosave = 1;
  opt_pref_width = opt_pref_height = opt_pref_bpp = 0;
  opt_verbose = 0;

  // init strings
  opt_command[0] = 0;
  opt_modlist[0] = 0;
  prev_cwd[0] = 0;
  g_file[0] = 0;

  getcwd(prev_cwd, sizeof(prev_cwd) - 1);

  opt_retval = process_options(argc, argv);

  if (!opt_retval) {
    set_bas_dir(prev_cwd, g_file);

    if (!opt_quiet) {
      printf("SmallBASIC version %s, use -h for help\n", SB_STR_VER);
    }

    // run it
    if (!opt_interactive) {
#ifndef HAVE_C_MALLOC
      memmgr_init();
#endif
      sbasic_main(g_file);
    }

    chdir(prev_cwd);
  }

  return gsb_last_error ? gsb_last_error : opt_retval;
}
