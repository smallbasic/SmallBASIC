// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <jni.h>
#include <string.h>

#include "common/sbapp.h"
#include "common/device.h"
#include "common/osd.h"
#include "common/str.h"

// TODO:
// - write well formed HTML and markup for font, colours etc
// - pass web args to command$
// - html5 versions of RECT and LINE

cstr g_buffer;

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_WebServer_init
  (JNIEnv *env, jclass jclazz) {
  opt_command[0] = '\0';
  opt_file_permitted = 0;
  opt_graphics = 0;
  opt_ide = 0;
  opt_modlist[0] = '\0';
  opt_nosave = 1;
  opt_pref_bpp = 0;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  os_color_depth = 1;
  os_graf_mx = 80;
  os_graf_my = 25;
}

extern "C" JNIEXPORT jstring JNICALL Java_net_sourceforge_smallbasic_WebServer_execute
  (JNIEnv *env, jclass jclazz, jstring str) {
  cstr_init(&g_buffer, 1024);
  const char *fileName = env->GetStringUTFChars(str, NULL);
  sbasic_main(fileName);
  env->ReleaseStringUTFChars(str, fileName);
  return env->NewStringUTF(g_buffer.buf);
}

void osd_write(const char *str) {
  int len = strlen(str);
  if (len) {
    int i, index = 0, escape = 0;
    char *buffer = (char *)malloc(len + 1);
    for (i = 0; i < len; i++) {
      if (i + 1 < len && str[i] == '\033' && str[i + 1] == '[') {
        escape = 1;
      } else if (escape && str[i] == 'm') {
        escape = 0;
      } else if (!escape) {
        buffer[index++] = str[i];
      }
    }
    if (index) {
      buffer[index] = 0;
      cstr_append(&g_buffer, buffer);
    }
    free(buffer);
  }
}

int osd_textwidth(const char *str) {
  return strlen(str);
}

int osd_devinit() { return 1;}
int osd_devrestore() { return 1; }
int osd_events(int wait_flag) { return 0; }
int osd_getpen(int code) {return 0;}
int osd_getx() { return 0; }
int osd_gety() { return 0; }
int osd_textheight(const char *str) { return 1; }
long osd_getpixel(int x, int y) {}
void osd_beep() {}
void osd_clear_sound_queue() {}
void osd_cls() {}
void osd_line(int x1, int y1, int x2, int y2) {}
void osd_rect(int x1, int y1, int x2, int y2, int fill) {}
void osd_refresh() {}
void osd_setcolor(long color) {}
void osd_setpenmode(int enable) {}
void osd_setpixel(int x, int y) {}
void osd_settextcolor(long fg, long bg) {}
void osd_setxy(int x, int y) {}
void osd_sound(int frq, int ms, int vol, int bgplay) {}
