// $Id: $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#include <QLabel>
#include <QPainter>
#include <QColor>

class AnsiWidget : public QLabel
{
  Q_OBJECT
public:
  explicit AnsiWidget(QWidget *parent = 0);
  ~AnsiWidget();

  // public api
  QRgb getPixel(int x, int y);
  int getFontSize();
  int getHeight() {return height();}
  int getWidth()  {return width();}
  int getX() {return curX;}
  int getY() {return curY;}
  int textHeight(void);
  int textWidth(const char* s);
  void beep() const;
  void clearScreen();
  void drawImage(QImage* img, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  void print(const char *str);
  void saveImage(const char* fn, int x, int y, int w, int h);
  void setColor(long color);
  void setFontSize(float i);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) {curX=x; curY=y;}

signals:

public slots:

private:
  bool doEscape(unsigned char *&p);
  bool setGraphicsRendition(char c, int escValue);
  int  calcTab(int x) const;
  QColor ansiToQt(long color);
  void destroyImage();
  void init();
  void newLine();
  void redraw();
  void reset();
  void updateFont();

  QPixmap* img;
  QColor bg;
  QColor fg;

  bool underline;
  bool invert;
  bool bold;
  bool italic;
  int curY;
  int curX;
  int curYSaved;
  int curXSaved;
  int tabSize;
  int textSize;
};

#endif // ANSIWIDGET_H
