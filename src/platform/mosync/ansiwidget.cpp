// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "platform/mosync/ansiwidget.h"
#include "platform/mosync/utils.h"

/* class AnsiWidget

  Displays ANSI escape codes.

  Escape sequences start with the characters ESC (ASCII 27d / 1Bh / 033o )
  and [ (left bracket). This sequence is called CSI for
  "Control Sequence Introducer".

  For more information about ANSI code see:
  http://en.wikipedia.org/wiki/ANSI_escape_code
  http://www.uv.tietgen.dk/staff/mlha/PC/Soft/Prog/BAS/VB/Function.html
  http://bjh21.me.uk/all-escapes/all-escapes.txt

  Supported control codes:
  \t      tab (20 px)
  \a      beep
  \r      return
  \n      next line
  \xC     clear screen (new page)
  \e[K    clear to end of line
  \e[0m   reset all attributes to their defaults
  \e[1m   set bold on
  \e[4m   set underline on
  \e[7m   reverse video
  \e[21m  set bold off
  \e[24m  set underline off
  \e[27m  set reverse off
*/

#define MAX_PENDING 250
#define BUTTON_PADDING 10
#define OVER_SCROLL 100
#define SWIPE_MAX_TIMER 6000
#define SWIPE_DELAY_STEP 250
#define SWIPE_SCROLL_FAST 30
#define SWIPE_SCROLL_SLOW 20
#define SWIPE_TRIGGER_FAST 55
#define SWIPE_TRIGGER_SLOW 80
#define FONT_FACTOR 32

char *options = NULL;
FormList *clickedList = NULL;

Widget::Widget(int bg, int fg, int x, int y, int w, int h) :
  Shape(x, y, w, h),
  pressed(false),
  bg(bg),
  fg(fg) {
}

void Widget::drawButton(const char *caption, int dx, int dy) {
  int r = dx + width - 1;
  int b = dy + height - 1;
  MAExtent textSize = maGetTextSize(caption);
  int textW = EXTENT_X(textSize);
  int textH = EXTENT_Y(textSize);
  int textX = dx + (width - textW - 1) / 2;
  int textY = dy + (height - textH - 1) / 2;

  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(dx, dy, width-1, height-1);

  if (pressed) {
    maSetColor(0x909090);
    maLine(dx, dy, r, dy);  // top
    maLine(dx, dy, dx, b);  // left
    maSetColor(0xd0d0d0);
    maLine(dx+1, b, r, b);     // bottom
    maLine(r-1, dy+1, r-1, b); // right
    maSetColor(0x606060);
    maLine(dx+1, dy+1, r-1, dy+1); // bottom
    maLine(dx+1, b-1, dx+1, b-1); // right
    textX += 2;
    textY += 2;
  } else {
    maSetColor(0xd0d0d0);
    maLine(dx, dy, r, dy); // top
    maLine(dx, dy, dx, b); // left
    maSetColor(0x606060);
    maLine(dx, b, r, b);     // bottom
    maLine(r-1, dy, r-1, b); // right
    maSetColor(0x909090);
    maLine(dx+1, b-1, r-1, b-1); // bottom
    maLine(r-2, dy+1, r-2, b-1); // right
  }

  maSetColor(fg);
  maDrawText(textX, textY, caption);
}

void Widget::drawLink(const char *caption, int dx, int dy) {
  maSetColor(fg);
  maDrawText(dx, dy, caption);
  maSetColor(pressed ? fg : bg);
  maLine(dx + 2, dy + height + 1, dx + width, dy + height + 1);
}

bool Widget::overlaps(MAPoint2d pt, int offsX, int offsY) {
  return !(OUTSIDE_RECT(pt.x, pt.y, x + offsX, y + offsY, width, height));
}

// returns setBG when the program colours are default
int Widget::getBackground(int buttonColor) {
  int result = bg;
  if (fg == DEFAULT_COLOR && bg == 0) {
    result = buttonColor;
  }
  return result;
}

Button::Button(Screen *screen, const char* action, const char *label,
               int x, int y, int w, int h) :
  Widget(screen->bg, screen->fg, x, y, w, h),
  action(action),
  label(label) {
  screen->shapes.add(this);
}

void Button::clicked(IButtonListener *listener, int x, int y) {
  listener->buttonClicked(action.c_str());
}

FormWidget::FormWidget(Screen *screen, int x, int y, int w, int h) :
  Widget(screen->bg, screen->fg, x, y, w, h),
  screen(screen),
  listener(NULL) {
  getScreen()->add(this); 
}

FormWidget::~FormWidget() {
  getScreen()->remove(this);
}

void FormWidget::clicked(IButtonListener *, int x, int y) {
  if (this->listener) {
    this->listener->buttonClicked(NULL); 
  }
}

FormButton::FormButton(Screen *screen, const char *caption, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  caption(caption) {
}

FormLabel::FormLabel(Screen *screen, const char *caption, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  caption(caption) {
}

FormLink::FormLink(Screen *screen, const char *link, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  link(link) {
}

FormLineInput::FormLineInput(Screen *screen, char *buffer, int maxSize, 
                             int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  buffer(buffer),
  maxSize(maxSize),
  scroll(0) {
}

void FormLineInput::draw(int dx, int dy) {
  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(dx, dy, width, height);
  maSetColor(fg);
  maDrawText(dx, dy, buffer + scroll);
}

bool FormLineInput::edit(int key) {
  bool changed = false;
  int len = strlen(buffer);

  if (key >= MAK_SPACE && key <= MAK_Z) {
    // insert
    if (len < maxSize - 1) {
      buffer[len] = key;
      buffer[++len] = '\0';
      int textWidth = get_text_width(buffer);
      if (textWidth > width) {
        if (textWidth > getScreen()->width) {
          scroll++;
        } else {
          width += getScreen()->charWidth;
        }
      }
      changed = true;
    }
  } else if (key == MAK_CLEAR) {
    // delete
    if (len > 0) {
      buffer[len - 1] = '\0';
      if (scroll) {
        scroll--;
      }
      changed = true;
    }
  } else {
    maShowVirtualKeyboard();
  }

  return changed;
}

FormList::FormList(Screen *screen, IFormWidgetListModel *model,
                   int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  model(model) {
  if (model->rows()) {
    model->selected(0);
  }
}

void FormList::clicked(IButtonListener *listener, int x, int y) {
  // calculate the size of the options buffer
  int optionsBytes = sizeof(int);
  for (int i = 0; i < model->rows(); i++) {
    const char *str = model->getTextAt(i);
    optionsBytes += (strlen(str) + 1) * sizeof(wchar);
  }

  // create the options buffer
  delete [] options;
  options = new char[optionsBytes];
  *(int *)options = model->rows();
  wchar_t *dst = (wchar_t *)(options + sizeof(int));
  
  for (int i = 0; i < model->rows(); i++) {
    const char *str = model->getTextAt(i);
    int len = strlen(str);
    swprintf(dst, len + 1, L"%hs", str);
    dst[len] = 0;
    dst += (len + 1);
  }
  
  clickedList = this;
  maOptionsBox(L"SmallBASIC", NULL, L"Close", (MAAddress)options, optionsBytes);
}

void FormList::draw(int x, int y) {
  if (model) {
    drawButton(getText(), x, y);
  }
}

void FormList::optionSelected(int index) {
  if (index < model->rows()) {
    model->selected(index);
    FormWidget::clicked(NULL, -1, -1);
  }
}

AnsiWidget::AnsiWidget(IButtonListener *listener, int width, int height) :
  back(NULL),
  front(NULL),
  focus(NULL),
  width(width),
  height(height),
  xTouch(-1),
  yTouch(-1),
  xMove(-1),
  yMove(-1),
  moveTime(0),
  moveDown(false),
  swipeExit(false),
  buttonListener(listener),
  activeButton(NULL) {
  for (int i = 0; i < MAX_SCREENS; i++) {
    screens[i] = NULL;
  }
  fontSize = min(width, height) / FONT_FACTOR;
  trace("width: %d height: %d fontSize:%d", width, height, fontSize);
}

bool AnsiWidget::construct() {
  bool result = false;
  back = new GraphicScreen(width, height, fontSize);
  if (back && back->construct()) {
    screens[0] = front = back;
    clearScreen();
    result = true;
  }
  return result;
}

// widget clean up
AnsiWidget::~AnsiWidget() {
  for (int i = 0; i < MAX_SCREENS; i++) {
    delete screens[i];
  }
  delete [] options;
}

// create audible beep sound
void AnsiWidget::beep() const {
  // http://www.mosync.com/documentation/manualpages/using-audio-api
}

// creates a Button attached to the current back screen
IFormWidget *AnsiWidget::createButton(char *caption, int x, int y, int w, int h) {
  FormButton *button = new FormButton(back, caption, x, y, w, h);
  return button;
}

// creates a Label attached to the current back screen
IFormWidget *AnsiWidget::createLabel(char *caption, int x, int y, int w, int h) {
  FormLabel *label = new FormLabel(back, caption, x, y, w, h);
  return label;
}

// creates a LineInput attached to the current back screen
IFormWidget *AnsiWidget::createLineInput(char *buffer, int maxSize,
                                         int x, int y, int w, int h) {
  FormLineInput *lineInput = new FormLineInput(back, buffer, maxSize, x, y, w, h);
  return lineInput;
}

// creates a form based hyperlink
IFormWidget *AnsiWidget::createLink(char *caption, int x, int y, int w, int h) {
  FormLink *result;
  if (w == 0 && h == 0) {
    result = (FormLink *)createLink(caption, caption, true, false);
  } else {
    result = new FormLink(back, caption, x, y, w, h);
  }
  return result;
}

// creates a List attached to the current back screen
IFormWidget *AnsiWidget::createList(IFormWidgetListModel *model, 
                                    int x, int y, int w, int h) {
  FormList *list = new FormList(back, model, x, y, w, h);
  return list;
}

// draws the given image onto the offscreen buffer
void AnsiWidget::drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h) {
  // TODO - draw image
  flush(false);
}

// draw a line onto the offscreen buffer
void AnsiWidget::drawLine(int x1, int y1, int x2, int y2) {
  back->drawLine(x1, y1, x2, y2);
  flush(false);
}

// draw a rectangle onto the offscreen buffer
void AnsiWidget::drawRect(int x1, int y1, int x2, int y2) {
  back->drawRect(x1, y1, x2, y2);
  flush(false);
}

// draw a filled rectangle onto the offscreen buffer
void AnsiWidget::drawRectFilled(int x1, int y1, int x2, int y2) {
  back->drawRectFilled(x1, y1, x2, y2);
  flush(false);
}

// perform editing when the formWidget belongs to the front screen
void AnsiWidget::edit(IFormWidget *formWidget, int c) {
  if (front == ((FormWidget *)formWidget)->getScreen()) {
    if (formWidget->edit(c)) {
      redraw();
    }
  }
}

// display and pending images changed
void AnsiWidget::flush(bool force, bool vscroll) {
  bool update = false;
  if (front != NULL) {
    if (force) {
      update = front->dirty;
    } else if (front->dirty) {
      update = (maGetMilliSecondCount() - front->dirty >= MAX_PENDING);
    }
    if (update) {
      front->draw(vscroll);
    }
  }
}

// returns the color of the pixel at the given xy location
int AnsiWidget::getPixel(int x, int y) {
  return back->getPixel(x, y);
}

// Returns the height in pixels using the current font setting
int AnsiWidget::textHeight(void) {
  return back->charHeight;
}

bool AnsiWidget::optionSelected(int index) {
  bool result = false;
  if (clickedList != NULL) {
    clickedList->optionSelected(index);
    clickedList = NULL;
    result = false;
  } 
  return result;
}

// prints the contents of the given string onto the backbuffer
void AnsiWidget::print(const char *str) {
  int len = strlen(str);
  if (len) {
    back->drawInto();

    int lineHeight = textHeight();

    // copy the string to allow length manipulation
    char *buffer = new char[len + 1];
    strncpy(buffer, str, len);
    buffer[len] = 0;
    char *p = buffer;

    while (*p) {
      switch (*p) {
      case '\a':   // beep
        beep();
        break;
      case '\t':
        back->calcTab();
        break;
      case '\003': // end of text
        flush(true);
        break;
      case '\xC':
        clearScreen();
        break;
      case '\033': // ESC ctrl chars
        handleEscape(p, lineHeight);
        break;
      case '\034':
        // file separator
        break;
      case '\n':   // new line
        back->newLine(lineHeight);
        break;
      case '\r':   // return
        back->curX = INITXY;     // erasing the line will clear any previous text
        break;
      default:
        p += back->print(p, lineHeight) - 1; // allow for p++
      };

      if (*p == '\0') {
        break;
      }
      p++;
    }

    // cleanup
    delete [] buffer;
    flush(false);
  }
}

// redraws and flushes the front screen
void AnsiWidget::redraw() {
  front->drawInto();
  flush(true);
}

// reinit for new program run
void AnsiWidget::reset() {
  back = front = screens[0];
  back->reset(fontSize);
  back->clear();

  // reset non-system screens
  for (int i = 1; i < SYSTEM_SCREENS; i++) {
    delete screens[i];
    screens[i] = NULL;
  }
}

// update the widget to new dimensions
void AnsiWidget::resize(int newWidth, int newHeight) {
  int lineHeight = textHeight();
  for (int i = 0; i < MAX_SCREENS; i++) {
    Screen *screen = screens[i];
    if (screen) {
      screen->resize(newWidth, newHeight, width, height, lineHeight);
      if (screen == front || i < SYSTEM_SCREENS) {
        screen->draw(false);
      }
    }
  }
  width = newWidth;
  height = newHeight;
}

// sets the current drawing color
void AnsiWidget::setColor(long fg) {
  back->setColor(fg);
}

// sets the text font size
void AnsiWidget::setFontSize(int fontSize) {
  this->fontSize = fontSize;
  for (int i = 0; i < SYSTEM_SCREENS; i++) {
    if (screens[i] != NULL) {
      screens[i]->reset(fontSize);
    }
  }
  redraw();
}

// sets the pixel to the given color at the given xy location
void AnsiWidget::setPixel(int x, int y, int c) {
  back->setPixel(x, y, c);
}

// sets the current text drawing color
void AnsiWidget::setTextColor(long fg, long bg) {
  back->setTextColor(fg, bg);
}

// handler for pointer touch events
void AnsiWidget::pointerTouchEvent(MAEvent &event) {
  // hit test buttons on the front screen
  if (setActiveButton(event, front)) {
    focus = front;
  } else {
    // hit test buttons on remaining screens
    for (int i = 0; i < SYSTEM_SCREENS; i++) {
      if (screens[i] != NULL && screens[i] != front) {
        if (setActiveButton(event, screens[i])) {
          focus = screens[i];
          break;
        }
      }
    }
  }
  // paint the pressed button
  if (activeButton != NULL) {
    drawActiveButton();
  }
  // setup vars for page scrolling
  if (front->overlaps(event.point.x, event.point.y)) {
    xTouch = xMove = event.point.x;
    yTouch = yMove = event.point.y;
  }
}

// handler for pointer move events
void AnsiWidget::pointerMoveEvent(MAEvent &event) {
  if (activeButton != NULL) {
    bool pressed = activeButton->overlaps(event.point, focus->x,
                                          focus->y - focus->scrollY);
    if (pressed != activeButton->pressed) {
      activeButton->pressed = pressed;
      drawActiveButton();
    }
  } else if (!swipeExit) {
    // scroll up/down
    if (front->overlaps(event.point.x, event.point.y)) {
      int vscroll = front->scrollY + (yMove - event.point.y);
      int maxScroll = (front->curY - front->height) + (2 * fontSize);
      if (vscroll < 0) {
        vscroll = 0;
      }
      if (vscroll != front->scrollY && maxScroll > 0 && 
          vscroll < maxScroll + OVER_SCROLL) {
        moveTime = maGetMilliSecondCount();
        moveDown = (front->scrollY < vscroll);
        front->drawInto();
        front->scrollY = vscroll;
        xMove = event.point.x;
        yMove = event.point.y;
        flush(true, true);
      }
    }
  }
}

// handler for pointer release events
void AnsiWidget::pointerReleaseEvent(MAEvent &event) {
  if (activeButton != NULL && activeButton->pressed) {
    activeButton->pressed = false;
    drawActiveButton();
    activeButton->clicked(buttonListener, event.point.x, event.point.y);
  } else if (swipeExit) {
    swipeExit = false;
  } else {
    int maxScroll = (front->curY - front->height) + (2 * fontSize);
    if (yMove != -1 && maxScroll > 0) {
      front->drawInto();
      bool swiped = (abs(xTouch - xMove) > (width / 3) ||
                     abs(yTouch - yMove) > (height / 3));
      int start = maGetMilliSecondCount();
      if (swiped && start - moveTime < SWIPE_TRIGGER_SLOW) {
        doSwipe(start, maxScroll);
      } else if (front->scrollY > maxScroll) {
        front->scrollY = maxScroll;
      }
      // ensure the scrollbar is removed
      front->dirty = true;
      flush(true);
      moveTime = 0;
    }
  }

  xTouch = xMove = -1;
  yTouch = yMove = -1;
  activeButton = NULL;
  focus = NULL;
}

// creates a status-bar label
void AnsiWidget::createLabel(char *&p) {
  Vector<String *> *items = getItems(p);
  const char *label = items->size() > 0 ? (*items)[0]->c_str() : "";
  back->label = label;
  deleteItems(items);
}

// creates a hyperlink, eg // ^[ hwww.foo.com|title;More text
Widget *AnsiWidget::createLink(char *&p, bool formLink, bool button) {
  Vector<String *> *items = getItems(p);
  const char *action = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *text = items->size() > 1 ? (*items)[1]->c_str() : action;
  Widget *result = createLink(action, text, formLink, button);
  deleteItems(items);
  return result;
}

Widget *AnsiWidget::createLink(const char *action, const char *text,
                               bool formLink, bool button) {
  MAExtent textSize = maGetTextSize(text);
  int w = EXTENT_X(textSize) + 2;
  int h = EXTENT_Y(textSize) + 2;
  int x = back->curX;
  int y = back->curY;

  if (button) {
    w += BUTTON_PADDING;
    h += BUTTON_PADDING;
    back->linePadding = BUTTON_PADDING;
  }
  if (back->curX + w >= width) {
    w = width - back->curX; // clipped
    back->newLine(EXTENT_Y(textSize) + LINE_SPACING);
  } else {
    back->curX += w;
  }
  Widget *result;
  if (formLink) {
    result = new FormLink(back, action, x, y, w, h);
  } else if (button) {
    result = new BlockButton(back, action, text, x, y, w, h);
  } else {
    result = new TextButton(back, action, text, x, y, w, h);
  }
  return result;
}

// create an options dialog
void AnsiWidget::createOptionsBox(char *&p) {
  Vector<String *> *items = getItems(p);
  if (items->size()) {
    // calculate the size of the options buffer
    int optionsBytes = sizeof(int);
    Vector_each(String*, it, *items) {
      const char *str = (*it)->c_str();
      optionsBytes += (strlen(str) + 1) * sizeof(wchar);
    }

    // create the options buffer
    delete [] options;
    options = new char[optionsBytes];
    *(int *)options = items->size();
    wchar_t *dst = (wchar_t *)(options + sizeof(int));

    Vector_each(String*, it, *items) {
      const char *str = (*it)->c_str();
      int len = strlen(str);
      swprintf(dst, len + 1, L"%hs", str);
      dst[len] = 0;
      dst += (len + 1);
    }
    maOptionsBox(L"SmallBASIC", NULL, L"Close", (MAAddress)options, optionsBytes);
  }
  deleteItems(items);
}

// cleanup the string list created in getItems()
void AnsiWidget::deleteItems(Vector<String *> *items) {
  Vector_each(String*, it, *items) {
    delete (*it);
  }
  delete items;
}

// handles the characters following the \e[ sequence. Returns whether a further call
// is required to complete the process.
bool AnsiWidget::doEscape(char *&p, int textHeight) {
  int escValue = 0;

  while (isdigit(*p)) {
    escValue = (escValue *10) + (*p - '0');
    p++;
  }

  if (*p == ' ') {
    p++;
    switch (*p) {
    case 'A':
      showAlert(p);
      break;
    case 'B':
      createLink(p, false, true);
      break;
    case 'C':
      // GSS Graphic Size Selection
      back->fontSize = escValue;
      back->updateFont();
      break;
    case 'K':
      maShowVirtualKeyboard();
      break;
    case 'H':
      createLink(p, false, false);
      break;
    case 'L':
      createLabel(p);
      break;
    case 'O':
      createOptionsBox(p);
      break;
    case 'S':
      screenCommand(p);
      break;
    }
    if (p[1] == ';') {
      // advance to the separator
      p++;
    }
  } else if (back->setGraphicsRendition(*p, escValue, textHeight)) {
    back->updateFont();
  }

  bool result = false;
  if (*p == ';') {
    result = true;
    // advance to next rendition
    p++;  
  }
  return result;
}

// swipe handler for pointerReleaseEvent()
void AnsiWidget::doSwipe(int start, int maxScroll) {
  MAEvent event;
  int elapsed = 0;
  int vscroll = front->scrollY;
  int scrollSize = (start - moveTime < SWIPE_TRIGGER_FAST) ? 
                   SWIPE_SCROLL_FAST : SWIPE_SCROLL_SLOW;
  int swipeStep = SWIPE_DELAY_STEP;
  while (elapsed < SWIPE_MAX_TIMER) {
    if (maGetEvent(&event) && event.type == EVENT_TYPE_POINTER_PRESSED) {
      // ignore the next move and release events
      swipeExit = true;
      break;
    }
    elapsed += (maGetMilliSecondCount() - start);
    if (elapsed > swipeStep && scrollSize > 1) {
      // step down to a lesser scroll amount
      scrollSize -= 1;
      swipeStep += SWIPE_DELAY_STEP;
    }
    if (scrollSize == 1) {
      maWait(20);
    }
    vscroll += moveDown ? scrollSize : -scrollSize;
    if (vscroll < 0) {
      vscroll = 0;
    } else if (vscroll > maxScroll) {
      vscroll = maxScroll;
    }
    if (vscroll != front->scrollY) {
      front->dirty = true; // forced
      front->scrollY = vscroll;
      flush(true, true);
    } else {
      break;
    }
  }
  
  // pause before removing the scrollbar
  maWait(500);
}

// draws the focus screen
void AnsiWidget::drawActiveButton() {
  MAHandle currentHandle = maSetDrawTarget(HANDLE_SCREEN);
  int x = focus->x + activeButton->x;
  int y = focus->y + activeButton->y - focus->scrollY;
  maSetClipRect(x, y, activeButton->width, activeButton->height + 2);
  activeButton->draw(x, y);
  maUpdateScreen();
  maResetBacklight();
  maSetDrawTarget(currentHandle);
}

// returns list of strings extracted from the vertical-bar separated input string
Vector<String *> *AnsiWidget::getItems(char *&p) {
  Vector<String *> *result = new Vector<String *>();
  char *next = p + 1;
  bool eot = false;

  while (*p && !eot) {
    if (p[1] < 32) {
      // end when control character encountered
      int len = (p - next) + 1;
      if (len) {
        result->add(new String((const char *)next, len));
      }
      eot = true;
    } else {
      p++;
      switch (*p) {
      case ';':
        eot = true;
        // fallthru
      case '|':
        result->add(new String((const char *)next, (p - next)));
        next = p + 1;
        break;
      default:
        break;
      }
    }
  }
  return result;
}

// print() helper
void AnsiWidget::handleEscape(char *&p, int lineHeight) {
  if (*(p + 1) == '[') {
    p += 2;
    while (doEscape(p, lineHeight)) {
      // continue
    }
  }
}

// remove the specified screen
void AnsiWidget::removeScreen(char *&p) {
  logEntered();
  Vector<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  if (n < 1 || n >= MAX_SCREENS) {
    print("ERR invalid screen number");
  } else if (screens[n] != NULL) {
    if (back = screens[n]) {
      back = screens[0];
    }
    if (front = screens[n]) {
      front = screens[0];
    }
    delete screens[n];
    screens[n] = NULL;
  }
  deleteItems(items);
}

// screen escape commands
void AnsiWidget::screenCommand(char *&p) {
  Screen *selected;
  p++;

  switch (*p) {
  case 'X': // double buffering - transpose write and display screens
    swapScreens();
    break;
  case 'E': // erase a screen
    removeScreen(p);
    break;
  case 'R': // redraw all user screens
    for (int i = 0; i < MAX_SCREENS; i++) {
      if (screens[i] && i < SYSTEM_SCREENS) {
        screens[i]->draw(false);
        front = back = screens[i];
      }
    }
    break;
  case 'W': // select write/back screen
    selected = selectScreen(p);
    if (selected) {
      back = selected;
    }
    break;
  case 'w': // revert write/back to front screen
    back = front;
    break;
  case 'D': // select display/front screen
    selected = selectScreen(p);
    if (selected) {
      front = selected;
      front->dirty = true;
      flush(true);
    }
    break;
  case '#': // open screen # for write/display
    selected = selectScreen(p);
    if (selected) {
      front = back = selected;
      front->dirty = true;
      flush(true);
    }
    break;
  default:
    print("ERR unknown screen command");
    break;
  }    
}

// returns whether the event is over the given screen
bool AnsiWidget::setActiveButton(MAEvent &event, Screen *screen) {
  bool result = false;
  if (screen->overlaps(event.point.x, event.point.y)) {
    Vector_each(Shape*, it, screen->shapes) {
      Widget *widget = (Widget *)(*it);
      if (widget->overlaps(event.point, screen->x,
                           screen->y - screen->scrollY)) {
        activeButton = widget;
        activeButton->pressed = true;
        break;
      }
    }
    // screen overlaps event - avoid search in other screens
    result = true;
  }
  return result;
}

// select the specified screen - returns whether the screen was changed
Screen *AnsiWidget::selectScreen(char *&p) {
  Vector<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  int x = items->size() > 1 ? atoi((*items)[1]->c_str()) : 0;
  int y = items->size() > 2 ? atoi((*items)[2]->c_str()) : 0;
  int w = items->size() > 3 ? atoi((*items)[3]->c_str()) : 100;
  int h = items->size() > 4 ? atoi((*items)[4]->c_str()) : 100;

  Screen *result = NULL;
  flush(true);

  if (n < 1 || n >= MAX_SCREENS) {
    n = 1;
  }

  x = min(max(x, 0), 100);
  y = min(max(y, 0), 100);
  w = min(max(w, 0), 100);
  h = min(max(h, 0), 100);

  result = screens[n];

  if (result != NULL) {
    // specified screen already exists
    if (result->x != x ||
        result->y != y ||
        result->width != width ||
        result->height != height) {
      delete result;
      result = NULL;
    }
  }
  
  if (result == NULL) {
    if (n > 1) {
      result = new TextScreen(width, height, fontSize, x, y, w, h);
    } else {
      result = new GraphicScreen(width, height, fontSize);
    }    
    if (result && result->construct()) {
      screens[n] = result;
      result->drawInto();
      result->clear();
    } else {
      trace("Failed to create screen %d", n);
    }
  }
  
  deleteItems(items);
  return result;
}

// display an alert box - eg // ^[ aAlert!!;
void AnsiWidget::showAlert(char *&p) {
  Vector<String *> *items = getItems(p);

  const char *title = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *message = items->size() > 1 ? (*items)[1]->c_str() : "";
  const char *button1 = items->size() > 2 ? (*items)[2]->c_str() : "";
  const char *button2 = items->size() > 3 ? (*items)[3]->c_str() : "";
  const char *button3 = items->size() > 4 ? (*items)[4]->c_str() : "";

  maAlert(title, message, button1, button2, button3);
  deleteItems(items);
}

// transpose the front and back screens
void AnsiWidget::swapScreens() {
  if (front == back) {
    if (screens[1] != NULL) {
      front = screens[1];
    } else {
      front = new GraphicScreen(width, height, fontSize);
      if (front && front->construct()) {
        screens[1] = front;
      } else {
        trace("Failed to create screen");
      }
    }
  } else {
    Screen *tmp = front;
    front = back;
    back = tmp;
    if (front->dirty) {
      front->draw(false);
    }
  }
}
