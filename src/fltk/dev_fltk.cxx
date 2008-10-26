// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2005 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"
#include "device.h"
#include "smbas.h"
#include "osd.h"

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/SharedImage.h>
#include <fltk/FL_VERSION.h>
#include <fltk/Rectangle.h>
#include <fltk/damage.h>

#include "MainWindow.h"
#include "HelpWidget.h"

extern "C" {
#include "fs_socket_client.h"
}
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
#define PEN_ON  2
#define PEN_OFF 0

HelpWidget *formView = 0;
Properties env;
String envs;
String eventName;
bool saveForm = false;

void getHomeDir(char *fileName);
bool cacheLink(dev_file_t * df, char *localFile);
void updateForm(const char *s);
void closeForm();
void closeModeless();           // in blib_fltk_ui.cpp
void clearOutput();

//--ANSI Output-----------------------------------------------------------------

C_LINKAGE_BEGIN int osd_devinit()
{
  wnd->resetPen();
  os_graphics = 1;

  // allow the application to set the preferred width and height
  if ((opt_pref_width || opt_pref_height) && wnd->isIdeHidden()) {
    int delta_x = wnd->w() - wnd->out->w();
    int delta_y = wnd->h() - wnd->out->h();
    wnd->outputGroup->resize(opt_pref_width + delta_x, opt_pref_height + delta_y);
  } 

  // show the output-group in case it's the full-screen container. a possible
  // bug with fltk on x11 prevents resize after the window has been shown
  if (wnd->isInteractive()) {
    wnd->outputGroup->show();
  }

  os_graf_mx = wnd->out->w();
  os_graf_my = wnd->out->h();

  os_ver = FL_MAJOR_VERSION + FL_MINOR_VERSION + FL_PATCH_VERSION;
  os_color = 1;
  os_color_depth = 16;
  setsysvar_str(SYSVAR_OSNAME, "FLTK");
  if (SharedImage::first_image) {
    SharedImage::first_image->clear_cache();
  }
  if (saveForm == false) {
    closeForm();
    osd_cls();
  }
  saveForm = false;
  dev_clrkb();
  closeModeless();
  return 1;
}

void osd_setcolor(long color)
{
  wnd->out->setColor(color);
}

void osd_settextcolor(long fg, long bg)
{
  wnd->out->setTextColor(fg, bg);
}

void osd_refresh()
{
  wnd->out->redraw();
}

int osd_devrestore()
{
  closeModeless();
  return 1;
}

/**
 *   system event-loop
 *   return value:
 *     0 continue 
 *    -1 close sbpad application
 *    -2 stop running basic application
 */
int osd_events(int wait_flag)
{
  if ((wait_flag && wnd->isTurbo == false) ||
      (wnd->penState == 0 && wnd->penMode == PEN_ON && fltk::ready() == false)) {
    // in a "while 1" loop checking for a pen/mouse
    // event with pen(0) or executing input statement.
    fltk::wait();
  }

  fltk::check();
  if (wnd->isBreakExec()) {
    clearOutput();
    return -2;
  }
  return 0;
}

void osd_setpenmode(int enable)
{
  wnd->penMode = (enable ? PEN_ON : PEN_OFF);
}

void get_mouse_xy()
{
  fltk::get_mouse(wnd->penDownX, wnd->penDownY);
  // convert mouse screen rect to out-client rect
  wnd->penDownX -= wnd->x() + wnd->out->x();
  wnd->penDownY -= wnd->y() + wnd->out->y();
  if (!wnd->isIdeHidden()) {
    wnd->penDownY -= wnd->tabGroup->y() + wnd->outputGroup->y();
  }
}

int osd_getpen(int code)
{
  if (wnd->isBreakExec()) {
    clearOutput();
    brun_break();
    return 0;
  }

  if (wnd->penMode == PEN_OFF) {
    fltk::wait();
  }

  switch (code) {
  case 0:     // return true if there is a waiting pen event (up/down)
    if (wnd->penState != 0) {
      wnd->penState = 0;
      get_mouse_xy();
      fltk::Rectangle * rc = wnd->out;
      if (rc->contains(wnd->penDownX, wnd->penDownY)) {
        return 1;
      }
    }
    fltk::wait();               // UNTIL PEN(0)
                                // fallthru to re-test 

  case 3:    // returns true if the pen is down (and save curpos)
    if (event_state() & ANY_BUTTON) {
      get_mouse_xy();
      fltk::Rectangle * rc = wnd->out;
      if (rc->contains(wnd->penDownX, wnd->penDownY)) {
        return 1;
      }
    }
    return 0;

  case 1:                      // last pen-down x
    return wnd->penDownX;

  case 2:                      // last pen-down y
    return wnd->penDownY;

  case 4:                      // cur pen-down x
  case 10:
    get_mouse_xy();
    return wnd->penDownX;

  case 5:                      // cur pen-down y
  case 11:
    get_mouse_xy();
    return wnd->penDownY;

  case 12:                     // true if left button pressed
    return (event_state() & BUTTON1);

  case 13:                     // true if right button pressed
    return (event_state() & BUTTON3);

  case 14:                     // true if middle button pressed
    return (event_state() & BUTTON2);
  }
  return 0;
}

int osd_getx()
{
  return wnd->out->getX();
}

int osd_gety()
{
  return wnd->out->getY();
}

void osd_setxy(int x, int y)
{
  wnd->out->setXY(x, y);
}

void osd_cls()
{
  wnd->out->clearScreen();
}

int osd_textwidth(const char *str)
{
  return (int)wnd->out->textWidth(str);
}

int osd_textheight(const char *str)
{
  return wnd->out->textHeight();
}

void osd_setpixel(int x, int y)
{
  wnd->out->setPixel(x, y, dev_fgcolor);
}

long osd_getpixel(int x, int y)
{
  return wnd->out->getPixel(x, y);
}

void osd_line(int x1, int y1, int x2, int y2)
{
  wnd->out->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill)
{
  if (bFill) {
    wnd->out->drawRectFilled(x1, y1, x2, y2);
  }
  else {
    wnd->out->drawRect(x1, y1, x2, y2);
  }
}

void osd_beep()
{
  wnd->out->beep();
}

void osd_sound(int frq, int ms, int vol, int bgplay)
{
#ifdef WIN32
  if (!bgplay) {
    ::Beep(frq, ms);
  }
#endif // WIN32
}

void osd_clear_sound_queue()
{
}

void osd_write(const char *s)
{
  wnd->out->print(s);
}

//--ENV-------------------------------------------------------------------------

int dev_putenv(const char *s)
{
  if (formView && formView->setInputValue(s)) {
    return 1;                   // updated form variable
  }

  envs.empty();
  envs.append(s);
  String lv = envs.lvalue();
  String rv = envs.rvalue();

  if (lv.equals("TITLE")) {
    rv.append(" - SmallBASIC");
    wnd->copy_label(rv);
  }
  else if (lv.equals("INDENT_LEVEL") && wnd->getEditor()) {
    wnd->getEditor()->setIndentLevel(rv.toInteger());
  }
  else if (lv.equals("TURBO")) {
    wnd->isTurbo = rv.toInteger() == 1 ? 1 : 0;
  }
  env.put(lv, rv);
  return 1;
}

char *dev_getenv(const char *s)
{
  if (formView) {
    char *var = (char *)(formView->getInputValue(formView->getInput(s)));
    if (var) {
      return var;
    }
  }
  String *str = env.get(s);
  return str ? (char *)str->toString() : getenv(s);
}

char *dev_getenv_n(int n)
{
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

int dev_env_count()
{
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

void doEvent(void *)
{
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
  }
  else if (wnd->siteHome.length() == 0) {
    // not currently visiting a remote site
    if (wnd->getEditor() && 
        wnd->getEditor()->checkSave(true) == false) {
      return;
    }
  }
  wnd->execLink(eventName.toString());
}

void modeless_cb(Widget * w, void *v)
{
  if (wnd->isEdit()) {
    // create a full url path from the given relative path
    const String & path = formView->getEventName();
    eventName.empty();
    if (path[0] != '!' &&
        path[0] != '|' &&
        path.startsWith("http://") == false && wnd->siteHome.length() > 0) {
      int i = wnd->siteHome.indexOf('/', 7);  // siteHome root
      if (path[0] == '/' && i != -1) {
        // add to absolute path from http://hostname/
        eventName.append(wnd->siteHome.substring(0, i));
      }
      else {
        // append path to siteHome
        eventName.append(wnd->siteHome);
      }
      if (eventName[eventName.length() - 1] != '/') {
        eventName.append("/");
      }
      eventName.append(path[0] == '/' ? path.substring(1) : path);
    }
    else {
      eventName.append(path);
    }

    fltk::add_check(doEvent);   // post message
  }
}

void modal_cb(Widget * w, void *v)
{
  fltk::exit_modal();
  dev_putenv(((HelpWidget *) w)->getEventName());
}

void dev_html(const char *html, const char *t, int x, int y, int w, int h)
{
  if (html == 0 || html[0] == 0) {
    closeForm();
  }
  else if (t && t[0]) {
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
  }
  else {
    // fit within output window
    if (x < wnd->out->x()) {
      x = wnd->out->x();
    }
    if (y < wnd->out->y()) {
      y = wnd->out->y();
    }
    int wmax = wnd->out->x() + wnd->out->w() - x;
    int hmax = wnd->out->y() + wnd->out->h() - y;
    if (w > wmax || w == 0) {
      w = wmax;
    }
    if (h > hmax || h == 0) {
      h = hmax;
    }
    closeForm();
    wnd->outputGroup->begin();
    formView = new HelpWidget(x, y, w, h);
    wnd->outputGroup->end();
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

Image *getImage(dev_file_t * filep, int index)
{
  // check for cached imaged
  SharedImage *image = loadImage(filep->name, 0);
  // if (image && image->drawn()) {
  // return image; // already loaded+drawn
  // }

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
    // force SharedImage::_draw() to call image->read()
    image->draw(fltk::Rectangle(0, 0));
  }
  return image;
}

void dev_image(int handle, int index, int x, int y, int sx, int sy, int w, int h)
{
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
      wnd->out->drawImage(img, x, y, sx, sy,
                          (w == 0 ? imgw : w), (h == 0 ? imgh : h));
    }
  }
  else {
    // output screen area image to jpeg
    wnd->out->saveImage(filep->name, x, y, sx, sy);
  }
}

int dev_image_width(int handle, int index)
{
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

int dev_image_height(int handle, int index)
{
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

void timeout_callback(void* data) {
  if (wnd->isModal()) {
    wnd->setModal(false);
  }
}

void dev_delay(dword ms) {
  if (!wnd->isBreakExec()) {
    add_timeout(((float)ms)/1000, timeout_callback, 0);
    wnd->setModal(true);
    while (wnd->isModal()) {
      fltk::wait(0.1);
    }
    fltk::remove_check(timeout_callback);
  }
}

//--INPUT-----------------------------------------------------------------------

void enter_cb(Widget *, void *v)
{
  wnd->setModal(false);
}

char *dev_gets(char *dest, int size)
{
  wnd->tabGroup->selected_child(wnd->outputGroup);
  wnd->outputGroup->begin();

  LineInput *in = new LineInput(wnd->out->getX() + 2,
                                wnd->out->getY() + 1,
                                20, wnd->out->textHeight() + 4);
  wnd->outputGroup->end();
  in->callback(enter_cb);
  in->reserve(size);
  in->textfont(wnd->out->labelfont());
  in->textsize(wnd->out->labelsize());

  wnd->setModal(true);

  while (wnd->isModal()) {
    fltk::wait();
  }

  if (wnd->isBreakExec()) {
    clearOutput();
    brun_break();
  }

  wnd->outputGroup->remove(in);
  int len = in->size() < size ? in->size() : size;
  strncpy(dest, in->value(), len);
  dest[len] = 0;
  delete in;

  // reposition x to adjust for input box
  wnd->out->setXY(wnd->out->getX() + 4, wnd->out->getY());
  wnd->out->print(dest);

  if (formView) {
    formView->redraw();
  }

  return dest;
}

C_LINKAGE_END

//--HTML Utils------------------------------------------------------------------

void getHomeDir(char *fileName)
{
  const char *home = getenv("HOME");
  if (home == 0) {
    home = getenv("TMP");
  }
  if (home == 0) {
    home = getenv("TEMP");
  }
  if (home == 0) {
    home = getenv("TMPDIR");
  }
  sprintf(fileName, "%s/.smallbasic/", home);
  makedir(fileName);
}

void closeForm()
{
  if (formView != 0) {
    formView->parent()->remove(formView);
    formView->parent(0);
    delete formView;
    formView = 0;
  }
  wnd->out->redraw();
}

void clearOutput()
{
  closeForm();
  closeModeless();
}

bool isFormActive()
{
  return formView != null;
}

// copy the url into the local cache
bool cacheLink(dev_file_t * df, char *localFile)
{
  char rxbuff[1024];
  FILE *fp;
  const char *url = df->name;
  const char *pathBegin = strchr(url + 7, '/');
  const char *pathEnd = strrchr(url + 7, '/');
  const char *pathNext;
  bool inHeader = true;
  bool httpOK = false;

  getHomeDir(localFile);
  strcat(localFile, "cache/");
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
    } while (pathBegin < pathEnd && ++level < 20);
  }
  if (pathEnd == 0 || pathEnd[1] == 0 || pathEnd[1] == '?') {
    strcat(localFile, "index.html");
  }
  else {
    strcat(localFile, pathEnd + 1);
  }

  fp = fopen(localFile, "w");
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
          fwrite(rxbuff + i + 3, bytes - i - 3, 1, fp);
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
    }
    else {
      fwrite(rxbuff, bytes, 1, fp);
    }
  }

  // cleanup
  fclose(fp);
  shutdown(df->handle, df->handle);
  return httpOK;
}

// redisplay the help widget and associated images
void updateForm(const char *s)
{
  if (formView) {
    formView->loadBuffer(s);
    formView->show();
    formView->take_focus();
  }
  else {
    dev_html(s, 0, 0, 0, 0, 0);
  }

//     List images;
//     char localFile[PATH_MAX];
//     dev_file_t df;
//     bool newContent = false;
//     formView->getImageNames(&images);
//     int len = images.length();
//     if (len == 0) {
//         return;
//     }
//     memset(&df, 0, sizeof(dev_file_t));
//     const char* host = wnd->siteHome.toString();
//     const char* hostRoot = strchr(host+7, '/');
//     int pathLen = hostRoot ? hostRoot-host : strlen(host);
//     Object** list = images.getList();

//     for (int i=0; i<len; i++) {
//         String* s = (String*)list[i];
//         eventName.empty();
//         if ((*s)[0] == '/') {
//             // append abs image path to root of host path
//             eventName.append(host, pathLen);
//         } else {
//             // append relative image path to host path
//             eventName.append(host);
//             eventName.append("/");
//         }
//         eventName.append(s);
//         strcpy(df.name, eventName);
//         df.handle = -1;
//         if (cacheLink(&df, localFile)) {
//             newContent = true;
//         }
//         shutdown(df.handle, df.handle);
//     }
//     if (newContent) {
//         formView->reloadImages();
//     }
}

// End of "$Id$".
