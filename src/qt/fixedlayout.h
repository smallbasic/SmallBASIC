// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef FIXEDLAYOUT_H
#define FIXEDLAYOUT_H

#include <QLayout>
#include <QLayoutItem>
#include <QLineEdit>

class LineInput : public QLineEdit {
public:
  LineInput() : QLineEdit() {}
  void keyPressEvent(QKeyEvent* event);
};

class FixedLayout : public QLayout {
public:
  FixedLayout(QWidget* parent) : QLayout(parent) {}
  ~FixedLayout();
  
  void addItem(QLayoutItem* item);
  QSize sizeHint() const;
  QSize minimumSize() const;
  int count() const;
  QLayoutItem* itemAt(int) const;
  QLayoutItem* takeAt(int);
  void setGeometry(const QRect &rect);
  
private:
  QList<QLayoutItem*> list;
};

#endif

// End of "$Id$".
