/**
 * HTMLWindow version 1.0 Chris Warren-Smith 12/12/2001
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

#ifndef HTML_WINDOW
#define HTML_WINDOW

////////////////////////////////////////////////////////////////////////////////
// class HTMLViewable

class HTMLViewable : public CViewable {
public:
    HTMLViewable(const char* str, int width, int height);
    virtual ~HTMLViewable() {}

    void home();
    void end();
    void pageUp();
    void lineUp();
    void pageDown();
    void lineDown();
    void loadPage(const char* str);
    void Draw();

    protected:
    int getStyle(bool bold, bool italic) const;
    void redraw();
    void parseAndPaint();
    void paint();
    int  paraRow(const char* text, int textLength, const FONT *font);
    void newLine(int& x, int& y, int lineHeight) {x=2; y+=lineHeight;}
    int getHeight();

    private:
    void init() {
        vScroll = 0;
        pageHeight = 0;
        maxLineHeight = 0;
    }

    int vScroll;
    int pageHeight;
    int maxLineHeight;
    int numChildren;
    List nodeList;
    const char* html;
    String titleText;
};

////////////////////////////////////////////////////////////////////////////////
// class HTMLWindow

class HTMLWindow : public CWindow {
    public:
    HTMLWindow(const char* s);
    HTMLWindow(const PKG *pkg);
    ~HTMLWindow() {}

    protected:
    const PKG *pkg;
    Properties anchorMap;
    HTMLViewable *htmlView;
    void setOrientation();
    void loadAnchors(const char* s) {anchorMap.load(s);}
    S32 MsgHandler(MSG_TYPE type, CViewable *object, S32 data);
    void Draw();
};

#endif
