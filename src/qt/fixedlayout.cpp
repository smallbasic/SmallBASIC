// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#include "mainwindow.h"
#include "fixedlayout.h"

// change the layout of the lineinput upon text change
void LineInput::keyPressEvent(QKeyEvent* event) {
  QLineEdit::keyPressEvent(event);

  if (event->key() == Qt::Key_B && event->modifiers() & Qt::ControlModifier) {
    wnd->runBreak();
  }
  else if (event->key() == Qt::Key_Escape) {
    wnd->endModal();
  }
  else {
    QFontMetrics fm = fontMetrics();
    int width = fm.width(text() + "ZZ");
    resize(width, size().height());
  }
}

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

// QList::value() performs index checking, and returns 0 if we are 
// outside the valid range
QLayoutItem* FixedLayout::itemAt(int idx) const {
  return list.value(idx);
}

// QList::take does not do index checking
QLayoutItem* FixedLayout::takeAt(int idx) {
  return idx >= 0 && idx < list.size() ? list.takeAt(idx) : 0;
}

void FixedLayout::addItem(QLayoutItem *item) {
  list.append(item);
}

void FixedLayout::setGeometry(const QRect &r) {
  QLayout::setGeometry(r);
  
  if (list.size() != 0) {
    int w = r.width() - (list.count() - 1) * spacing();
    int h = r.height() - (list.count() - 1) * spacing();
    int i = 0;
    while (i < list.size()) {
      QLayoutItem *o = list.at(i);
      QRect geom(r.x() + i * spacing(), r.y() + i * spacing(), w, h);
      //      o->setGeometry(geom);
      ++i;
    }
  }
}

QSize FixedLayout::sizeHint() const {
  QSize s(0,0);
  int n = list.count();
  if (n > 0) {
    s = QSize(100,70); //start with a nice default size
  }
  int i = 0;
  while (i < n) {
    QLayoutItem *o = list.at(i);
    s = s.expandedTo(o->sizeHint());
    ++i;
  }
  return s + n*QSize(spacing(), spacing());
}

QSize FixedLayout::minimumSize() const {
  QSize s(0,0);
  int n = list.count();
  int i = 0;
  while (i < n) {
    QLayoutItem *o = list.at(i);
    s = s.expandedTo(o->minimumSize());
    ++i;
  }
  return s + n*QSize(spacing(), spacing());
}

// End of "$Id$".
