// This file is part of SmallBASIC
//
// Copyright(C) 2001-2012 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef ANSIWIDGET_H
#define ANSIWIDGET_H

#include <QAbstractButton>
#include <QColor>
#include <QScrollBar>
#include <QWidget>

struct AnsiWidgetListener {
  virtual void mouseMoveEvent(bool down) = 0;
  virtual void mousePressEvent() = 0;
  virtual void mouseReleaseEvent() = 0;
  virtual void loadPath(QString path, bool showPath, bool setHistory) = 0;
};

class AnsiWidget : public QWidget {
  Q_OBJECT

public:
  explicit AnsiWidget(QWidget *parent = 0);
  ~AnsiWidget();

  // public api
  void beep() const;
  void clearScreen();
  void drawArc(int xc, int yc, double r, double start, double end, double aspect);
  void drawEllipse(int xc, int yc, int xr, int yr, double aspect, int fill);
  void drawImage(QImage *img, int x, int y, int sx, int sy, int w, int h);
  void drawLine(int x1, int y1, int x2, int y2);
  void drawRect(int x1, int y1, int x2, int y2);
  void drawRectFilled(int x1, int y1, int x2, int y2);
  QColor getBackgroundColor() { return bg; }
  QColor getColor() { return fg; }
  QRgb getPixel(int x, int y);
  int getHeight() { return height(); }
  int getWidth()  { return width(); }
  int getX() { return curX; }
  int getY() { return curY; }
  int textHeight(void);
  int textWidth(const char *s);
  void print(const char *str);
  void saveImage(const char *fn, int x, int y, int w, int h);
  void setColor(long color);
  void setPixel(int x, int y, int c);
  void setTextColor(long fg, long bg);
  void setXY(int x, int y) { curX=x; curY=y; }
  void setScrollSize(int scrollSize);

  // mouse support
  int getMouseX(bool down) { return down ? markX : pointX; }
  int getMouseY(bool down) { return down ? markY : pointY; }
  bool getMouseMode() { return mouseMode; }
  void resetMouse();
  void setMouseMode(bool mode);
  void setMouseListener(AnsiWidgetListener *ml) {
    listener = ml;
  }

public slots:
  void copySelection();
  void linkClicked();
  void findNextText();
  void findText();
  void selectAll();

private slots:
  void scrollChanged(int value);

private:
  QColor ansiToQt(long color);
  int calcTab(int x) const;
  void createLink(unsigned char *&p, bool execLink);
  void destroyImage();
  bool doEscape(unsigned char *&p);
  void newLine();
  void reset(bool init);
  bool setGraphicsRendition(char c, int escValue);
  void updateFont();

  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void showEvent(QShowEvent *event);

  QPixmap *img;
  QColor bg;
  QColor fg;
  QScrollBar *scrollbar;

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
  int scrollSize;

  // clipboard handling
  int markX, markY, pointX, pointY;
  bool copyMode;

  // mouse handling
  bool mouseMode;               // PEN ON/OFF
  AnsiWidgetListener *listener;
  QList < QAbstractButton *>hyperlinks;
};

#endif // ANSIWIDGET_H
