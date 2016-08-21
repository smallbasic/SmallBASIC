// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>

#include <microhttpd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "common/sbapp.h"
#include "common/device.h"
#include "common/osd.h"
#include "platform/web/canvas.h"

Canvas g_canvas;
dword g_start = 0;
dword g_maxTime = 2000;
bool g_graphicText = true;

static struct option OPTIONS[] = {
  {"help",           no_argument,       NULL, 'h'},
  {"verbose",        no_argument,       NULL, 'v'},
  {"file-permitted", no_argument,       NULL, 'f'},
  {"port",           optional_argument, NULL, 'p'},
  {"run",            optional_argument, NULL, 'r'},
  {"width",          optional_argument, NULL, 'w'},
  {"height",         optional_argument, NULL, 'e'},
  {"command",        optional_argument, NULL, 'c'},
  {"graphic-text",   optional_argument, NULL, 'g'},
  {"max-time",       optional_argument, NULL, 't'},
  {"module",         optional_argument, NULL, 'm'},
  {0, 0, 0, 0}
};

void init() {
  opt_command[0] = '\0';
  opt_file_permitted = 0;
  opt_graphics = 1;
  opt_ide = 0;
  opt_modlist[0] = '\0';
  opt_nosave = 1;
  opt_pref_bpp = 0;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  os_graf_mx = 1024;
  os_graf_my = 768;
}

void show_help() {
  fprintf(stdout,
          "SmallBASIC version %s - kw:%d, pc:%d, fc:%d, ae:%d I=%d N=%d\n\n",
          SB_STR_VER, kwNULL, (kwNULLPROC - kwCLS) + 1,
          (kwNULLFUNC - kwASC) + 1, (int)(65536 / sizeof(var_t)),
          (int)sizeof(var_int_t), (int)sizeof(var_num_t));
  fprintf(stdout, "usage: sbasicw [options]...\n");
  int i = 0;
  while (OPTIONS[i].name != NULL) {
    fprintf(stdout, OPTIONS[i].has_arg ?
            "  -%c, --%s='<argument>'\n" : "  -%c, --%s\n",
            OPTIONS[i].val, OPTIONS[i].name);
    i++;
  }
  fprintf(stdout, "\nhttp://smallbasic.sourceforge.net\n\n");
}

// allow or deny access
int accept_cb(void *cls,
              const struct sockaddr *addr,
              socklen_t addrlen) {
  return MHD_YES;
}

MHD_Response *execute(struct MHD_Connection *connection, const char *bas) {
  const char *width =
    MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "width");
  const char *height =
    MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "height");
  const char *command =
    MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "command");
  const char *graphicText =
    MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "graphic-text");
  if (width != NULL) {
    os_graf_mx = atoi(width);
  }
  if (height != NULL) {
    os_graf_my = atoi(height);
  }
  if (graphicText != NULL) {
    g_graphicText = atoi(graphicText) > 0;
  }
  if (command != NULL) {
    strcpy(opt_command, command);
  }
  fprintf(stdout, "sbasicw: file[%s] cmd[%s] dim[%dX%d]\n", bas, opt_command, os_graf_mx, os_graf_my);
  g_canvas.reset();
  g_start = dev_get_millisecond_count();
  g_canvas.setGraphicText(g_graphicText);
  sbasic_main(bas);
  String page = g_canvas.getPage();
  return MHD_create_response_from_buffer(page.length(), (void *)page.c_str(),
                                         MHD_RESPMEM_MUST_COPY);
}

MHD_Response *get_response(struct MHD_Connection *connection, const char *path) {
  MHD_Response *response = NULL;
  struct stat stbuf;
  if (strcmp(path, "favicon.ico") == 0) {
    int fd = open("./images/sb4w.ico", O_RDONLY);
    if (!fstat(fd, &stbuf)) {
      response = MHD_create_response_from_fd(stbuf.st_size, fd);
    }
    close(fd);
  } else if (stat(path, &stbuf) != -1 && S_ISREG(stbuf.st_mode)) {
    response = execute(connection, path);
  }
  return response;
}

// server callback
// see: /usr/share/doc/libmicrohttpd-dev/examples
int access_cb(void *cls,
              struct MHD_Connection *connection,
              const char *url,
              const char *method,
              const char *version,
              const char *upload_data,
              size_t *upload_data_size,
              void **ptr) {
  static int dummy;
  if (strcmp(method, "GET") != 0) {
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

  int result;
  struct MHD_Response *response = get_response(connection, url + 1);
  if (response != NULL) {
    result = MHD_queue_response(connection, MHD_HTTP_OK, response);
  } else {
    String error;
    error.append("File not found: ").append(url);
    response = MHD_create_response_from_buffer(error.length(), (void *)error.c_str(),
                                               MHD_RESPMEM_MUST_COPY);
    result = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  }
  MHD_destroy_response(response);
  return result;
}

int main(int argc, char **argv) {
  init();
  int port = 8080;
  char *runBas = NULL;

  while (1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "hvfp:t:m:r:w:e:c:g:", OPTIONS, &option_index);
    if (c == -1) {
      break;
    }
    if (OPTIONS[option_index].has_arg && optarg == '\0') {
      show_help();
      exit(1);
    }
    switch (c) {
    case 'r':
      runBas = optarg;
      break;
    case 'w':
      os_graf_mx = atoi(optarg);
      break;
    case 'e':
      os_graf_my = atoi(optarg);
      break;
    case 'c':
      strcpy(opt_command, optarg);
      break;
    case 'g':
      g_graphicText = atoi(optarg) > 1;
      break;
    case 'v':
      opt_verbose = true;
      opt_quiet = false;
      break;
    case 'f':
      opt_file_permitted = true;
      break;
    case 'h':
      show_help();
      exit(1);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 't':
      g_maxTime = atoi(optarg);
      break;
    case 'm':
      opt_loadmod = 1;
      strcpy(opt_modlist, optarg);
      break;
    default:
      show_help();
      exit(1);
      break;
    }
  }

  if (runBas != NULL) {
    g_canvas.reset();
    g_start = dev_get_millisecond_count();
    sbasic_main(runBas);
    puts(g_canvas.getPage().c_str());
  } else {
    fprintf(stdout, "Starting SmallBASIC web server on port:%d\n", port);
    MHD_Daemon *d =
    MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, port,
                     &accept_cb, NULL,
                     &access_cb, NULL, MHD_OPTION_END);
    if (d == NULL) {
      fprintf(stderr, "startup failed\n");
      return 1;
    }
    getc(stdin);
    MHD_stop_daemon(d);
  }
  return 0;
}

//
// common device implementation
//
int osd_textwidth(const char *str) {
  return strlen(str);
}

void osd_line(int x1, int y1, int x2, int y2) {
  g_canvas.drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int fill) {
  if (fill) {
    g_canvas.drawRectFilled(x1, y1, x2, y2);
  } else {
    g_canvas.drawRect(x1, y1, x2, y2);
  }
}

void osd_setcolor(long color) {
  g_canvas.setColor(color);
}

void osd_settextcolor(long fg, long bg) {
  g_canvas.setTextColor(fg, bg);
}

void osd_setpixel(int x, int y) {
  g_canvas.setPixel(x, y, dev_fgcolor);
}

void osd_setxy(int x, int y) {
  g_canvas.setXY(x, y);
}

void osd_cls(void) {
  g_canvas.clearScreen();
}

int osd_events(int wait_flag) {
  int result;
  if (dev_get_millisecond_count() - g_start > g_maxTime) {
    result = -2;
  } else {
    result = 0;
  }
  return result;
}

void osd_write(const char *str) {
  if (strlen(str) > 0) {
    g_canvas.print(str);
  }
}

char *dev_gets(char *dest, int maxSize) {
  return NULL;
}

//
// not implemented
//
int osd_devinit() { return 1;}
int osd_devrestore() { return 1; }
int osd_getpen(int code) {return 0;}
int osd_getx() { return 0; }
int osd_gety() { return 0; }
int osd_textheight(const char *str) { return 1; }
long osd_getpixel(int x, int y) { return 0;}
void osd_beep() {}
void osd_clear_sound_queue() {}
void osd_refresh() {}
void osd_setpenmode(int enable) {}
void osd_sound(int frq, int ms, int vol, int bgplay) {}
void v_create_image(var_p_t var) {}
void v_create_form(var_p_t var) {}
void v_create_window(var_p_t var) {}
void dev_show_page() {}
void dev_delay(dword ms) {}
