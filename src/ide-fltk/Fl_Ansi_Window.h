// -*- c-file-style: "java" -*-
// $Id: Fl_Ansi_Window.h,v 1.8 2004-11-25 11:13:25 zeeb90au Exp $
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <fltk/Widget.H>
#include <fltk/draw.H>
#include <fltk/Image.h>
#include <fltk/Font.h>

#ifndef FL_ANSI_WINDOW
#define FL_ANSI_WINDOW

using namespace fltk;

class Fl_Ansi_Window : public Widget {
    public:
    Fl_Ansi_Window(int x, int y, int w, int h);
    virtual ~Fl_Ansi_Window();

    // inherited methods
    void draw();
    void layout();
    int handle(int e);

    // public api
    void clearScreen();
    void saveScreen();
    void restoreScreen();
    void print(const char *str);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawFGRectFilled(int x, int y, int width, int height);
    void drawBGRectFilled(int x, int y, int width, int height);
    void drawFGRect(int x, int y, int width, int height);
    void drawBGRect(int x, int y, int width, int height);
    void drawImage(Image* img, int x, int y, int sx, int sy, int w, int h);

    void setTextColor(long fg, long bg) {
        labelcolor(ansiToFltk(fg));
        color(ansiToFltk(bg));
    }
    void setColor(long color) {
        labelcolor(ansiToFltk(color));
    }
    int getX() {return curX;}
    int getY() {return curY;}
    void setPixel(int x, int y, int c);
    int getPixel(int x, int y);
    void setXY(int x, int y) {curX=x; curY=y;}
    int textWidth(const char* s);
    int textHeight(void);
    int getWidth()  {return w();}
    int getHeight() {return h();}

    private:
    void init();
    void initOffscreen();
    bool setGraphicsRendition(char c, int escValue);
    Color ansiToFltk(long color) const;
    bool doEscape(unsigned char *&p);
    int calcTab(int x) const;
    void newLine();
    void reset();

    Image* img;
    bool underline;
    bool invert;
    bool bold;
    bool italic;
    int curY;
    int curX;
    int curYSaved;
    int curXSaved;
    int tabSize;
};

#endif
