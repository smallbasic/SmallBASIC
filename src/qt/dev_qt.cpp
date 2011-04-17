// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "config.h"
#include "sys.h"
#include "device.h"
#include "smbas.h"
#include "osd.h"
#include "blib_ui.h"
#include "mainwindow.h"
#include "form_ui.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QLineEdit>
#include <QMap>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtGlobal>

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

#define PEN_OFF   0 // pen mode disabled
#define PEN_ON    2 // pen mode active

clock_t lastEventTime;
dword eventsPerTick;

QMap<QString, QString> env;

#define EVT_MAX_BURN_TIME (CLOCKS_PER_SEC / 4)
#define EVT_PAUSE_TIME 5 // MS
#define EVT_CHECK_EVERY ((50 * CLOCKS_PER_SEC) / 1000)

void getHomeDir(char *fileName, bool appendSlash=true);
bool cacheLink(dev_file_t * df, char *localFile);

//--ANSI Output-----------------------------------------------------------------

C_LINKAGE_BEGIN 

int osd_devinit() {
  wnd->out->resetMouse();

  // allow the application to set the preferred width and height
  if ((opt_pref_width || opt_pref_height)) {
    int delta_x = wnd->width() - wnd->out->width();
    int delta_y = wnd->height() - wnd->out->height();
    if (opt_pref_width < 10) {
      opt_pref_width = 10;
    }
    if (opt_pref_height < 10) {
      opt_pref_height = 10;
    }
    //    wnd->outputGroup->resize(opt_pref_width + delta_x,
    //                       opt_pref_height + delta_y);
  } 

  os_graf_mx = wnd->out->width();
  os_graf_my = wnd->out->height();

  os_ver = QT_VERSION;
  os_color = 1;
  os_color_depth = 16;
  setsysvar_str(SYSVAR_OSNAME, "QT");

  osd_cls();
  dev_clrkb();
  ui_reset();
  return 1;
}

void osd_setcolor(long color) {
  wnd->out->setColor(color);
}

void osd_settextcolor(long fg, long bg) {
  wnd->out->setTextColor(fg, bg);
}

void osd_refresh() {
  wnd->out->update();
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
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
    break;
  case 2:
    // pause
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, EVT_PAUSE_TIME);
    break;
  default:
    // pump messages without pausing
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }

  if (wnd->isBreakExec()) {
    ui_reset();
    return -2;
  }
  return 0;
}

void osd_setpenmode(int enable) {
  wnd->out->setMouseMode(enable ? PEN_ON : PEN_OFF);
}

int osd_getpen(int code) {
  if (wnd->isBreakExec()) {
    ui_reset();
    brun_break();
    return 0;
  }

  if (wnd->out->getMouseMode() == PEN_OFF) {
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
  }

  switch (code) {
  case 0:
    // UNTIL PEN(0) - wait until click or move
    if (form_event()) {
      // clicked a form widget
      return 1;
    }

    // flush any waiting update/paint events to allow mouse events to fire
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // receive any mouse events in wnd->out
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);

    // fallthru to re-test 

  case 3:  // returns true if the pen is down (and save curpos)
    return (Qt::LeftButton == QApplication::mouseButtons());

  case 1:  // last down x
    return wnd->out->getMouseX(true);

  case 2:  // last down y
    return wnd->out->getMouseY(true);

  case 4:  // cur x
  case 10:
    return wnd->out->getMouseX(false);

  case 5:  // cur y
  case 11:
    return wnd->out->getMouseY(false);

  case 12: // true if left button pressed
    return (Qt::LeftButton == QApplication::mouseButtons());

  case 13: // true if right button pressed
    return (Qt::RightButton == QApplication::mouseButtons());

  case 14: // true if middle button pressed
    return (Qt::MiddleButton == QApplication::mouseButtons());
  }
  return 0;
}

int osd_getx() {
  return wnd->out->getX();
}

int osd_gety() {
  return wnd->out->getY();
}

void osd_setxy(int x, int y) {
  wnd->out->setXY(x, y);
}

void osd_cls() {
  // send reset and clear screen codes
  if (opt_interactive) {
    wnd->out->print("\033[0m\xC");
  }
}

int osd_textwidth(const char *str) {
  return (int) wnd->out->textWidth(str);
}

int osd_textheight(const char *str) {
  return wnd->out->textHeight();
}

void osd_setpixel(int x, int y) {
  wnd->out->setPixel(x, y, dev_fgcolor);
}

long osd_getpixel(int x, int y) {
  return wnd->out->getPixel(x, y);
}

void osd_line(int x1, int y1, int x2, int y2) {
  wnd->out->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
  if (bFill) {
    wnd->out->drawRectFilled(x1, y1, x2, y2);
  }
  else {
    wnd->out->drawRect(x1, y1, x2, y2);
  }
}

void dev_arc(int xc, int yc, double r, double start, double end, double aspect) {
  wnd->out->drawArc(xc, yc, r, start, end, aspect);
}

void dev_ellipse(int xc, int yc, int xr, int yr, double aspect, int fill) {
  wnd->out->drawEllipse(xc, yc, xr, yr, aspect, fill);
}

void osd_beep() {
  wnd->out->beep();
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
}

void osd_clear_sound_queue() {
}

void osd_write(const char* s) {
  wnd->out->print(s);
}

void lwrite(const char* s) {
  wnd->logWrite(s);
}

/**
 * run a program (if retflg wait and return; otherwise just exec())
 */
int dev_run(const char *src, int retflg) {
  int result = 0;
  if (retflg) {
    result = QProcess::execute(src);
  }
  else {
    QProcess::startDetached(src);
  }
  return result;
}

//--ENV-------------------------------------------------------------------------

int dev_putenv(const char *s) {
  QStringList elems = QString::fromAscii(s).split("=");
  QString s1 = elems.at(0);
  QString s2 = elems.at(1);
  env.insert(s1, s2);
  return 1;
}

char* dev_getenv(const char *s) {
  QString str = env.value(s);
  return str.count() ? str.toUtf8().data() : getenv(s);
}

char* dev_getenv_n(int n) {
  int count = env.count();
  if (n < count) {
    // first n number of elements exist in the qmap
    QString e;
    e.append(env.keys().at(n));
    e.append("=");
    e.append(env.values().at(n));
    return e.toUtf8().data();
  }

  return 0;
}

int dev_env_count() {
  int count = env.count();
  return count;
}

//--IMAGE-----------------------------------------------------------------------

QImage *getImage(dev_file_t * filep, int index) {
  // check for cached imaged
  QImage *image = new QImage(filep->name);
  char localFile[PATH_MAX];

  // read image from web server
  switch (filep->type) {
  case ft_http_client:
    // open "http://localhost/image1.gif" as #1
    if (cacheLink(filep, localFile) == false) {
      return 0;
    }
    strcpy(filep->name, localFile);
    image = new QImage(filep->name);
    break;
  case ft_stream:
    // loaded in SharedImage
    break;
  default:
    return 0;
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
    QImage *img = getImage(filep, index);
    if (img != 0) {
      // input/read image and display
      imgw = img->width();
      imgh = img->height();
      wnd->out->drawImage(img, x, y, sx, sy,
                          (w == 0 ? imgw : w), (h == 0 ? imgh : h));
    }
  }
  else {
    // output screen area image to jpeg
    wnd->out->saveImage(filep->name, x, y, sx, sy);
  }
}

int dev_image_width(int handle, int index) {
  int imgw = -1;
  dev_file_t *filep = dev_getfileptr(handle);
  if (filep == 0 || filep->open_flags != DEV_FILE_INPUT) {
    return 0;
  }

  QImage *img = getImage(filep, index);
  if (img) {
    imgw = img->width();
  }
  return imgw;
}

int dev_image_height(int handle, int index) {
  int imgh = -1;
  dev_file_t *filep = dev_getfileptr(handle);
  if (filep == 0 || filep->open_flags != DEV_FILE_INPUT) {
    return 0;
  }

  QImage *img = getImage(filep, index);
  if (img) {
    imgh = img->height();
  }
  return imgh;
}

//--DELAY-----------------------------------------------------------------------

void dev_delay(dword ms) {
  if (!wnd->isBreakExec()) {
    wnd->setModal(true);
    QTimer::singleShot(ms, wnd, SLOT(endModal()));

    while (wnd->isModal()) {
      QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);
    }
  }
}

//--INPUT-----------------------------------------------------------------------

char *dev_gets(char *dest, int size) {
  QLineEdit *in = new QLineEdit(wnd->out);

  in->setGeometry(wnd->out->getX() + 2,
                  wnd->out->getY() + 1, 20, 
                  wnd->out->textHeight() + 4);
  in->setFont(wnd->out->font());
  in->connect(in, SIGNAL(returnPressed()), wnd, SLOT(endModal()));
  wnd->setModal(true);

  while (wnd->isModal()) {
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 100);
  }

  if (wnd->isBreakExec()) {
    ui_reset();    
    brun_break();
  }

  QString text = in->text();
  int len = text.length() < size ? text.length() : size;
  strncpy(dest, text.toUtf8().data(), len);
  dest[len] = 0;
  delete in;

  // reposition x to adjust for input box
  wnd->out->setXY(wnd->out->getX() + 4, wnd->out->getY());
  wnd->out->print(dest);

  return dest;
}

C_LINKAGE_END

//--HTML Utils------------------------------------------------------------------

void getHomeDir(char *fileName, bool appendSlash) {
  const char* vars[] = {
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

// copy the url into the local cache
bool cacheLink(dev_file_t * df, char *localFile) {
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
  }
  else {
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

// End of "$Id$".
