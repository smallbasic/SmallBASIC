// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2011 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QColor>

class AnsiWidget : public QWidget
{
  Q_OBJECT
public:
  explicit AnsiWidget(QWidget *parent = 0);
  ~AnsiWidget();

  // public api
  void beep() const;
  void clearScreen();
  void drawImage(QImage* img, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  QColor getBackgroundColor() {return bg;}
  QColor getColor() {return fg;}
  QRgb getPixel(int x, int y);
  int  getHeight() {return height();}
  int  getWidth()  {return width();}
  int  getX() {return curX;}
  int  getY() {return curY;}
  int  textHeight(void);
  int  textWidth(const char* s);
  void print(const char *str);
  void saveImage(const char* fn, int x, int y, int w, int h);
  void setColor(long color);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) {curX=x; curY=y;}

signals:

public slots:

private:
  QColor ansiToQt(long color);
  int  calcTab(int x) const;
  void destroyImage();
  bool doEscape(unsigned char *&p);
  void initImage();
  void newLine();
  void reset(bool init);
  bool setGraphicsRendition(char c, int escValue);
  void updateFont();

  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void paintEvent(QPaintEvent* event);
  void resizeEvent(QResizeEvent* event); 

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

  // clipboard handling
  int markX, markY, pointX, pointY;
};

#endif // ANSIWIDGET_H

// End of "$Id$".
