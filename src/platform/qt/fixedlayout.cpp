// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "mainwindow.h"
#include "fixedlayout.h"

// creates a new line input
LineInput::LineInput(const QFont &font, int x, int y) : QLineEdit() {
  setFont(font);
  QFontMetrics fm = fontMetrics();
  setGeometry(x, y, fm.width("ZZZ"), 8 + fm.ascent() + fm.descent());
}

// change the layout of the lineinput upon text change
void LineInput::keyPressEvent(QKeyEvent *event) {
  QLineEdit::keyPressEvent(event);

  if (event->key() == Qt::Key_B &&event->modifiers() &Qt::ControlModifier) {
    wnd->runBreak();
  } else if (event->key() == Qt::Key_Escape) {
    wnd->endModal();
  } else {
    QFontMetrics fm = fontMetrics();
    int width = fm.width(text() + "ZZ");
    if (width + pos().x() < parentWidget()->width()) {
      resize(width, size().height());
    }
  }
}

// creates the fixed layout
FixedLayout::FixedLayout(QWidget *parent) : 
  QLayout(parent) {
  // empty
}

// destroys the fixed layout
FixedLayout::~FixedLayout() {
  QLayoutItem *item;
  while ((item = takeAt(0))) {
    delete item;
  }
}

// returns the number of QLayoutItems in the list
int FixedLayout::count() const {
  return list.size();
}

// QList::value() performs index checking, and returns 0 if we are // outside the valid range
QLayoutItem *FixedLayout::itemAt(int idx) const {
  return list.value(idx);
}

// QList::take does not do index checking
QLayoutItem *FixedLayout::takeAt(int idx) {
  return idx >= 0 &&idx < list.size() ? list.takeAt(idx) : 0;
}

void FixedLayout::addItem(QLayoutItem *item) {
  list.append(item);
}

void FixedLayout::setGeometry(const QRect &r) {
  QLayout::setGeometry(r);
}

QSize FixedLayout::sizeHint() const {
  return parentWidget()->size();
}
