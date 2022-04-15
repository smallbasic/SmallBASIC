// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

struct Canvas {
  Canvas();
  Canvas(int width, int height);
  virtual ~Canvas();

  bool create(int width, int height);
  void setClip(int x, int y, int w, int h);

  MARect *_clip;
  int _id;
  int _w;
  int _h;
};

struct Font {
  Font(int size, bool bold, bool italic);
  virtual ~Font();

  int _size;
  bool _bold;
  bool _italic;

  strlib::String _face;
};
