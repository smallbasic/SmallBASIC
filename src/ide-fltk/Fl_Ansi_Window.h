// -*- c-file-style: "java" -*-
// $Id: Fl_Ansi_Window.h,v 1.1 2004-10-27 23:31:47 zeeb90au Exp $
// This file is part of EBjLib
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
/*                  _.-_:\
//                 /      \
//                 \_.--*_/
//                       v
*/
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FL_ANSI_WINDOW
#define FL_ANSI_WINDOW

class Fl_Ansi_Window : public Fl_Widget {
    public:
    Fl_Ansi_Window(int x, int y, int w, int h);
    virtual ~Fl_Ansi_Window();

    // inherited methods
    void draw();
    void resize(int x, int y, int width, int height);

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

    void setTextColor(long fg, long bg) {
        labelcolor(ansiToFltk(fg));
        color(ansiToFltk(bg));
    }
    void setColor(long color) {
        labelcolor(ansiToFltk(color));
    }
    int getX() {return curX;}
    int getY() {return curY;}
    void setXY(int x, int y) {curX=x; curY=y;}
    double textWidth(const char* s) {return fl_width(s);}
    int textHeight(void) {return fl_height();}
    int getWidth()  {return w();}
    int getHeight() {return h();}

    private:
    void init();
    bool setGraphicsRendition(char c, int escValue);
    Fl_Color ansiToFltk(long color) const;
    bool doEscape(unsigned char *&p);
    int calcTab(int x) const;
    void newLine();
    void reset();

    Fl_Offscreen img;
    bool underline;
    bool invert;
    int curY;
    int curX;
    int curYSaved;
    int curXSaved;
    int tabSize;
};

#endif
