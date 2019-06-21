// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef DISPLAY_H
#define DISPLAY_H

#include <FL/fl_draw.H>
#include <FL/Fl_Rect.H>
#include <FL/Fl_Image_Surface.H>
#include "lib/maapi.h"

struct Font {
  Font(Fl_Font font, Fl_Fontsize size) :  _font(font),  _size(size) {}
  void setCurrent() { fl_font(_font, _size); }

private:
  Fl_Font _font;
  Fl_Fontsize _size;
};

struct Canvas {
  Canvas();
  virtual ~Canvas();

  bool create(int w, int h);
  void drawArc(int xc, int yc, double r, double start, double end, double aspect);
  void drawEllipse(int xc, int yc, int rx, int ry, bool fill);
  void drawLine(int startX, int startY, int endX, int endY);
  void drawPixel(int posX, int posY);
  void drawRGB(const MAPoint2d *dstPoint, const void *src, const MARect *srcRect, int opacity, int bytesPerLine);
  void drawRegion(Canvas *src, const MARect *srcRect, int dstx, int dsty);
  void drawText(Font *font, int left, int top, const char *str, int len);
  void fillRect(int x, int y, int w, int h, Fl_Color color);
  Fl_Color getDrawColor() { return _drawColor; }
  void getImageData(uint8_t *image, const MARect *srcRect, int bytesPerLine);
  void setClip(int x, int y, int w, int h);
  void setColor(Fl_Color color) { _drawColor = color; }

  int x() { return _clip ? _clip->x() : 0; }
  int y() { return _clip ? _clip->y() : 0; }
  int w() { return _clip ? _clip->w() : _w; }
  int h() { return _clip ? _clip->h() : _h; }

  int _w;
  int _h;
  float _scale;
  Fl_Offscreen _offscreen;
  Fl_Rect *_clip;
  Fl_Color _drawColor;
};

class GraphicsWidget : public Fl_Widget {
public:
  GraphicsWidget(int x, int y, int w, int h);
  virtual ~GraphicsWidget();

  void deleteFont(Font *font);
  void drawText(int left, int top, const char *str, int len);
  int getHeight() { return _screen->_h; }
  int getWidth() { return _screen->_w; }
  Canvas *getDrawTarget() { return _drawTarget; }
  Canvas *getScreen() { return _screen; }
  MAExtent getTextSize(const char *str);
  void layout();
  void resize(int x, int y, int w, int h);
  MAHandle setDrawTarget(MAHandle maHandle);
  void setFont(Font *font) { _font = font; }

private:
  void draw();

  Canvas *_screen;
  Canvas *_drawTarget;
  Font *_font;
  int _textOffset;
};

#endif
