// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>

#include <microhttpd.h>
#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "include/osd.h"
#include "common/sbapp.h"
#include "common/device.h"
#include "platform/web/canvas.h"
#include "platform/web/proxy.h"

static Canvas g_canvas;
static uint32_t g_start = 0;
static uint32_t g_maxTime = 2000;
static bool g_graphicText = true;
static bool g_noExecute = false;
static bool g_json = false;
static char *execBas = nullptr;
static MHD_Connection *g_connection;
static StringList g_cookies;
static String g_path;
static String g_data;

static struct option OPTIONS[] = {
  {"file-permitted", no_argument,       nullptr, 'f'},
  {"help",           no_argument,       nullptr, 'h'},
  {"json-content",   no_argument,       nullptr, 'j'},
  {"no-execute",     no_argument,       nullptr, 'x'},
  {"verbose",        no_argument,       nullptr, 'v'},
  {"command",        optional_argument, nullptr, 'c'},
  {"exec-bas",       optional_argument, nullptr, 'i'},
  {"graphic-text",   optional_argument, nullptr, 'g'},
  {"height",         optional_argument, nullptr, 'e'},
  {"max-time",       optional_argument, nullptr, 't'},
  {"module",         optional_argument, nullptr, 'm'},
  {"port",           optional_argument, nullptr, 'p'},
  {"run",            optional_argument, nullptr, 'r'},
  {"width",          optional_argument, nullptr, 'w'},
  {"proxy-path",     optional_argument, nullptr, 'a'},
  {"proxy-host",     optional_argument, nullptr, 'o'},
  {0, 0, 0, 0}
};

void init() {
  opt_command[0] = '\0';
  opt_file_permitted = 0;
  opt_graphics = 1;
  opt_ide = 0;
  opt_modpath[0] = '\0';
  opt_nosave = 1;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_autolocal = 0;
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
  while (OPTIONS[i].name != nullptr) {
    fprintf(stdout, OPTIONS[i].has_arg ?
            "  -%c, --%s='<argument>'\n" : "  -%c, --%s\n",
            OPTIONS[i].val, OPTIONS[i].name);
    i++;
  }
  fprintf(stdout, "\nhttps://smallbasic.github.io\n\n");
}

void log(const char *format, ...) {
  va_list args;
  va_start(args, format);
  unsigned size = format == nullptr ? 0 : vsnprintf(nullptr, 0, format, args);
  va_end(args);

  if (size) {
    char *buf = (char *)malloc(size + 1);
    buf[0] = '\0';
    va_start(args, format);
    vsnprintf(buf, size + 1, format, args);
    va_end(args);
    buf[size] = '\0';

    char date[18];
    time_t t = time(nullptr);
    struct tm *p = localtime(&t);
    strftime(date, sizeof(date), "%Y%m%d %H:%M:%S", p);
    fprintf(stdout, "%s %s\n", date, buf);
    free(buf);
  }
}

// allow or deny access
MHD_Result accept_cb(void *cls, const struct sockaddr *addr, socklen_t addrlen) {
  return MHD_YES;
}

MHD_Response *execute(MHD_Connection *connection, const char *bas) {
  const char *width = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "width");
  const char *height = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "height");
  const char *command = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "command");
  const char *graphicText = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "graphic-text");
  const char *accept = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT);
  const char *contentType = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_TYPE);

  if (width != nullptr) {
    os_graf_mx = atoi(width);
  }
  if (height != nullptr) {
    os_graf_my = atoi(height);
  }
  if (graphicText != nullptr) {
    g_graphicText = atoi(graphicText) > 0;
  }
  if (command != nullptr) {
    strlcpy(opt_command, command, sizeof(opt_command));
  }

  log("%s dim:%dX%d [accept=%s, content-type=%s]", bas, os_graf_mx, os_graf_my, accept, contentType);
  g_connection = connection;
  g_canvas.reset();
  g_start = dev_get_millisecond_count();
  g_canvas.setGraphicText(g_graphicText);
  g_canvas.setJSON(g_json || (accept && strncmp(accept, "application/json", 16) == 0));
  g_cookies.removeAll();
  sbasic_main(bas);
  g_connection = nullptr;
  String page = g_canvas.getPage();
  MHD_Response *response = MHD_create_response_from_buffer(page.length(), (void *)page.c_str(), MHD_RESPMEM_MUST_COPY);
  List_each(String *, it, g_cookies) {
    String *next = (*it);
    MHD_add_response_header(response, MHD_HTTP_HEADER_SET_COOKIE, next->c_str());
  }
  return response;
}

MHD_Response *serve_file(const char *path) {
  MHD_Response *response;
  struct stat stbuf;
  int fd = open(path, O_RDONLY | O_BINARY);
  if (!fstat(fd, &stbuf)) {
    response = MHD_create_response_from_fd(stbuf.st_size, fd);

    // add the content-type headers for browsing
    unsigned len = strlen(path);
    if (len > 4 && strcmp(path + len - 4, ".css") == 0) {
      MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/css");
    } else if (len > 3 && strcmp(path + len - 3, ".js") == 0) {
      MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/javascript");
    }
  } else {
    response = nullptr;
  }
  return response;
}

MHD_Response *get_response(MHD_Connection *connection, const char *path, const char *method) {
  MHD_Response *response = nullptr;
  struct stat stbuf;

  g_path = path;

  if (proxy_accept(connection, path)) {
    response = proxy_request(connection, path, method, g_data.c_str());
  } else if (execBas && stat(execBas, &stbuf) != -1 && S_ISREG(stbuf.st_mode)) {
    response = execute(connection, execBas);
  } else if (path[0] == '\0') {
    if (stat("index.bas", &stbuf) != -1 && S_ISREG(stbuf.st_mode)) {
      response = execute(connection, "index.bas");
    } else {
      response = serve_file("index.html");
    }
  } else if (strcmp(path, "favicon.ico") == 0) {
    response = serve_file(path);
  } else if (strstr(path, "..") == nullptr) {
    const char *dot = strrchr(path, '.');
    if (dot && !g_noExecute && strncasecmp(dot, ".bas", 4) == 0 &&
        stat(path, &stbuf) != -1 && S_ISREG(stbuf.st_mode)) {
      response = execute(connection, path);
    } else {
      response = serve_file(path);
    }
  }
  return response;
}

// server callback
// see: /usr/share/doc/libmicrohttpd-dev/examples
MHD_Result access_cb(void *cls,
                     MHD_Connection *connection,
                     const char *url,
                     const char *method,
                     const char *version,
                     const char *upload_data,
                     size_t *upload_data_size,
                     void **ptr) {
  static int dummy;
  if (&dummy != *ptr) {
    // The first time only the headers are valid,
    // do not respond in the first round
    *ptr = &dummy;
    return MHD_YES;
  }
  // clear context pointer
  *ptr = nullptr;

  if (upload_data != nullptr) {
    // curl -H "Accept: application/json" -d '{"productId": 123456, "quantity": 100}' http://localhost:8080/foo
    size_t size = OPT_CMD_SZ - 1;
    if (*upload_data_size < size) {
      size = *upload_data_size;
    }
    g_data.clear();
    g_data.append(upload_data, size);
    *upload_data_size = 0;
    return MHD_YES;
  }

  MHD_Result result;
  MHD_Response *response = get_response(connection, url + 1, method);
  if (response != nullptr) {
    int code = g_canvas.getPage().length() ? MHD_HTTP_OK : MHD_HTTP_NO_CONTENT;
    result = MHD_queue_response(connection, code, response);
  } else {
    String error;
    error.append("File not found: ").append(url);
    log(error.c_str());
    response = MHD_create_response_from_buffer(error.length(), (void *)error.c_str(), MHD_RESPMEM_MUST_COPY);
    result = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  }
  MHD_destroy_response(response);
  return result;
}

int main(int argc, char **argv) {
  init();
  int port = 8080;
  char *runBas = nullptr;
  char *proxyPath = nullptr;
  char *proxyHost = nullptr;

  while (1) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "hvfxjp:t:m::r:w:e:c:g:i:a:o:", OPTIONS, &option_index);
    if (c == -1) {
      break;
    }
    if (OPTIONS[option_index].has_arg && optarg[0] == '\0') {
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
      strlcpy(opt_command, optarg, sizeof(opt_command));
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
      if (optarg) {
        strlcpy(opt_modpath, optarg, sizeof(opt_modpath));
      }
      break;
    case 'i':
      if (optarg) {
        execBas = strdup(optarg);
      }
      break;
    case 'x':
      g_noExecute = true;
      break;
    case 'j':
      g_json = true;
      break;
    case 'a':
      if (optarg) {
        proxyPath = strdup(optarg);
      }
      break;
    case 'o':
      if (optarg) {
        proxyHost = strdup(optarg);
      }
      break;
    default:
      show_help();
      exit(1);
      break;
    }
  }

  proxy_init(proxyPath, proxyHost);

  if (runBas != nullptr) {
    g_canvas.reset();
    g_start = dev_get_millisecond_count();
    sbasic_main(runBas);
    puts(g_canvas.getPage().c_str());
  } else {
    fprintf(stdout, "Starting SmallBASIC web server on port:%d. Press return to exit.\n", port);
    MHD_Daemon *d = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port,
                                     &accept_cb, nullptr,
                                     &access_cb, nullptr, MHD_OPTION_END);
    if (d == nullptr) {
      fprintf(stderr, "startup failed\n");
      return 1;
    }

    while(getc(stdin) != '\n') {
      usleep(10000); // On Raspberry Pi OS getc returns immediately EOF if in background.
                     // Sleep to reduce cpu usage.
    }

    MHD_stop_daemon(d);
  }

  proxy_cleanup();
  free(execBas);
  free(proxyPath);
  free(proxyHost);
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
  return nullptr;
}

void lwrite(const char *buf) {
  if (buf[0] != '\0' && buf[0] != '\r' && buf[0] != '\n') {
    log("%s %s", "LOGPRINT:", buf);
  }
}

//
// Handle ENV using cookies, HTTP headers and GET arguments
//
struct ValueIteratorClosure {
  ValueIteratorClosure(int el) :
    _result(nullptr),
    _count(0),
    _element(el) {
  }
  const char *_result;
  int _count;
  int _element;
};

MHD_Result valueIterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  ValueIteratorClosure *closure = (ValueIteratorClosure *)cls;
  MHD_Result result;
  if (closure->_count++ == closure->_element) {
    closure->_result = value;
    result = MHD_NO;
  } else {
    result = MHD_YES;
  }
  return result;
}

MHD_Result countIterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
  return MHD_YES;
}

int dev_setenv(const char *key, const char *value) {
  String cookie;
  cookie.append(key).append("=").append(value);
  g_cookies.add(cookie);
  return 0;
}

const char *dev_getenv(const char *key) {
  const char *result;
  if (strcmp(key, "path") == 0) {
    result = g_path.c_str();
  } else if (strcmp(key, "data") == 0) {
    result = g_data.c_str();
  } else if (g_connection != nullptr) {
    result = MHD_lookup_connection_value(g_connection, MHD_COOKIE_KIND, key);
    if (result == nullptr) {
      result = MHD_lookup_connection_value(g_connection, MHD_HEADER_KIND, key);
    }
    if (result == nullptr) {
      result = MHD_lookup_connection_value(g_connection, MHD_GET_ARGUMENT_KIND, key);
    }
  } else {
    result = nullptr;
  }
  return result;
}

int dev_env_count() {
  int count = 2;
  if (g_connection != nullptr) {
    count += MHD_get_connection_values(g_connection, MHD_COOKIE_KIND, countIterator, nullptr);
    count += MHD_get_connection_values(g_connection, MHD_HEADER_KIND, countIterator, nullptr);
    count += MHD_get_connection_values(g_connection, MHD_GET_ARGUMENT_KIND, countIterator, nullptr);
  }
  return count;
}

const char *dev_getenv_n(int n) {
  const char *result = nullptr;
  if (g_connection != nullptr) {
    ValueIteratorClosure closure(n);
    MHD_get_connection_values(g_connection, MHD_COOKIE_KIND, valueIterator, &closure);
    MHD_get_connection_values(g_connection, MHD_HEADER_KIND, valueIterator, &closure);
    MHD_get_connection_values(g_connection, MHD_GET_ARGUMENT_KIND, valueIterator, &closure);
    result = closure._result;
  }
  return result;
}

void dev_delay(uint32_t ms) {
  usleep(1000 * ms);
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
void osd_audio(const char *path) {}
void osd_sound(int frq, int ms, int vol, int bgplay) {}
void osd_ellipse(int xc, int yc, int xr, int yr, int fill) {}
void osd_arc(int xc, int yc, double r, double as, double ae, double aspect) {}
void v_create_image(var_p_t var) {}
void v_create_form(var_p_t var) {}
void v_create_window(var_p_t var) {}
void dev_show_page() {}
void dev_log_stack(const char *keyword, int type, int line) {}
