// -*- c-file-style: "java" -*-
// $Id: dev_fltk.cpp,v 1.18 2004-12-14 22:26:04 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2003 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "sys.h"
#include "device.h"
#include "smbas.h"

#include <sys/socket.h>
#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/SharedImage.h>
#include <fltk/FL_VERSION.h>
#include <fltk/HelpView.h>

#include "MainWindow.h"

#ifdef WIN32
#include <windows.h>
#endif

C_LINKAGE_BEGIN

#define PEN_ON  2
#define PEN_OFF 0

extern MainWindow *wnd;
HelpView* helpView = 0;
const char* anchor = 0;

void closeHelp() {
    if (helpView) {
        helpView->parent()->remove(helpView);
        helpView->parent(0);
        delete helpView;
        helpView = 0;
        wnd->out->redraw();
    }
}

int osd_devinit() {
    wnd->resetPen();
    os_graphics = 1;
    os_graf_mx = wnd->out->w();
    os_graf_my = wnd->out->h();
    os_ver = FL_MAJOR_VERSION+FL_MINOR_VERSION+FL_PATCH_VERSION;
    os_color = 1;
    os_color_depth = 16;
    setsysvar_str(SYSVAR_OSNAME, "FLTK");
    if (SharedImage::first_image) {
        SharedImage::first_image->clear_cache();
    }
    closeHelp();
    return 1;
}

void osd_setcolor(long color) {
    wnd->out->setColor(color);
}

void osd_settextcolor(long fg, long bg) {
    wnd->out->setTextColor(fg, bg);
}

void osd_refresh() {
    wnd->out->redraw();
}

int osd_devrestore() {
    return 1;
}

/**
 *   system event-loop
 *   return value:
 *     0 continue 
 *    -1 close sbpad application
 *    -2 stop running basic application
 */
int osd_events(int wait_flag) {
    if ((wait_flag && wnd->isTurbo == false) ||
        (wnd->penState == PEN_ON && fltk::ready() == false)) {
        // in a "while 1" loop checking for a pen/mouse
        // event with pen(0) or executing input statement. 
        fltk::wait();
    }

    fltk::check();
    return wnd->isBreakExec() ? -2 : 0;
}

void osd_setpenmode(int enable) {
    wnd->penState = (enable ? PEN_ON : PEN_OFF);
}

void get_mouse_xy() {
    fltk::get_mouse(wnd->penDownX, wnd->penDownY);
    // convert mouse screen rect to out-client rect
    wnd->penDownX -= wnd->x() + wnd->out->x();
    wnd->penDownY -= wnd->y() + wnd->out->y();
    wnd->penDownY -= wnd->tabGroup->y() + wnd->outputGroup->y();
}

int osd_getpen(int code) {
    if (wnd->penState == PEN_OFF) {
        fltk::wait();
    }

    switch (code) {
    case 0: // return true if there is a waiting pen event
    case 3: // returns true if the pen is down (and save curpos)
        if (wnd->isTurbo == false) {
            wait();
        }
        if (event_state() & ANY_BUTTON) {
            get_mouse_xy();
            return 1;
        }
        return 0;

    case 1:  // last pen-down x
        return wnd->penDownX;

    case 2:  // last pen-down y
        return wnd->penDownY;

    case 4:  // cur pen-down x
        get_mouse_xy();
        return wnd->penDownX;

    case 5:  // cur pen-down y
        get_mouse_xy();
        return wnd->penDownY;

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
    wnd->out->setXY(x,y);
}

void osd_cls() {
    wnd->out->clearScreen();
}

int osd_textwidth(const char *str) {
    return (int)wnd->out->textWidth(str);
}

int osd_textheight(const char *str) {
    return wnd->out->textHeight();
}

void osd_setpixel(int x, int y) {
    wnd->out->setPixel(x, y, dev_fgcolor);
}

long osd_getpixel(int x, int y) {
    return wnd->out->getPixel(x,y);
}

void osd_line(int x1, int y1, int x2, int y2) {
    wnd->out->drawLine(x1, y1, x2, y2);
}

void osd_rect(int x1, int y1, int x2, int y2, int bFill) {
    if (bFill) {
        wnd->out->drawRectFilled(x1, y1, x2, y2);
    } else {
        wnd->out->drawRect(x1, y1, x2, y2);
    }
}

void osd_beep() {
#ifdef WIN32
   MessageBeep(MB_ICONASTERISK);
#elif defined(__APPLE__)
   SysBeep(30);
#else
   XBell(fl_display, 100);
#endif // WIN32
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
    wnd->out->print(s);
}

void doAnchor(void*) {
    wnd->execLink(anchor);
    free((void*)anchor);
    anchor = 0;
}

void anchor_cb(Widget* w, void* v) {
    if (wnd->isEdit()) {
        anchor = strdup((const char*)v);
        closeHelp();
        fltk::add_timeout(0.5, doAnchor); // post message
    }
}

void dev_html(const char* html, const char* t, int x, int y, int w, int h) {

    // fit within output window
    x += wnd->out->x();
    y += wnd->out->y();

    if (x+w > wnd->out->w()) {
        w = wnd->out->w()-x; 
    }
    if (y+h > wnd->out->h()) {
        h = wnd->out->h()-y; 
    }

    if (helpView) {
        if (!html || !html[0] || x<0 || y<0) {
            closeHelp();
            return;
        }
        helpView->x(x);
        helpView->y(y);
        helpView->w(w);
        helpView->h(h);
    } else {
        wnd->outputGroup->begin();
        helpView = new HelpView(x, y, w, h);
        wnd->outputGroup->end();
        helpView->box(SHADOW_BOX);
        helpView->callback(anchor_cb);
    }
    helpView->value(html);
    helpView->take_focus();
    helpView->show();
}

// image factory based on file extension
Image* loadImage(const char* name, uchar* buff) {
    int len = strlen(name);
    if (strcmpi(name+(len-4), ".jpg") == 0 ||
        strcmpi(name+(len-5), ".jpeg") == 0) {
        //image = jpegImage::get(filep->name);
    } else if (strcmpi(name+(len-4), ".gif") == 0) {
        return gifImage::get(name, buff);
    } else if (strcmpi(name+(len-4), ".png") == 0) {
        //image = pngImage::get(filep->name);
    } else if (strcmpi(name+(len-4), ".xpm") == 0) {
        return xpmFileImage::get(name, buff);
    }
    return 0;
}

Image* getImage(int handle, int index) {
    dev_file_t* filep = dev_getfileptr(handle);
    if (filep == 0) {
        return 0;
    }

    // check for cached imaged
    Image* image = loadImage(filep->name, 0);
    if (image && image->drawn()) {
        return image;
    }

    uchar* buff = 0;
    unsigned blockSize = 1024;
    unsigned size = blockSize;
    unsigned len = 0;

    // read image from web server
    switch (filep->type) {
    case ft_http_client:
        // open "http://localhost/image1.gif" as #1
        buff = (uchar*)tmp_alloc(size);
        while (true) {
            unsigned bytes = recv(filep->handle, (char*)buff+len, blockSize, 0);
            len += bytes;
            if (bytes == 0 || bytes < blockSize) {
                break; // no more data
            }
            size += blockSize;
            buff = (uchar*)tmp_realloc(buff, size);
        }
        if (strstr((const char*)buff, "<title>404")) {
            return 0;
        }
        break;
    case ft_stream:
        break;
    default:
        return 0;
    }

    image = loadImage(filep->name, buff);
    if (image) {
        // force SharedImage::_draw() to call image->read()
        image->draw(0,0,0,0,0,0);
    }

    return image;
}

void dev_image(int handle, int index, int x, int y, 
               int sx, int sy, int w, int h) {
    Image* img = getImage(handle, index);
    if (img != 0) {
        wnd->out->drawImage(img, x, y, sx, sy, 
                            (w==0 ? img->w() : w), 
                            (h==0 ? img->h() : h));
    }
}

int dev_image_width(int handle, int index) {
    Image* img = getImage(handle, index);
    return (img != 0 ? img->w() : -1);
}

int dev_image_height(int handle, int index) {
    Image* img = getImage(handle, index);
    return (img != 0 ? img->h() : -1);
}

void enter_cb(Widget*, void* v) {
    wnd->setModal(false);
}

struct LineInput : public fltk::Input {
    LineInput(int def_w) : 
        fltk::Input(wnd->out->getX()+2, 
                    wnd->out->getY()+1, 
                    def_w,
                    wnd->out->textHeight()+4) {
        this->def_w = def_w;
        this->orig_x = x();
        this->orig_y = y();
        this->orig_w = w();
        this->orig_h = h();
    }
    bool replace(int b, int e, const char* text, int ilen) {
        // grow the input box width
        if (ilen) {
            int strw = (int)getwidth(value())+def_w;
            if (strw > w()) {
                w(strw);
                orig_w = strw;
                redraw();
            }
        }
        return Input::replace(b, e, text, ilen);
    }
    void layout() {
        fltk::Input::layout();
        // veto the layout changes
        x(orig_x);
        y(orig_y);
        w(orig_w);
        h(orig_h);
    }

    int orig_x, orig_y;
    int orig_w, orig_h;
    int def_w;
};

char *dev_gets(char *dest, int size) {
    wnd->outputGroup->begin();
    LineInput* in = new LineInput(20);
    wnd->outputGroup->end();
    in->callback(enter_cb);
    in->when(WHEN_ENTER_KEY_ALWAYS);
    in->box(BORDER_BOX);
    in->color(color(220,220,220));
    in->take_focus();
    in->reserve(size);
    in->textfont(wnd->out->labelfont());
    in->textsize(wnd->out->labelsize());

    wnd->setModal(true);
    while (wnd->isModal()) {
        wait();
    }

    if (wnd->isBreakExec()) {
        brun_break();
    }

    wnd->outputGroup->remove(in);
    int len = in->size() < size ? in->size() : size;
    strncpy(dest, in->value(), len);
    dest[len] = 0;
    delete in;

    return dest;
}

C_LINKAGE_END

