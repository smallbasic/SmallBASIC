// -*- c-file-style: "java" -*-
// $Id: HelpWidget.h,v 1.6 2005-03-17 22:30:35 zeeb90au Exp $
//
// Copyright(C) 2001-2004 Chris Warren-Smith. Gawler, South Australia
// cwarrens@twpo.com.au
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef FL_HELP_WIDGET
#define FL_HELP_WIDGET

#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/Image.h>
#include <fltk/Group.h>
#include <fltk/Scrollbar.h>

#include "StringLib.h"

#define ID_MAIN_WND 1
#define ID_ANCHOR   2
#define ID_IANCHOR  3
#define ID_BUTTON   4
#define ID_TEXTBOX  5
#define ID_CHKBOX   6
#define ID_RADIO    7
#define ID_OPTION   8
#define ID_CTRL_BEGIN ID_ANCHOR
#define ID_CTRL_END   ID_OPTION

using namespace fltk;
using namespace strlib;

class HelpWidget : public Group {
public:
    HelpWidget(int x, int y, int width, int height);
    virtual ~HelpWidget();

    void loadPage(const char* str);
    void reloadPage();
    void navigateTo(const char* anchorName);
    void navigateTo(int vscroll);
    U8 find(const char* s, U8 matchCase);
    Widget* getInput(const char* name);
    const char* getInputValue(Widget* button);
    const char* getInputName(Widget* button);
    const char* getAnchor() {return anchor.toString();}
    void getInputProperties(Properties& p);
    void copyText(int begin, int end);
    void load(const char *f);

    protected:
    void compileHTML();

    // fltk methods
    void draw();
    void layout();
    int onMove(int event);
    int onPush(int event);
    int handle(int event);

    private:
    void init();
    void cleanup();

    Color background;
    Scrollbar* scrollbar;
    S16 vscroll;
    U16 scrollHeight;
    strlib::List nodeList;
    strlib::List namedInputs;
    strlib::List inputs;
    strlib::List anchors;
    strlib::String htmlStr;
    strlib::String anchor;
};

#ifdef FL_HELP_WIDGET_RESOURCES
// somewhere to keep this clutter

static char* dot_xpm[] = {
    "5 5 3 1",
    "   c None",
    ".  c #F4F4F4",
    "+  c #000000",
    ".+++.",
    "+++++",
    "+++++",
    "+++++",
    ".+++."};

static xpmImage dotImage(dot_xpm);

struct {
    const char* ent;
    int elen;
    char xlat;
} entityMap [] = {
    { "Aacute;", 8, 193},
    { "aacute;", 8, 225},
    { "Acirc;", 7, 194},
    { "acirc;", 7, 226},
    { "acute;", 7, 180},
    { "AElig;", 7, 198},
    { "aelig;", 7, 230},
    { "Agrave;", 8, 192},
    { "agrave;", 8, 224},
    { "amp;", 5, '&'},
    { "Aring;", 7, 197},
    { "aring;", 7, 229},
    { "Atilde;", 8, 195},
    { "atilde;", 8, 227},
    { "Auml;", 6, 196},
    { "auml;", 6, 228},
    { "brvbar;", 8, 166},
    { "Ccedil;", 8, 199},
    { "ccedil;", 8, 231},
    { "cedil;", 7, 184},
    { "cent;", 6, 162},
    { "copy;", 6, 169},
    { "curren;", 8, 164},
    { "deg;", 5, 176},
    { "divide;", 8, 247},
    { "Eacute;", 8, 201},
    { "eacute;", 8, 233},
    { "Ecirc;", 7, 202},
    { "ecirc;", 7, 234},
    { "Egrave;", 8, 200},
    { "egrave;", 8, 232},
    { "ETH;", 5, 208},
    { "eth;", 5, 240},
    { "Euml;", 6, 203},
    { "euml;", 6, 235},
    { "frac12;", 8, 189},
    { "frac14;", 8, 188},
    { "frac34;", 8, 190},
    { "gt;", 4, '>'},
    { "Iacute;", 8, 205},
    { "iacute;", 8, 237},
    { "Icirc;", 7, 206},
    { "icirc;", 7, 238},
    { "iexcl;", 7, 161},
    { "Igrave;", 8, 204},
    { "igrave;", 8, 236},
    { "iquest;", 8, 191},
    { "Iuml;", 6, 207},
    { "iuml;", 6, 239},
    { "laquo;", 7, 171},
    { "lt;", 4, '<'},
    { "macr;", 6, 175},
    { "micro;", 7, 181},
    { "middot;", 8, 183},
    { "nbsp;", 6, ' '},
    { "not;", 5, 172},
    { "Ntilde;", 8, 209},
    { "ntilde;", 8, 241},
    { "Oacute;", 8, 211},
    { "oacute;", 8, 243},
    { "Ocirc;", 7, 212},
    { "ocirc;", 7, 244},
    { "Ograve;", 8, 210},
    { "ograve;", 8, 242},
    { "ordf;", 6, 170},
    { "ordm;", 6, 186},
    { "Oslash;", 8, 216},
    { "oslash;", 8, 248},
    { "Otilde;", 8, 213},
    { "otilde;", 8, 245},
    { "Ouml;", 6, 214},
    { "ouml;", 6, 246},
    { "para;", 6, 182},
    { "plusmn;", 8, 177},
    { "pound;", 7, 163},
    { "quot;", 6, '\"'},
    { "raquo;", 7, 187},
    { "reg;", 5, 174},
    { "sect;", 6, 167},
    { "shy;", 5, 173},
    { "sup1;", 6, 185},
    { "sup2;", 6, 178},
    { "sup3;", 6, 179},
    { "szlig;", 7, 223},
    { "THORN;", 7, 222},
    { "thorn;", 7, 254},
    { "times;", 7, 215},
    { "Uacute;", 8, 218},
    { "uacute;", 8, 250},
    { "Ucirc;", 7, 219},
    { "ucirc;", 7, 251},
    { "Ugrave;", 8, 217},
    { "ugrave;", 8, 249},
    { "uml;", 5, 168},
    { "Uuml;", 6, 220},
    { "uuml;", 6, 252},
    { "Yacute;", 8, 221},
    { "yacute;", 8, 253},
    { "yen;", 5, 165},
    { "yuml;", 6, 255},
    { "bull;", 6, 206}
};

int entityMapLen=(int)(sizeof(entityMap)/sizeof(entityMap[0]));

#endif

#endif
