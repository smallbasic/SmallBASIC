// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef FL_HELP_WIDGET
#define FL_HELP_WIDGET

#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/Image.h>
#include <fltk/SharedImage.h>
#include <fltk/Group.h>
#include <fltk/Scrollbar.h>

#include "ui/StringLib.h"

#define ID_BUTTON   1
#define ID_TEXTBOX  2
#define ID_TEXTAREA 3
#define ID_CHKBOX   4
#define ID_RADIO    5
#define ID_SELECT   6
#define ID_RANGEVAL 7
#define ID_HIDDEN   8
#define ID_READONLY 9

#define MIN_FONT_SIZE 11
#define MAX_FONT_SIZE 22
#define EVENT_INCREASE_FONT 100
#define EVENT_DECREASE_FONT 101
#define EVENT_COPY_TEXT     102
#define EVENT_SEL_ALL_TEXT  103
#define EVENT_FIND          104
#define EVENT_PG_DOWN       105
#define EVENT_PG_UP         106

using namespace fltk;
using namespace strlib;

SharedImage *loadImage(const char *name, uchar *buff);
void browseFile(const char *s);

struct BaseNode;
struct NamedInput;
struct InputNode;
struct AnchorNode;
struct ImageNode;

class HelpWidget : public Group {
public:
  HelpWidget(int x, int y, int width, int height, int defsize = MIN_FONT_SIZE);
  virtual ~HelpWidget();

  void loadBuffer(const char *buffer);
  void loadFile(const char *fileName, bool useDocHome = false);
  void navigateTo(const char *fileName);
  void scrollTo(const char *anchorName);
  void scrollTo(int vscroll);
  int getVScroll() { return vscroll; }
  int getHScroll() { return hscroll; }
  bool find(const char *s, bool matchCase);
  Widget *getInput(const char *name);
  const char *getInputValue(Widget *button);
  const char *getInputValue(int i);
  const char *getInputName(Widget *button);
  const char *getTitle() { return title; }
  const char *getFileName() { return fileName; }
  const char *getDocHome() { return docHome; }
  const char *getSelection() { return selection; }
  const strlib::String getEventName() { return event; }
  void getText(strlib::String *s);
  void getInputProperties(Properties *p);
  void setCookies(Properties *p) { cookies = p; }
  bool setInputValue(const char *assignment);
  void selectAll();
  void copySelection();
  void onclick(Widget *button);        // private
  void setDocHome(const char *home);
  void reloadImages();
  void setFontSize(int i);
  int getFontSize() { return (int)labelsize(); }
  void setSelectMode() { mouseMode = mm_select; }
  void setPageMode() { mouseMode = mm_page; }
  void setScrollMode() { mouseMode = mm_scroll; }
  int getNumAnchors() { return anchors.size(); }
  const char *getAnchor(int index);
  bool isHtmlFile();
  void setTitle(const char *s) { title.empty(); title.append(s); }

protected:
  void reloadPage();
  void compile();
  void init();
  void cleanup();
  void endSelection();

  // fltk methods
  void draw();
  void layout();
  int onMove(int event);
  int onPush(int event);
  int handle(int event);

private:
  Color background, foreground;
  Scrollbar *scrollbar;
  S16 vscroll, hscroll;
  U16 scrollHeight;
  S16 markX, markY, pointX, pointY;
  S16 scrollY;                  // nm_scroll
  enum { mm_select, mm_page, mm_scroll } mouseMode;
  strlib::List<BaseNode *> nodeList;
  strlib::List<NamedInput *> namedInputs;
  strlib::List<InputNode *> inputs;
  strlib::List<AnchorNode *> anchors;
  strlib::List<ImageNode *> images;
  strlib::Properties *cookies;
  strlib::String htmlStr;
  strlib::String event;
  strlib::String fileName;
  strlib::String docHome;
  strlib::String title;
  strlib::String selection;
};

#ifdef FL_HELP_WIDGET_RESOURCES
// somewhere to keep this clutter

static const char *dot_xpm[] = {
  "5 5 3 1",
  "   c None",
  ".  c #F4F4F4",
  "+  c #000000",
  ".+++.",
  "+++++",
  "+++++",
  "+++++",
  ".+++."
};

static xpmImage dotImage(dot_xpm);

static const char *ellipse_xpm[] = {
  "6 1 2 1",
  " 	c #000000",
  ".	c #FFFFFF",
  " . . ."
};

static xpmImage ellipseImage(ellipse_xpm);

static const char *broken_xpm[] = {
  "16 18 4 1",
  "@ c #000000",
  "  c #ffffff",
  "+ c none",
  "x c #ff0000",
  // pixels
  "@@@@@@@+++++++++",
  "@    @++++++++++",
  "@   @+++++++++++",
  "@   @++@++++++++",
  "@    @@+++++++++",
  "@     @+++@+++++",
  "@     @++@@++++@",
  "@ xxx  @@  @++@@",
  "@  xxx    xx@@ @",
  "@   xxx  xxx   @",
  "@    xxxxxx    @",
  "@     xxxx     @",
  "@    xxxxxx    @",
  "@   xxx  xxx   @",
  "@  xxx    xxx  @",
  "@ xxx      xxx @",
  "@              @",
  "@@@@@@@@@@@@@@@@",
  NULL
};

static xpmImage brokenImage(broken_xpm);

struct ENTITY_MAP {
  const char *ent;
  int elen;
  char xlat;
} entityMap[] = {
  {
  "lsquor;", 8, 130}, {
  "fnof;", 5, 131}, {
  "ldquor;", 8, 132}, {
  "hellip;", 8, 133}, {
  "dagger;", 8, 134}, {
  "Dagger;", 8, 135}, {
  "xxx;", 5, 136}, {
  "permil;", 8, 137}, {
  "Scaron;", 8, 138}, {
  "lsaquo;", 8, 139}, {
  "OElig;", 7, 140}, {
  "OElig;", 7, 141}, {
  "OElig;", 7, 142}, {
  "OElig;", 7, 143}, {
  "OElig;", 7, 144}, {
  "lsquo;", 7, 145}, {
  "rsquo;", 7, 146}, {
  "ldquo;", 7, 147}, {
  "rdquo;", 7, 148}, {
  "bull;", 6, 149}, {
  "ndash;", 7, 150}, {
  "mdash;", 7, 151}, {
  "tilde;", 7, 152}, {
  "trade;", 7, 153}, {
  "scaron;", 8, 154}, {
  "rsaquo;", 8, 155}, {
  "oelig;", 7, 156}, {
  "oelig;", 7, 157}, {
  "oelig;", 7, 158}, {
  "Yuml;", 6, 159}, {
  "nbsp;", 6, ' '}, {
  "iexcl;", 7, 161}, {
  "cent;", 6, 162}, {
  "pound;", 7, 163}, {
  "curren;", 8, 164}, {
  "yen;", 5, 165}, {
  "brvbar;", 8, 166}, {
  "sect;", 6, 167}, {
  "uml;", 5, 168}, {
  "copy;", 6, 169}, {
  "ordf;", 6, 170}, {
  "laquo;", 7, 171}, {
  "not;", 5, 172}, {
  "shy;", 5, 173}, {
  "reg;", 5, 174}, {
  "macr;", 6, 175}, {
  "deg;", 5, 176}, {
  "plusmn;", 8, 177}, {
  "sup2;", 6, 178}, {
  "sup3;", 6, 179}, {
  "acute;", 7, 180}, {
  "micro;", 7, 181}, {
  "para;", 6, 182}, {
  "middot;", 8, 183}, {
  "cedil;", 7, 184}, {
  "sup1;", 6, 185}, {
  "ordm;", 6, 186}, {
  "raquo;", 7, 187}, {
  "frac14;", 8, 188}, {
  "frac12;", 8, 189}, {
  "frac34;", 8, 190}, {
  "iquest;", 8, 191}, {
  "Agrave;", 8, 192}, {
  "Aacute;", 8, 193}, {
  "Acirc;", 7, 194}, {
  "Atilde;", 8, 195}, {
  "Auml;", 6, 196}, {
  "Aring;", 7, 197}, {
  "AElig;", 7, 198}, {
  "Ccedil;", 8, 199}, {
  "Egrave;", 8, 200}, {
  "Eacute;", 8, 201}, {
  "Ecirc;", 7, 202}, {
  "Euml;", 6, 203}, {
  "Igrave;", 8, 204}, {
  "Iacute;", 8, 205}, {
  "Icirc;", 7, 206}, {
  "Iuml;", 6, 207}, {
  "ETH;", 5, 208}, {
  "Ntilde;", 8, 209}, {
  "Ograve;", 8, 210}, {
  "Oacute;", 8, 211}, {
  "Ocirc;", 7, 212}, {
  "Otilde;", 8, 213}, {
  "Ouml;", 6, 214}, {
  "times;", 7, 215}, {
  "Oslash;", 8, 216}, {
  "Ugrave;", 8, 217}, {
  "Uacute;", 8, 218}, {
  "Ucirc;", 7, 219}, {
  "Uuml;", 6, 220}, {
  "Yacute;", 8, 221}, {
  "THORN;", 7, 222}, {
  "szlig;", 7, 223}, {
  "agrave;", 8, 224}, {
  "aacute;", 8, 225}, {
  "acirc;", 7, 226}, {
  "atilde;", 8, 227}, {
  "auml;", 6, 228}, {
  "aring;", 7, 229}, {
  "aelig;", 7, 230}, {
  "ccedil;", 8, 231}, {
  "egrave;", 8, 232}, {
  "eacute;", 8, 233}, {
  "ecirc;", 7, 234}, {
  "euml;", 6, 235}, {
  "igrave;", 8, 236}, {
  "iacute;", 8, 237}, {
  "icirc;", 7, 238}, {
  "iuml;", 6, 239}, {
  "eth;", 5, 240}, {
  "ntilde;", 8, 241}, {
  "ograve;", 8, 242}, {
  "oacute;", 8, 243}, {
  "ocirc;", 7, 244}, {
  "otilde;", 8, 245}, {
  "ouml;", 6, 246}, {
  "divide;", 8, 247}, {
  "oslash;", 8, 248}, {
  "ugrave;", 8, 249}, {
  "uacute;", 8, 250}, {
  "ucirc;", 7, 251}, {
  "uuml;", 6, 252}, {
  "yacute;", 8, 253}, {
  "thorn;", 7, 254}, {
  "yuml;", 6, 255}, {
  "gt;", 4, '>'}, {
  "lt;", 4, '<'}, {
  "amp;", 5, '&'}, {
  "quot;", 6, '\"'},};

int entityMapLen = (int)(sizeof(entityMap) / sizeof(entityMap[0]));

#endif

#endif
