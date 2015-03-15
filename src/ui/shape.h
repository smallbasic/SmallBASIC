// This file is part of SmallBASIC
//
// Copyright(C) 2001-2014 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SHAPE_H
#define SHAPE_H

struct Shape {
  Shape(int x, int y, int w, int h) : _x(x), _y(y), _width(w), _height(h) {}
  virtual ~Shape() {}
  virtual void draw(int x, int y, int w, int h, int cw) {}

  int w() { return _width; }
  int h() { return _height; }
  int _x, _y, _width, _height;
};

#endif
