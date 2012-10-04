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
#define SWIPE_DISTANCE 100
#define SWIPE_MAX_TIMER 6000
#define SWIPE_DELAY_STEP 250
#define SWIPE_SCROLL_FAST 30
#define SWIPE_SCROLL_SLOW 20
#define SWIPE_TRIGGER_FAST 55
#define SWIPE_TRIGGER_SLOW 80

char *options = NULL;
FormList *clickedList = NULL;

Widget::Widget(int bg, int fg, int x, int y, int w, int h) :
  Shape(x, y, w, h),
  pressed(false),
  bg(bg),
  fg(fg) {
}

void Widget::drawButton(const char *caption) {
  int r = x+width-1;
  int b = y+height-1;
  int textX = x + 4;
  int textY = y + 4;

  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(x, y, width-1, height-1);

  if (pressed) {
    maSetColor(0x909090);
    maLine(x, y, r, y); // top
    maLine(x, y, x, b); // left
    maSetColor(0xd0d0d0);
    maLine(x+1, b, r, b);     // bottom
    maLine(r-1, y+1, r-1, b); // right
    maSetColor(0x606060);
    maLine(x+1, y+1, r-1, y+1); // bottom
    maLine(x+1, b-1, x+1, b-1); // right
    textX += 1;
    textY += 1;
  } else {
    maSetColor(0xd0d0d0);
    maLine(x, y, r, y); // top
    maLine(x, y, x, b); // left
    maSetColor(0x606060);
    maLine(x, b, r, b);     // bottom
    maLine(r-1, y, r-1, b); // right
    maSetColor(0x909090);
    maLine(x+1, b-1, r-1, b-1); // bottom
    maLine(r-2, y+1, r-2, b-1); // right
  }

  maSetColor(fg);
  maDrawText(textX, textY, caption);
}

bool Widget::overlaps(MAPoint2d pt, int scrollX, int scrollY) {
  return !(OUTSIDE_RECT(pt.x, pt.y, x - scrollX, y - scrollY, width, height));
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
}

void Button::clicked(IButtonListener *listener, int x, int y) { 
  listener->buttonClicked(action.c_str());
}

BlockButton::BlockButton(Screen *screen, const char *action, const char *label,
                         int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {
}

TextButton::TextButton(Screen *screen, const char *action, const char *label,
                       int x, int y, int w, int h) :
  Button(screen, action, label, x, y, w, h) {
}

void TextButton::draw() {
  maSetColor(fg);
  maDrawText(x, y, label.c_str());
  maSetColor(pressed ? fg : bg);
  maLine(x + 2, y + height + 1, x + width, y + height + 1);
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

void FormWidget::clicked(IButtonListener *listener, int x, int y) { 
  this->listener->buttonClicked(NULL); 
}

FormButton::FormButton(Screen *screen, const char *caption, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  caption(caption) {
}

FormLabel::FormLabel(Screen *screen, const char *caption, int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  caption(caption) {
}

FormLineInput::FormLineInput(Screen *screen, char *buffer, int maxSize, 
                             int x, int y, int w, int h) :
  FormWidget(screen, x, y, w, h),
  buffer(buffer),
  maxSize(maxSize),
  scroll(0) {
}

void FormLineInput::draw() {
  maSetColor(getBackground(GRAY_BG_COL));
  maFillRect(x, y, width, height);
  maSetColor(fg);
  maDrawText(x, y, buffer + scroll);
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

void FormList::draw() {
  if (model) {
    drawButton(getText());
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
  pushed(NULL),
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
  fontSize = min(width, height) / 40;
  trace("width: %d height: %d fontSize:%d", width, height, fontSize);
}

bool AnsiWidget::construct() {
  bool result = false;
  back = new GraphicScreen(0, 0, width, height, fontSize);
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

// creates a LineInput attached to the current back screen
IFormWidget *AnsiWidget::createLineInput(char *buffer, int maxSize,
                                         int x, int y, int w, int h) {
  FormLineInput *lineInput = new FormLineInput(back, buffer, maxSize, x, y, w, h);
  return lineInput;
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

// creates a List attached to the current back screen
IFormWidget *AnsiWidget::createList(IFormWidgetListModel *model, 
                                    int x, int y, int w, int h) {
  FormList *list = new FormList(back, model, x, y, w, h);
  return list;
}

// draws the given image onto the offscreen buffer
void AnsiWidget::drawImage(MAHandle image, int x, int y, int sx, int sy, int w, int h) {
  back->drawInto();
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
        if (*(p + 1) == '[') {
          p += 2;
          while (doEscape(p, lineHeight)) {
            // continue
          }
        }
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
}

// update the widget to new dimensions
void AnsiWidget::resize(int newWidth, int newHeight) {
  int lineHeight = textHeight();
  for (int i = 0; i < MAX_SCREENS; i++) {
    if (screens[i]) {
      screens[i]->resize(newWidth, newHeight, width, height, lineHeight);
    }
  }
  width = newWidth;
  height = newHeight;
  back->draw(false);
}

// sets the current drawing color
void AnsiWidget::setColor(long fg) {
  back->setColor(fg);
}

// sets the pixel to the given color at the given xy location
void AnsiWidget::setPixel(int x, int y, int c) {
  back->setPixel(x, y, c);
}

// sets the current text drawing color
void AnsiWidget::setTextColor(long fg, long bg) {
  back->setTextColor(fg, bg);
}

// return whether any of the screens contain a hyperlink
bool AnsiWidget::hasUI() {
  bool result = false;
  for (int i = 0; i < MAX_SCREENS && !result; i++) {
    if (screens[i] != NULL && screens[i]->shapes.size() > 0) {
      result = true;
    }
  }
  return result;
}

// handler for pointer touch events
void AnsiWidget::pointerTouchEvent(MAEvent &event) {
  if (!OUTSIDE_RECT(event.point.x, event.point.y,
                    front->x, front->y,
                    front->width, front->height)) {
    xTouch = xMove = event.point.x;
    yTouch = yMove = event.point.y;

    Vector_each(Shape*, it, front->shapes) {
      Widget *widget = (Widget *)(*it);
      if (widget->overlaps(event.point, 0, front->scrollY)) {
        activeButton = widget;
        activeButton->pressed = true;
        redraw();
        break;
      }
    }
  }
}

// handler for pointer move events
void AnsiWidget::pointerMoveEvent(MAEvent &event) {
  if (activeButton != NULL) {
    bool pressed = activeButton->overlaps(event.point, 0, front->scrollY);
    if (pressed != activeButton->pressed) {
      activeButton->pressed = pressed;
      redraw();
    }
  } else if (!swipeExit) {
    // scroll up/down
    if (!OUTSIDE_RECT(event.point.x, event.point.y,
                      front->x, front->y,
                      front->width, front->height)) {
      int vscroll = front->scrollY + (yMove - event.point.y);
      int maxScroll = (front->curY - front->height) + (2 * fontSize);
      if (vscroll < 0) {
        vscroll = 0;
      }
      if (vscroll != front->scrollY && maxScroll > 0) {
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
    activeButton->clicked(buttonListener, event.point.x, event.point.y);
    redraw();
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
}

// creates a status-bar label
void AnsiWidget::createLabel(char *&p) {
  Vector<String *> *items = getItems(p);
  const char *label = items->size() > 0 ? (*items)[0]->c_str() : "";
  back->label = label;
  deleteItems(items);
}

// creates a hyperlink, eg // ^[ hwww.foo.com|title;More text
void AnsiWidget::createLink(char *&p, bool execLink, bool button) {
  Vector<String *> *items = getItems(p);
  const char *action = items->size() > 0 ? (*items)[0]->c_str() : "";
  const char *text = items->size() > 1 ? (*items)[1]->c_str() : action;

  if (execLink && buttonListener) {
    buttonListener->buttonClicked(action);
  } else {
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
    Button *link;
    if (button) {
      link = new BlockButton(back, action, text, x, y, w, h);
    } else {
      link = new TextButton(back, action, text, x, y, w, h);
    }
    back->shapes.add(link);
  }

  deleteItems(items);
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
    case 'h':
      createLink(p, true, false);
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

// returns list of strings extracted from the vertical-bar separated input string
Vector<String *> *AnsiWidget::getItems(char *&p) {
  Vector<String *> *result = new Vector<String *>();
  char *next = p + 1;
  bool eot = false;
  int index = 0;

  while (*p && !eot) {
    p++;
    switch (*p) {
    case '\034':
    case '\033':
    case '\n':
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
  return result;
}

// remove the specified screen
void AnsiWidget::removeScreen(char *&p) {
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
  case 'R': // remove a screen
    removeScreen(p);
    break;
  case 'P': // remember the current screen
    pushed = back;
    break;
  case 'p': // restore the saved write and display screens
    if (pushed) {
      back = front = pushed;
      back->drawInto();
      flush(true);
      pushed = NULL;
    }
    break;
  case 'W': // select write/back screen
    selected = selectScreen(p);
    if (selected) {
      back = selected;
    }
    break;
  case 'w': // revert write/back to front screen
    back = pushed != NULL ? pushed : front;
    break;
  case 'D': // select display/front screen
    selected = selectScreen(p);
    if (selected) {
      front = selected;
      front->dirty = true;
      flush(true);
    }
    break;
  case 'd': // revert display/front to back screen
    front = back;
    front->dirty = true;
    flush(true);
    break;
  default:
    print("ERR unknown screen command");
    break;
  }    
}

// select the specified screen - returns whether the screen was changed
Screen *AnsiWidget::selectScreen(char *&p) {
  Vector<String *> *items = getItems(p);
  int n = items->size() > 0 ? atoi((*items)[0]->c_str()) : 0;
  int x = items->size() > 1 ? atoi((*items)[1]->c_str()) : 0;
  int y = items->size() > 2 ? atoi((*items)[2]->c_str()) : 0;
  int w = items->size() > 3 ? atoi((*items)[3]->c_str()) : width;
  int h = items->size() > 4 ? atoi((*items)[4]->c_str()) : height;

  Screen *result = NULL;
  flush(true);

  if (n < 0 || n >= MAX_SCREENS) {
    print("ERR invalid screen number");
  } else if (screens[n] != NULL) {
    // specified screen already exists
    result = screens[n];
  } else {
    if (n > 1) {
      result = new TextScreen(x, y, w, h, fontSize);
    } else {
      result = new GraphicScreen(x, y, w, h, fontSize);
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
      front = new GraphicScreen(0, 0, width, height, fontSize);
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
