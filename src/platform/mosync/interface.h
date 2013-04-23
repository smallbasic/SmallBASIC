// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef INTERFACE_H
#define INTERFACE_H

struct IButtonListener {
  virtual ~IButtonListener() {}
  virtual void buttonClicked(const char *action) = 0;
};

struct IFormWidgetListModel {
  virtual ~IFormWidgetListModel() {}
  virtual const char *getTextAt(int index) = 0;
  virtual int getIndex(const char *label) = 0;
  virtual int rows() const = 0;
  virtual int selected() const = 0;
  virtual void selected(int index) = 0;
};

struct IFormWidget {
  virtual ~IFormWidget() {}
  virtual bool edit(int key) = 0;
  virtual IFormWidgetListModel *getList() const = 0;
  virtual const char *getText() const = 0;
  virtual void setText(const char *text) = 0;
  virtual void setListener(IButtonListener *listener) = 0;
  virtual int getX() = 0;
  virtual int getY() = 0;
  virtual int getW() = 0;
  virtual int getH() = 0;
  virtual void setX(int x) = 0;
  virtual void setY(int y) = 0;
  virtual void setW(int w) = 0;
  virtual void setH(int h) = 0;
};

#endif
