// -*- c-file-style: "java" -*-
// $Id: Fl_Ansi_Window.h,v 1.20 2005-08-12 06:25:52 zeeb90au Exp $
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FL_ANSI_WINDOW
#define FL_ANSI_WINDOW

#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/Image.h>

using namespace fltk;

struct ScreenImage : public Image {
    ScreenImage(int w, int h) : Image(w,h) {}
    void update() {}
};

class AnsiWindow : public Widget {
    public:
    AnsiWindow(int x, int y, int w, int h, int defsize);
    virtual ~AnsiWindow();

    // inherited methods
    void draw();
    void layout();
    int handle(int e);

    // public api
    void clearScreen();
    void print(const char *str);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRectFilled(int x1, int y1, int x2, int y2);
    void drawRect(int x1, int y1, int x2, int y2);
    void drawImage(Image* img, int x, int y, int sx, int sy, int w, int h);
    void saveImage(const char* fn, int x, int y, int w, int h);
    void setTextColor(long fg, long bg);
    void setColor(long color);
    int getX() {return curX;}
    int getY() {return curY;}
    void setPixel(int x, int y, int c);
    int getPixel(int x, int y);
    void setXY(int x, int y) {curX=x; curY=y;}
    int textWidth(const char* s);
    int textHeight(void);
    int getWidth()  {return w();}
    int getHeight() {return h();}
    void fontSize(int i) {labelsize(i);}
    int fontSize() {return (int)labelsize();}
    void beep() const;

    private:
    void init();
    void destroyImage();
    void initImage();
    bool setGraphicsRendition(char c, int escValue);
    Color ansiToFltk(long color) const;
    bool doEscape(unsigned char *&p);
    int calcTab(int x) const;
    void newLine();
    void reset();

    ScreenImage* img;
    bool underline;
    bool invert;
    bool bold;
    bool italic;
    bool resized;
    int curY;
    int curX;
    int curYSaved;
    int curXSaved;
    int tabSize;
};

#endif
