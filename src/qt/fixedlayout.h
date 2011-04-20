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

struct LineInput : public QLineEdit {
  LineInput(const QFont& font, int x, int y);
  ~LineInput() {};
  void keyPressEvent(QKeyEvent* event);
};

struct FixedLayout : public QLayout {
  FixedLayout(QWidget* parent);
  ~FixedLayout();
  
  void addItem(QLayoutItem* item);
  QSize sizeHint() const;
  int count() const;
  QLayoutItem* itemAt(int) const;
  QLayoutItem* takeAt(int);
  void setGeometry(const QRect &rect);
  
private:
  QList<QLayoutItem*> list;
};

#endif

// End of "$Id$".
