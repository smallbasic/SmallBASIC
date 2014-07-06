// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <SDL.h>
#include <fontconfig/fontconfig.h>

#include "platform/common/utils.h"
#include "platform/common/StringLib.h"
#include "platform/sdl/runtime.h"

using namespace strlib;

const char* FONTS[] = {
  "Ubuntu Mono",
  "DejaVu Sans Mono",
  "FreeMono",
  "Liberation Mono",
  "TlwgMono",
  "Courier New",
  NULL
};

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
  fprintf(stderr, buf, 0);
}

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

int main(int argc, char* argv[]) {
  logEntered();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *window = SDL_CreateWindow("SmallBASIC",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        640, 480,
                                        SDL_WINDOW_SHOWN | 
                                        SDL_WINDOW_INPUT_FOCUS |
                                        SDL_WINDOW_MOUSE_FOCUS);
  if (window == NULL) {
    trace("Could not create window: %s\n", SDL_GetError());
  } else {
    String font, fontBold;
    if (!getFontFiles(NULL, font, fontBold)) {
      trace("Failed to locate display font\n");
    }
    else {
      Runtime *runtime = new Runtime(window);
      runtime->construct(font.c_str(), fontBold.c_str());
      runtime->runShell();
      delete runtime;
    }
    // cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
  }
  logLeaving();
  return 0;
}
