/**
 * AnsiWindow version 1.0 Chris Warren-Smith 12/12/2001
 * Derived from SmallBASIC - http://smallbasic.sourceforge.net
 *
 *                  _.-_:\
 *                 /      \
 *                 \_.--*_/
 *                       v
 *
 * This program is distributed under the terms of the GPL v2.0 or later
 * Download the GNU Public License (GPL) from www.gnu.org
 * 
 */

#ifndef ANSI_WINDOW
#define ANSI_WINDOW

class AnsiWindow : public CWindow {
    public:
    AnsiWindow(U16 id);
    AnsiWindow(U16 id, int width, int height);
    virtual ~AnsiWindow();

    void clearScreen();
    void saveScreen();
    void restoreScreen();
    void write(const char *str);
    void setColor(long color) {
        fgColor = qbToEbmColor(color);
    }
    void setTextColor(long fg, long bg) {
        fgColor = qbToEbmColor(fg);
        bgColor = qbToEbmColor(bg);
    }
    int getX() const {return curX;}
    int getY() const {return curY;}
    void setXY(int x, int y) {curX=x; curY=y;}
    int textWidth(const char* s) const {
        return GUI_TextWidth(font, s, strlen(s));
    }
    int textHeight(void) const {
        return GUI_FontHeight(font);
    }
    void drawLine(S16 x1, S16 y1, S16 x2, S16 y2) {
        CWindow::DrawLine(x1, y1, x2, y2, fgColor);
    }
    void drawFGRectFilled(S16 x, S16 y, S16 width, S16 height) {
        CWindow::DrawRectFilled(x, y, width, height, fgColor);
    }
    void drawBGRectFilled(S16 x, S16 y, S16 width, S16 height) {
        CWindow::DrawRectFilled(x, y, width, height, bgColor);
    }
    void drawFGRect(S16 x, S16 y, S16 width, S16 height) {
        CWindow::DrawRect(x, y, width, height, fgColor);
    }
    void drawBGRect(S16 x, S16 y, S16 width, S16 height) {
        CWindow::DrawRect(x, y, width, height, bgColor);
    }

    protected:
    COLOR qbToEbmColor(long color) const;
    void setGraphicsRendition(char c, int escValue);
    bool doEscape(U8 *&p);
    void Draw() {}
    int getStyle() const;
    int calcTab(int x) const;
    void newLine();

    private:
    void init() {
        lcd = 0;
        curY = 0;
        curX = 0;
        tabSize = 40; // tab size in pixels (160/32 = 5)
        reset();
    }

    void reset() {
        size = 9;
        invert = false;
        italic = false;
        bold = false;
        underline = false;
        font = GUI_GetFont(size, CTRL_NORMAL);
        fontHeight = GUI_FontHeight(font);
        maxLine = GetHeight()/fontHeight;
        fgColor = qbToEbmColor(0); // black
        bgColor = qbToEbmColor(15); // white
        curYSaved = 0;
        curXSaved = 0;
    }

    COLOR fgColor;
    COLOR bgColor;
    int curY;
    int curX;
    int curYSaved;
    int curXSaved;
    int tabSize;
    int maxLine;
    int size;
    bool underline;
    bool invert;
    bool italic;
    bool bold;
    const FONT *font;
    int fontHeight;
    IMAGE *lcd;
    int lcdHeight;
    int lcdWidth;
};

#endif
