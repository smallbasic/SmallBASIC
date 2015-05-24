// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <SDL.h>
#include <getopt.h>

#include "ui/utils.h"
#include "ui/strlib.h"
#include "platform/sdl/runtime.h"
#include "platform/sdl/settings.h"
#include "platform/sdl/syswm.h"
#include "common/smbas.h"

#if !defined(_Win32)
#include <fontconfig/fontconfig.h>
#endif

extern "C" unsigned
  lodepng_decode32(unsigned char** out, unsigned* w, unsigned* h,
                   const unsigned char* in, size_t insize);

using namespace strlib;

const char* FONTS[] = {
  "Envy Code R",
  "Source Code Pro",
  "Ubuntu Mono",
  "DejaVu Sans Mono",
  "FreeMono",
  "Liberation Mono",
  "Courier New",
  NULL
};

static struct option OPTIONS[] = {
  {"help",     no_argument,       NULL, 'h'},
  {"verbose",  no_argument,       NULL, 'v'},
  {"command",  optional_argument, NULL, 'c'},
  {"font",     optional_argument, NULL, 'f'},
  {"run",      optional_argument, NULL, 'r'},
  {"module",   optional_argument, NULL, 'm'},
  {"keywords", no_argument,       NULL, 'k'},
  {0, 0, 0, 0}
};

const char *CONFIG_NAME = "SmallBASIC";

void appLog(const char *format, ...) {
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof buf - 1, format, args);
  va_end(args);

  while (p > buf && isspace(p[-1])) {
    *--p = '\0';
  }

  *p++ = '\r';
  *p++ = '\n';
  *p = '\0';

#if defined(WIN32)
  if (opt_verbose) {
    fprintf(stderr, buf, 0);
  }
  OutputDebugString(buf);
#else
  fprintf(stderr, buf, 0);
#endif
}

#if defined(_Win32)
bool getFontFiles(const char *familyName, String &fontFile, String &fontFileBold) {
  fontFile = "Envy Code R.ttf";
  fontFileBold = "Envy Code R Bold.ttf";
  if (access(fontFile.c_str(), 0) != 0) {
    fontFile = "SourceCodePro-Regular.ttf";
    fontFileBold = "SourceCodePro-Bold.ttf";
  }
  if ((familyName != NULL && strcasecmp(familyName, "consola") == 0)
      || access(fontFile.c_str(), 0) != 0) {
    fontFile = "c:/Windows/Fonts/consola.ttf";
    fontFileBold = "c:/Windows/Fonts/consolab.ttf";
  }
  if ((familyName != NULL && strcasecmp(familyName, "courier") == 0)
      || access(fontFile.c_str(), 0) != 0) {
    fontFile = "c:/Windows/Fonts/cour.ttf";
    fontFileBold = "c:/Windows/Fonts/courbd.ttf";
  }
  return true;
}
#else
bool getFont(FcFontSet *fs, const char *familyName, int fontWeight, String &name) {
  bool result = false;
  for (int i=0; fs && i < fs->nfont; i++) {
    FcPattern *font = fs->fonts[i];
    FcChar8 *file, *family;
    int spacing, weight, slant;
    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
        FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch &&
        FcPatternGetInteger(font, FC_SPACING, 0, &spacing) == FcResultMatch &&
        FcPatternGetInteger(font, FC_SLANT, 0, &slant) == FcResultMatch &&
        FcPatternGetInteger(font, FC_WEIGHT, 0, &weight) == FcResultMatch) {
      const char *filename = (const char *)file;
      int len = strlen(filename);
      if (spacing == FC_MONO
          && weight == fontWeight
          && slant == 0
          && strcasecmp(filename + len - 4, ".ttf") == 0
          && strcasecmp(familyName, (const char *)family) == 0) {
        name.empty();
        name.append(filename);
        result = true;
      }
    }
  }
  return result;
}

bool getFontFiles(const char *familyName, String &fontFile, String &fontFileBold) {
  bool result = false;
  if (FcInit()) {
    FcConfig *config = FcConfigGetCurrent();
    FcConfigSetRescanInterval(config, 0);
    FcPattern *pat = FcPatternCreate();
    FcObjectSet *os = FcObjectSetBuild(FC_SPACING, FC_WEIGHT, FC_SLANT, FC_FAMILY, FC_FILE, (char *) 0);
    FcFontSet *fs = FcFontList(config, pat, os);

    if (familyName != NULL &&
        getFont(fs, familyName, FC_WEIGHT_REGULAR, fontFile) &&
        getFont(fs, familyName, FC_WEIGHT_BOLD, fontFileBold)) {
      result = true;
    }

    if (familyName != NULL && !result) {
      fprintf(stderr, "Failed to load %s\n", familyName);
    }

    int i = 0;
    while (!result && FONTS[i] != NULL) {
      if (getFont(fs, FONTS[i], FC_WEIGHT_REGULAR, fontFile) &&
          getFont(fs, FONTS[i], FC_WEIGHT_BOLD, fontFileBold)) {
        result = true;
      }
      i++;
    }

    FcFontSetDestroy(fs);
    FcObjectSetDestroy(os);
    FcPatternDestroy(pat);
    FcConfigDestroy(config);
  }
  return result;
}
#endif

void printKeywords() {
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
  for (int j = 0; opr_table[j].name[0] != '\0'; j++) {
    printf("%s\n", opr_table[j].name);
  }

  // print keywords
  printf("$$$-keywords\n");
  for (int j = 0; keyword_table[j].name[0] != '\0'; j++) {
    if (keyword_table[j].name[0] != '$') {
      printf("%s\n", keyword_table[j].name);
    }
  }

  // special separators
  for (int j = 0; spopr_table[j].name[0] != '\0'; j++) {
    printf("%s\n", spopr_table[j].name);
  }

  // functions
  printf("$$$-functions\n");
  for (int j = 0; func_table[j].name[0] != '\0'; j++) {
    printf("%s\n", func_table[j].name);
  }

  // procedures
  printf("$$$-procedures\n");
  for (int j = 0; proc_table[j].name[0] != '\0'; j++) {
    printf("%s\n", proc_table[j].name);
  }
}

void showHelp() {
  fprintf(stdout,
          "SmallBASIC version %s - kw:%d, pc:%d, fc:%d, ae:%d\n\n",
          SB_STR_VER, kwNULL, (kwNULLPROC - kwCLS) + 1,
          (kwNULLFUNC - kwASC) + 1, (int)(65536 / sizeof(var_t)));
  fprintf(stdout, "usage: sbasicg [options]... [BAS] [OPTION]\n");
  int i = 0;
  while (OPTIONS[i].name != NULL) {
    fprintf(stdout, OPTIONS[i].has_arg ?
            "  -%c, --%s='<argument>'\n" : "  -%c, --%s\n",
            OPTIONS[i].val, OPTIONS[i].name);
    i++;
  }
  fprintf(stdout, "\nhttp://smallbasic.sourceforge.net\n\n");
}

int main(int argc, char* argv[]) {
  logEntered();

  opt_command[0] = 0;
  opt_verbose = false;
  opt_quiet = true;

  char *fontFamily = NULL;
  char *runFile = NULL;

  while (1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "vhc:f:r:m:k", OPTIONS, &option_index);
    if (c == -1) {
      // no more options
      if (!option_index) {
        for (int i = 1; i < argc; i++) {
          const char *s = argv[i];
          int len = strlen(s);
          if (runFile == NULL
              && ((strcasecmp(s + len - 4, ".bas") == 0 && access(s, 0) == 0)
                  || (strstr(s, "://") != NULL))) {
            runFile = strdup(s);
          } else if (chdir(s) != 0) {
            strcpy(opt_command, s);
          }
        }
      }
      break;
    }
    if (OPTIONS[option_index].has_arg && !optarg) {
      showHelp();
      exit(1);
    }
    switch (c) {
    case 'v':
      opt_verbose = true;
      opt_quiet = false;
      break;
    case 'c':
      strcpy(opt_command, optarg);
      break;
    case 'f':
      fontFamily = strdup(optarg);
      break;
    case 'r':
      runFile = strdup(optarg);
      break;
    case 'm':
      opt_loadmod = 1;
      strcpy(opt_modlist, optarg);
      break;
    case 'h':
      showHelp();
      exit(1);
      break;
    case 'k':
      printKeywords();
      exit(1);
      break;
    default:
      exit(1);
      break;
    }
  }

  int fontScale;
  SDL_Rect rect;

  restoreSettings(CONFIG_NAME, rect, fontScale);
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *window = SDL_CreateWindow("SmallBASIC",
                                        rect.x, rect.y, rect.w, rect.h,
                                        SDL_WINDOW_SHOWN |
                                        SDL_WINDOW_RESIZABLE |
                                        SDL_WINDOW_INPUT_FOCUS |
                                        SDL_WINDOW_MOUSE_FOCUS);
  if (window != NULL) {
    String font, fontBold;
    if (getFontFiles(fontFamily, font, fontBold)) {
      loadIcon(window);
      Runtime *runtime = new Runtime(window);
      runtime->construct(font.c_str(), fontBold.c_str());
      fontScale = runtime->runShell(runFile, fontScale);
      delete runtime;
    } else {
      fprintf(stderr, "Failed to locate display font\n");
    }
    saveSettings(CONFIG_NAME, window, fontScale);
    SDL_DestroyWindow(window);
  } else {
    fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
  }

  SDL_Quit();
  free(fontFamily);
  free(runFile);
  logLeaving();
  return 0;
}
