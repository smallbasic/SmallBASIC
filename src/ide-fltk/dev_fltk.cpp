// -*- c-file-style: "java" -*-
// $Id: dev_fltk.cpp,v 1.32 2005-04-01 23:09:47 zeeb90au Exp $
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
#include <fltk/Rectangle.h>

#include "MainWindow.h"
#include "HelpWidget.h"

#ifdef WIN32
#include <windows.h>
#endif

C_LINKAGE_BEGIN

#define PEN_ON  2
#define PEN_OFF 0

extern MainWindow *wnd;
HelpWidget* helpView = 0;
char* eventName = 0;
Properties env;
String envs;
void closeHelp();
bool activeForm = 0;

//--ANSI Output-----------------------------------------------------------------

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
    if (activeForm == 0) {
        closeHelp();
    }
    activeForm = 0;
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
    if (wnd->isBreakExec()) {
        closeHelp();
        return -2;
    }
    return 0;
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
            fltk::wait();
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
    wnd->out->beep();
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

//--HTML------------------------------------------------------------------------

int dev_putenv(const char *s) {
    if (helpView && helpView->setInputValue(s)) {
        return 1;
    }
    envs.empty();
    envs.append(s);
    env.put(envs.lvalue(), envs.rvalue());
    return 1;
}

char* dev_getenv(const char *s) {
    if (helpView) {
        char* var = (char*)(helpView->getInputValue(helpView->getInput(s)));
        if (var) {
            return var;
        }
    }
    String* str = env.get(s);
    return str ? (char*)str->toString() : getenv(s);
}

char* dev_getenv_n(int n) {
    if (helpView) {
        return (char*)(helpView->getInputValue(n));
    }
    
    int count = env.length();
    if (n < count) {
        envs.empty();
        envs.append(env.getKey(n));
        envs.append("=");
        envs.append(env.get(n));
        return (char*)envs.toString();
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
    if (helpView) {
        Properties p;
        helpView->getInputProperties(&p);
        return p.length();
    }
    int count = env.length();
    while (environ[count]) count++;
    return count;
}

void closeHelp() {
    if (helpView) {
        helpView->parent()->remove(helpView);
        helpView->parent(0);
        helpView->getInputProperties(&env);
        delete helpView;
        helpView = 0;
        wnd->out->redraw();
    }
}

void doEvent(void*) {
    fltk::remove_check(doEvent);
    if (eventName[0] == '|') {
        activeForm = true;
    } else {
        closeHelp();
    }
    wnd->execLink(eventName);
    free((void*)eventName);
    eventName = 0;
}

void modeless_cb(Widget* w, void* v) {
    if (wnd->isEdit()) {
        eventName = strdup(helpView->getEventName());
        fltk::add_check(doEvent); // post message
    }
}

void modal_cb(Widget* w, void* v) {
    fltk::exit_modal();
    dev_putenv(((HelpWidget*)w)->getEventName());
}

void dev_html(const char* html, const char* t, int x, int y, int w, int h) {
    if (html == 0 || html[0] == 0) {
        closeHelp();
    } else if (t && t[0]) {
        // offset from main window
        x += wnd->x();
        y += wnd->y();
        Group::current(0);
        Window window(x, y, w, h, t);
        window.begin();
        HelpWidget out(0, 0, w, h);
        if (strnicmp("file:", html, 5) == 0) {
            out.loadFile(html+5);
        } else {
            out.loadBuffer(html);
        }
        out.callback(modal_cb);
        window.resizable(&out);
        window.end();
        window.exec(wnd);
        out.getInputProperties(&env);
    } else {
        // fit within output window
        if (x < wnd->out->x()) {
            x = wnd->out->x();
        }
        if (y < wnd->out->y()) {
            y = wnd->out->y();
        }
        int wmax = wnd->out->x()+wnd->out->w()-x;
        int hmax = wnd->out->y()+wnd->out->h()-y;
        if (w > wmax || w == 0) {
            w = wmax;
        } 
        if (h > hmax || h == 0) {
            h = hmax;
        }
        closeHelp();
        wnd->outputGroup->begin();
        helpView = new HelpWidget(x, y, w, h);
        wnd->outputGroup->end();
        helpView->callback(modeless_cb);
        if (strnicmp("file:", html, 5) == 0) {
            helpView->loadFile(html+5);
        } else {
            helpView->loadBuffer(html);
        }
        helpView->show();
        helpView->take_focus();
    }
}

//--IMAGE-----------------------------------------------------------------------

Image* getImage(int handle, int index) {
    dev_file_t* filep = dev_getfileptr(handle);
    if (filep == 0) {
        return 0;
    }

    // check for cached imaged
    SharedImage* image = loadImage(filep->name, 0);
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
            tmp_free(buff);
            return 0;
        }
        break;
    case ft_stream:
        // loaded in SharedImage
        break;
    default:
        return 0;
    }

    image = loadImage(filep->name, buff);
    if (image) {
        // force SharedImage::_draw() to call image->read()
        image->draw(fltk::Rectangle(0,0),0,0);
    }

    if (filep->type == ft_http_client) {
        // cleanup
        tmp_free(buff);
    }

    return image;
}

void dev_image(int handle, int index, int x, int y, 
               int sx, int sy, int w, int h) {
    int imgw = -1;
    int imgh = -1;
    Image* img = getImage(handle, index);
    if (img != 0) {
        img->measure(imgw, imgh);
        wnd->out->drawImage(img, x, y, sx, sy, 
                            (w==0 ? imgw : w),
                            (h==0 ? imgh : h));
    }
}

int dev_image_width(int handle, int index) {
    int imgw = -1;
    int imgh = -1;
    Image* img = getImage(handle, index);
    if (img) {
        img->measure(imgw, imgh);
    }
    return imgw;
}

int dev_image_height(int handle, int index) {
    int imgw = -1;
    int imgh = -1;
    Image* img = getImage(handle, index);
    if (img) {
        img->measure(imgw, imgh);
    }
    return imgh;
}

//--INPUT-----------------------------------------------------------------------

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

    int handle(int event) {
        if (event == fltk::KEY &&
            (event_key_state(LeftCtrlKey) || 
             event_key_state(RightCtrlKey)) &&
            event_key() == 'b') {
            wnd->setBreak();
        }
        return fltk::Input::handle(event);
    }

    int orig_x, orig_y;
    int orig_w, orig_h;
    int def_w;
};

char *dev_gets(char *dest, int size) {
    wnd->tabGroup->selected_child(wnd->outputGroup);
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
        fltk::wait();
    }

    if (wnd->isBreakExec()) {
        closeHelp();
        brun_break();
    }

    wnd->outputGroup->remove(in);
    int len = in->size() < size ? in->size() : size;
    strncpy(dest, in->value(), len);
    dest[len] = 0;
    delete in;

    // reposition x to adjust for input box
    wnd->out->setXY(wnd->out->getX()+4, wnd->out->getY());
    wnd->out->print(dest);

    if (helpView) {
        helpView->redraw();
    }

    return dest;
}

C_LINKAGE_END

