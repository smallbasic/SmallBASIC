// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "common/sbapp.h"
#include "common/device.h"
#include "common/osd.h"
#include "common/str.h"

// TODO:
// - write well formed HTML and markup for font, colours etc
// - pass web args to command$
// fixed font
// /usr/share/doc/libmicrohttpd-dev/examples/

cstr g_buffer;

void init() {
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

// http://www.html5canvastutorials.com/tutorials/html5-canvas-rectangles/
void osd_line(int x1, int y1, int x2, int y2) {
}

// http://www.html5canvastutorials.com/tutorials/html5-canvas-lines/
void osd_rect(int x1, int y1, int x2, int y2, int fill) {

}

void osd_setcolor(long color) {

}

void osd_settextcolor(long fg, long bg) {

}

// allow or deny access
int accept_cb(void *cls, 
              const struct sockaddr *addr,
              socklen_t addrlen) {
  return MHD_YES;
}

// server callback
int access_cb(void *cls,
              struct MHD_Connection *connection,
              const char *url,
              const char *method,
              const char *version,
              const char *upload_data,
              size_t *upload_data_size,
              void **ptr) {
  static int dummy;
  struct MHD_Response *response;
  struct stat buf;
  int result;
  
  if (0 != strcmp(method, "GET")) {
    // unexpected method
    return MHD_NO;
  }
  if (&dummy != *ptr) {
    // The first time only the headers are valid,
    // do not respond in the first round
    *ptr = &dummy;
    return MHD_YES;
  }

  if (0 != *upload_data_size) {
    // cannot upload data in a GET
    return MHD_NO;
  }

  // clear context pointer
  *ptr = NULL;

  if (strcmp(url, "/favicon.ico") == 0) {
    int fd = open("./images/sb4w.ico", O_RDONLY);
    if (fstat(fd, &buf)) {
      close(fd);
      response = NULL;
    } else {
      response = MHD_create_response_from_fd(buf.st_size, fd);
    }
  } else {
    cstr_init(&g_buffer, 1024);
    if (stat(url + 1, &buf) == 0) {
      sbasic_main(url + 1);
    } else {
      fprintf(stderr, "File not found %s\n", url);
      cstr_append(&g_buffer, "File not found");
    }
    response = MHD_create_response_from_buffer(g_buffer.length, (void *)g_buffer.buf,
                                               MHD_RESPMEM_PERSISTENT);
  }
  if (response != NULL) {
    result = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
  } else {
    result = MHD_NO;
  }
  return result;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("%s PORT\n", argv[0]);
    return 1;
  }
  init();
  fprintf(stdout, "Starting SmallBASIC web server on port:%s\n", argv[1]);

  // MHD_http_unescape
  MHD_Daemon *d =
    MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, atoi(argv[1]),
                     &accept_cb, NULL,
                     &access_cb, NULL, MHD_OPTION_END);
  if (d == NULL) {
    return 1;
  }
  getc(stdin);
  MHD_stop_daemon(d);
  return 0;
}

// not implemented
int osd_devinit() { return 1;}
int osd_devrestore() { return 1; }
int osd_events(int wait_flag) { return 0; }
int osd_getpen(int code) {return 0;}
int osd_getx() { return 0; }
int osd_gety() { return 0; }
int osd_textheight(const char *str) { return 1; }
long osd_getpixel(int x, int y) { return 0;}
void osd_beep() {}
void osd_clear_sound_queue() {}
void osd_cls() {}
void osd_refresh() {}
void osd_setpenmode(int enable) {}
void osd_setpixel(int x, int y) {}
void osd_setxy(int x, int y) {}
void osd_sound(int frq, int ms, int vol, int bgplay) {}
