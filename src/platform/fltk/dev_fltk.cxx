// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "common/sys.h"
#include "common/device.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/blib_ui.h"

#include <fltk/ask.h>
#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/SharedImage.h>
#include <fltk/FL_VERSION.h>
#include <fltk/Rectangle.h>
#include <fltk/damage.h>

#include "MainWindow.h"
#include "HelpWidget.h"
#include "TtyWidget.h"

#include "common/fs_socket_client.h"

#ifdef WIN32
#include <windows.h>
#ifdef __MINGW32__
#include <winsock.h>
#endif
#else
#include <sys/socket.h>
#endif

#if defined(__MINGW32__)
#define makedir(f) mkdir(f)
#else
#define makedir(f) mkdir(f, 0700)
#endif

#define PEN_OFF   0             // pen mode disabled
#define PEN_ON    2             // pen mode active

HelpWidget *formView = 0;
Properties env;
String envs;
String eventName;
bool saveForm = false;
clock_t lastEventTime;
dword eventsPerTick;

#define EVT_MAX_BURN_TIME (CLOCKS_PER_SEC / 4)
#define EVT_PAUSE_TIME 0.005
#define EVT_CHECK_EVERY ((50 * CLOCKS_PER_SEC) / 1000)

void getHomeDir(char *fileName, bool appendSlash = true);
bool cacheLink(dev_file_t *df, char *localFile);
void updateForm(const char *s);
void closeForm();
void clearOutput();

// in blib_fltk_ui.cxx
bool form_event();

//--ANSI Output-----------------------------------------------------------------

C_LINKAGE_BEGIN

int osd_devinit() {
  wnd->resetPen();
  os_graphics = 1;

  // allow the application to set the preferred width and height
  if ((opt_pref_width || opt_pref_height) && wnd->isIdeHidden()) {
    int delta_x = wnd->w() - wnd->_out->w();
    int delta_y = wnd->h() - wnd->_out->h();
    if (opt_pref_width < 10) {
      opt_pref_width = 10;
    }
    if (opt_pref_height < 10) {
      opt_pref_height = 10;
    }
    wnd->_outputGroup->resize(opt_pref_width + delta_x, opt_pref_height + delta_y);
  }
  // show the output-group in case it's the full-screen container. a possible
  // bug with fltk on x11 prevents resize after the window has been shown
  if (wnd->isInteractive() && !wnd->logPrint()) {
    wnd->_outputGroup->show();
  }

  os_graf_mx = wnd->_out->w();
  os_graf_my = wnd->_out->h();

  os_ver = FL_MAJOR_VERSION + FL_MINOR_VERSION + FL_PATCH_VERSION;
  os_color = 1;
  os_color_depth = 16;
  setsysvar_str(SYSVAR_OSNAME, "FLTK");
  if (SharedImage::first_image) {
    SharedImage::first_image->clear_cache();
  }
  if (saveForm == false) {
    closeForm();
  }
  osd_cls();
  saveForm = false;
  dev_clrkb();
  ui_reset();
  return 1;
}

void osd_setcolor(long color) {
  wnd->_out->setColor(color);
}

void osd_settextcolor(long fg, long bg) {
  wnd->_out->setTextColor(fg, bg);
}

void osd_refresh() {
  wnd->_out->redraw();
}

int osd_devrestore() {
  ui_reset();
  return 1;
}

/**
 * system event-loop
 * return value:
 *   0 continue 
 *  -1 close sbpad application
 *  -2 stop running basic application
 */
int osd_events(int wait_flag) {
  if (!wait_flag) {
    // pause when we have been called too frequently
    clock_t now = clock();
    if (now - lastEventTime <= EVT_CHECK_EVERY) {
      eventsPerTick += (now - lastEventTime);
      if (eventsPerTick >= EVT_MAX_BURN_TIME) {
        eventsPerTick = 0;
        wait_flag = 2;
      }
    }
    lastEventTime = now;
  }

  switch (wait_flag) {
  case 1:
    // wait for an event
    fltk::wait();
    break;
  case 2:
    // pause
    fltk::wait(EVT_PAUSE_TIME);
    break;
  default:
    // pump messages without pausing
    fltk::check();
  }

  if (wnd->isBreakExec()) {
    clearOutput();
    return -2;
  }
  return 0;
}

void osd_setpenmode(int enable) {
  wnd->_penMode = (enable ? PEN_ON : PEN_OFF);
}

/**
 * sets the current mouse position and returns whether the mouse is within the output window
 */
bool get_mouse_xy() {
  fltk::Rectangle rc;
  int x, y;

  fltk::get_mouse(x, y);
  wnd->_out->get_absolute_rect(&rc);

  // convert mouse screen rect to out-client rect
  wnd->_penDownX = x - rc.x();
  wnd->_penDownY = y - rc.y();

  return rc.contains(x, y);
}

int osd_getpen(int code) {
  if (wnd->isBreakExec()) {
    clearOutput();
    brun_break();
    return 0;
  }

  if (wnd->_penMode == PEN_OFF) {
    fltk::wait();
  }

  switch (code) {
  case 0:
    // UNTIL PEN(0) - wait until move click or move
    if (form_event()) {
      // clicked a form widget
      get_mouse_xy();
      return 1;
    }
    fltk::wait();               // fallthru to re-test 

  case 3:                      // returns true if the pen is down (and save curpos)
    if (event_state() & ANY_BUTTON) {
      if (get_mouse_xy()) {
        return 1;
      }
    }
    return 0;

  case 1:                      // last pen-down x
    return wnd->_penDownX;

  case 2:                      // last pen-down y
    return wnd->_penDownY;

  case 4:                      // cur pen-down x
  case 10:
    get_mouse_xy();
    return wnd->_penDownX;

  case 5:                      // cur pen-down y
  case 11:
    get_mouse_xy();
    return wnd->_penDownY;

  case 12:                     // true if left button pressed
    return (event_state() & BUTTON1);

  case 13:                     // true if right button pressed
    return (event_state() & BUTTON3);

  case 14:                     // true if middle button pressed
    return (event_state() & BUTTON2);
  }
  return 0;
}

int osd_getx() {
  return wnd->_out->getX();
}

int osd_gety() {
  return wnd->_out->getY();
}

void osd_setxy(int x, int y) {
  wnd->_out->setXY(x, y);
}

void osd_cls() {
  // send reset and clear screen codes
  if (opt_interactive) {
    wnd->_out->print("\033[0m\xC");
    TtyWidget *tty = wnd->tty();
    if (tty) {
      tty->clearScreen();
    }
  }
}

int osd_textwidth(const char *str) {
  return (int)fltk::getwidth(str);
}

int osd_textheight(const char *str) {
  return wnd->_out->textHeight();
}

void osd_setpixel(int x, int y) {
  wnd->_out->setPixel(x, y, dev_fgcolor);
}

long osd_getpixel(int x, int y) {
  int xoffs = 0;
  int yoffs = 0;

#if !defined(WIN32)
  // offset x/y as getPixel is relative to the outer window
  fltk::Rectangle rc;
  wnd->_out->get_absolute_rect(&rc);
  xoffs = rc.x();
  yoffs = rc.y();

  wnd->get_absolute_rect(&rc);
  xoffs -= rc.x();
  yoffs -= rc.y();
#endif
  return wnd->_out->getPixel(x + xoffs, y + yoffs);
}

void osd_line(int x1, int y1, int x2, int y2) {
  wnd->_out->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
  if (bFill) {
    wnd->_out->drawRectFilled(x1, y1, x2, y2);
  } else {
    wnd->_out->drawRect(x1, y1, x2, y2);
  }
}

void osd_beep() {
  fltk::beep(fltk::BEEP_MESSAGE);
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
#ifdef WIN32
  if (!bgplay) {
    ::Beep(frq, ms);
  }
#endif // WIN32
}

void osd_clear_sound_queue() {
}

void osd_write(const char *s) {
  if (wnd->tty() && wnd->logPrint()) {
    wnd->tty()->print(s);
  }
  wnd->_out->print(s);
}

void lwrite(const char *s) {
  TtyWidget *tty = wnd->tty();
  if (tty) {
    tty->print(s);
  }
}

//--ENV-------------------------------------------------------------------------

int dev_putenv(const char *s) {
  if (formView && formView->setInputValue(s)) {
    return 1;                   // updated form variable
  }

  envs.empty();
  envs.append(s);

  String lv = envs.lvalue();
  String rv = envs.rvalue();

  env.put(lv, rv);
  return 1;
}

char *dev_getenv(const char *s) {
  if (formView) {
    char *var = (char *)(formView->getInputValue(formView->getInput(s)));
    if (var) {
      return var;
    }
  }
  String *str = env.get(s);
  return str ? (char *)str->toString() : getenv(s);
}

char *dev_getenv_n(int n) {
  if (formView) {
    return (char *)(formView->getInputValue(n));
  }

  int count = env.length();
  if (n < count) {
    envs.empty();
    envs.append(env.getKey(n));
    envs.append("=");
    envs.append(env.get(n));
    return (char *)envs.toString();
  }

  while (environ[count]) {
    if (n == count) {
      return environ[count];
    }
    count++;
  }
  return 0;
}

int dev_env_count() {
  if (formView) {
    Properties p;
    formView->getInputProperties(&p);
    return p.length();
  }
  int count = env.length();
  while (environ[count]) {
    count++;
  }
  return count;
}

//--HTML------------------------------------------------------------------------

void doEvent(void *) {
  fltk::remove_check(doEvent);
  if (eventName[0] == '|') {
    // user flag to indicate UI should remain
    // for next program execution
    const char *filename = eventName.toString();
    int len = strlen(filename);
    if (strcasecmp(filename + len - 4, ".htm") == 0 || 
        strcasecmp(filename + len - 5, ".html") == 0) {
      // "execute" a html file
      formView->loadFile(filename + 1, true);
      return;
    }
    saveForm = true;
  } else if (wnd->_siteHome.length() == 0) {
    // not currently visiting a remote site
    if (wnd->getEditor() && wnd->getEditor()->checkSave(true) == false) {
      return;
    }
  }
  wnd->execLink(eventName);
}

void modeless_cb(Widget *w, void *v) {
  if (wnd->isEdit()) {
    // create a full url path from the given relative path
    const String & path = formView->getEventName();
    eventName.empty();
    if (path[0] != '!' && path[0] != '|' && path.startsWith("http://") == false && 
        wnd->_siteHome.length() > 0) {
      int i = wnd->_siteHome.indexOf('/', 7);    // siteHome root
      if (path[0] == '/' && i != -1) {
        // add to absolute path from http://hostname/
        eventName.append(wnd->_siteHome.substring(0, i));
      } else {
        // append path to siteHome
        eventName.append(wnd->_siteHome);
      }
      if (eventName[eventName.length() - 1] != '/') {
        eventName.append("/");
      }
      eventName.append(path[0] == '/' ? path.substring(1) : path);
    } else {
      eventName.append(path);
    }

    fltk::add_check(doEvent);   // post message
  }
}

void modal_cb(Widget *w, void *v) {
  fltk::exit_modal();
  dev_putenv(((HelpWidget *)w)->getEventName());
}

void dev_html(const char *html, const char *t, int x, int y, int w, int h) {
  if (html == 0 || html[0] == 0) {
    closeForm();
  } else if (strncmp(html, "file:///", 8) == 0 || 
             strncmp(html, "http://", 7) == 0) {
    browseFile(html);
  } else if (t && t[0]) {
    // offset from main window
    x += wnd->x();
    y += wnd->y();
    Group::current(0);
    Window window(x, y, w, h, t);
    window.begin();
    HelpWidget out(0, 0, w, h);
    out.loadBuffer(html);
    out.callback(modal_cb);
    window.resizable(&out);
    window.end();
    window.exec(wnd);
    out.getInputProperties(&env);
  } else {
    // fit within output window
    if (x < wnd->_out->x()) {
      x = wnd->_out->x();
    }
    if (y < wnd->_out->y()) {
      y = wnd->_out->y();
    }
    int wmax = wnd->_out->x() + wnd->_out->w() - x;
    int hmax = wnd->_out->y() + wnd->_out->h() - y;
    if (w > wmax || w == 0) {
      w = wmax;
    }
    if (h > hmax || h == 0) {
      h = hmax;
    }
    closeForm();
    wnd->_outputGroup->begin();
    formView = new HelpWidget(x, y, w, h);
    wnd->_outputGroup->end();
    formView->callback(modeless_cb);
    formView->loadBuffer(html);
    formView->show();
    formView->take_focus();

    // update the window title using the html <title> tag contents
    const char *s = formView->getTitle();
    if (s && s[0]) {
      String title;
      title.append(s);
      title.append(" - SmallBASIC");
      wnd->copy_label(title);
    }
  }
}

//--IMAGE-----------------------------------------------------------------------

Image *getImage(dev_file_t *filep, int index) {
  // check for cached imaged
  SharedImage *image = loadImage(filep->name, 0);
  char localFile[PATH_MAX];

  // read image from web server
  switch (filep->type) {
  case ft_http_client:
    // open "http://localhost/image1.gif" as #1
    if (cacheLink(filep, localFile) == false) {
      return 0;
    }
    strcpy(filep->name, localFile);
    image = loadImage(filep->name, 0);
    break;
  case ft_stream:
    // loaded in SharedImage
    break;
  default:
    return 0;
  }

  if (image) {
    image->fetch_if_needed();
  }

  return image;
}

void dev_image(int handle, int index, int x, int y, int sx, int sy, int w, int h) {
  int imgw = -1;
  int imgh = -1;
  dev_file_t *filep = dev_getfileptr(handle);
  if (filep == 0) {
    return;
  }

  if (filep->open_flags == DEV_FILE_INPUT) {
    Image *img = getImage(filep, index);
    if (img != 0) {
      // input/read image and display
      img->measure(imgw, imgh);
      wnd->_out->drawImage(img, x, y, sx, sy, (w == 0 ? imgw : w), (h == 0 ? imgh : h));
    }
  } else {
    // output screen area image to jpeg
    wnd->_out->saveImage(filep->name, x, y, sx, sy);
  }
}

int dev_image_width(int handle, int index) {
  int imgw = -1;
  int imgh = -1;
  dev_file_t *filep = dev_getfileptr(handle);
  if (filep == 0 || filep->open_flags != DEV_FILE_INPUT) {
    return 0;
  }

  Image *img = getImage(filep, index);
  if (img) {
    img->measure(imgw, imgh);
  }
  return imgw;
}

int dev_image_height(int handle, int index) {
  int imgw = -1;
  int imgh = -1;
  dev_file_t *filep = dev_getfileptr(handle);
  if (filep == 0 || filep->open_flags != DEV_FILE_INPUT) {
    return 0;
  }

  Image *img = getImage(filep, index);
  if (img) {
    img->measure(imgw, imgh);
  }
  return imgh;
}

//--DELAY-----------------------------------------------------------------------

void timeout_callback(void *data) {
  if (wnd->isModal()) {
    wnd->setModal(false);
  }
}

void dev_delay(dword ms) {
  if (!wnd->isBreakExec()) {
    add_timeout(((float)ms) / 1000, timeout_callback, 0);
    wnd->setModal(true);
    while (wnd->isModal()) {
      fltk::wait(0.1);
    }
  }
}

//--INPUT-----------------------------------------------------------------------

void enter_cb(Widget *, void *v) {
  wnd->setModal(false);
}

char *dev_gets(char *dest, int size) {
  if (!wnd->isInteractive() || wnd->logPrint()) {
    EditorWidget *editor = wnd->_runEditWidget;
    if (!editor) {
      editor = wnd->getEditor(false);
    }
    if (editor) {
      editor->getInput(dest, size);
    }
    return dest;
  }

  wnd->_tabGroup->selected_child(wnd->_outputGroup);
  wnd->_outputGroup->begin();

  LineInput *in = new LineInput(wnd->_out->getX() + 2,
                                wnd->_out->getY() + 1,
                                20, wnd->_out->textHeight() + 4);
  wnd->_outputGroup->end();
  in->callback(enter_cb);
  in->reserve(size);
  in->textfont(wnd->_out->labelfont());
  in->textsize(wnd->_out->labelsize());

  wnd->setModal(true);

  while (wnd->isModal()) {
    fltk::wait();
  }

  if (wnd->isBreakExec()) {
    clearOutput();
    brun_break();
  }

  wnd->_outputGroup->remove(in);
  int len = in->size() < size ? in->size() : size;
  strncpy(dest, in->value(), len);
  dest[len] = 0;
  delete in;

  // reposition x to adjust for input box
  wnd->_out->setXY(wnd->_out->getX() + 4, wnd->_out->getY());
  wnd->_out->print(dest);

  if (formView) {
    formView->redraw();
  }

  return dest;
}

C_LINKAGE_END

//--HTML Utils------------------------------------------------------------------
void getHomeDir(char *fileName, bool appendSlash) {
  const char *vars[] = {
    "APPDATA", "HOME", "TMP", "TEMP", "TMPDIR"
  };

  int vars_len = sizeof(vars) / sizeof(vars[0]);

  fileName[0] = 0;

  for (int i = 0; i < vars_len; i++) {
    const char *home = getenv(vars[i]);
    if (home && access(home, R_OK) == 0) {
      strcpy(fileName, home);
      if (i == 1) {
        // unix path
        strcat(fileName, "/.config");
        makedir(fileName);
      }
      strcat(fileName, "/SmallBASIC");
      if (appendSlash) {
        strcat(fileName, "/");
      }
      makedir(fileName);
      break;
    }
  }
}

void closeForm() {
  if (formView != 0) {
    formView->parent()->remove(formView);
    formView->parent(0);
    delete formView;
    formView = 0;
  }
  wnd->_out->redraw();
}

void clearOutput() {
  closeForm();
  ui_reset();
}

bool isFormActive() {
  return formView != null;
}

// copy the url into the local cache
bool cacheLink(dev_file_t *df, char *localFile) {
  char rxbuff[1024];
  FILE *fp;
  const char *url = df->name;
  const char *pathBegin = strchr(url + 7, '/');
  const char *pathEnd = strrchr(url + 7, '/');
  const char *pathNext;
  bool inHeader = true;
  bool httpOK = false;

  getHomeDir(localFile);
  strcat(localFile, "/cache/");
  makedir(localFile);

  // create host name component
  strncat(localFile, url + 7, pathBegin - url - 7);
  strcat(localFile, "/");
  makedir(localFile);
  if (formView) {
    formView->setDocHome(localFile);
  }

  if (pathBegin != 0 && pathBegin < pathEnd) {
    // re-create the server path in cache
    int level = 0;
    pathBegin++;
    do {
      pathNext = strchr(pathBegin, '/');
      strncat(localFile, pathBegin, pathNext - pathBegin + 1);
      makedir(localFile);
      pathBegin = pathNext + 1;
    }
    while (pathBegin < pathEnd && ++level < 20);
  }
  if (pathEnd == 0 || pathEnd[1] == 0 || pathEnd[1] == '?') {
    strcat(localFile, "index.html");
  } else {
    strcat(localFile, pathEnd + 1);
  }

  fp = fopen(localFile, "wb");
  if (fp == 0) {
    if (df->handle != -1) {
      shutdown(df->handle, df->handle);
    }
    return false;
  }

  if (df->handle == -1) {
    // pass the cache file modified time to the HTTP server
    struct stat st;
    if (stat(localFile, &st) == 0) {
      df->drv_dw[2] = st.st_mtime;
    }
    if (http_open(df) == 0) {
      return false;
    }
  }
  // TODO: move this to a separate thread
  while (true) {
    int bytes = recv(df->handle, (char *)rxbuff, sizeof(rxbuff), 0);
    if (bytes == 0) {
      break;                    // no more data
    }
    // assumes http header < 1024 bytes
    if (inHeader) {
      int i = 0;
      while (true) {
        int iattr = i;
        while (rxbuff[i] != 0 && rxbuff[i] != '\n') {
          i++;
        }
        if (rxbuff[i] == 0) {
          inHeader = false;
          break;                // no end delimiter
        }
        if (rxbuff[i + 2] == '\n') {
          if (!fwrite(rxbuff + i + 3, bytes - i - 3, 1, fp)) {
            break;
          }
          inHeader = false;
          break;                // found start of content
        }
        // null terminate attribute (in \r)
        rxbuff[i - 1] = 0;
        i++;
        if (strstr(rxbuff + iattr, "200 OK") != 0) {
          httpOK = true;
        }
//                 if (strncmp(rxbuff+iattr, "Last-Modified: ", 15) == 0) {
//                     // Last-Modified: Tue, 29 Jul 2003 20:19:10 GMT 
//                     if (access(localFile, 0) == 0) {
//                         fclose(fp);
//                         shutdown(df->handle, df->handle);
//                         return true;
//                     }
//                 }
        if (strncmp(rxbuff + iattr, "Location: ", 10) == 0) {
          // handle redirection
          shutdown(df->handle, df->handle);
          strcpy(df->name, rxbuff + iattr + 10);
          if (http_open(df) == 0) {
            fclose(fp);
            return false;
          }
          break;                // scan next header
        }
      }
    } else {
      if (!fwrite(rxbuff, bytes, 1, fp)) {
        break;
      }
    }
  }

  // cleanup
  fclose(fp);
  shutdown(df->handle, df->handle);
  return httpOK;
}

// redisplay the help widget and associated images
void updateForm(const char *s) {
  if (formView) {
    formView->loadBuffer(s);
    formView->show();
    formView->take_focus();
  } else {
    dev_html(s, 0, 0, 0, 0, 0);
  }
}
