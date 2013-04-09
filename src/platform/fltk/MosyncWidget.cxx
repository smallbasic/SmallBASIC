// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <string.h>

#include "MosyncWidget.h"
#include "platform/mosync/ansiwidget.h"

MosyncWidget::MosyncWidget(int x, int y, int w, int h, int defsize) :
  Widget(x, y, w, h, 0) {

}

MosyncWidget::~MosyncWidget() {
}

void MosyncWidget::clearScreen() {

}

void MosyncWidget::print(const char *str) {

}

void MosyncWidget::drawLine(int x1, int y1, int x2, int y2) {

}

void MosyncWidget::drawRectFilled(int x1, int y1, int x2, int y2) {

}

void MosyncWidget::drawRect(int x1, int y1, int x2, int y2) {

}

void MosyncWidget::drawImage(fltk::Image *img, int x, int y, int sx, int sy, int w, int h) {

}

void MosyncWidget::saveImage(const char *fn, int x, int y, int w, int h) {

}

void MosyncWidget::setTextColor(long fg, long bg) {

}

void MosyncWidget::setColor(long color) {
  ;
}

int MosyncWidget::getX() {
  return 0;
}

int MosyncWidget::getY() {
  return 0;
}

void MosyncWidget::setPixel(int x, int y, int c) {

}

int MosyncWidget::getPixel(int x, int y) {
  return 0;
}

void MosyncWidget::setXY(int x, int y) {

}

int MosyncWidget::textWidth(const char *s) {
  return 0;
}

int MosyncWidget::textHeight(void) {
  return 0;
}

void MosyncWidget::setFontSize(float i) {

}

int MosyncWidget::getFontSize() {
  return 0;
}

void MosyncWidget::beep() const {

}

void MosyncWidget::resize(int x, int y, int w, int h) {

}

int get_text_width(char *s) {
  return 0;
}

int maFontDelete(MAHandle font) {
  return RES_FONT_OK;
}

int maSetColor(int rgb) {
  return rgb;
}

void maSetClipRect(int left, int top, int width, int height) {
}

void maPlot(int posX, int posY) {
}

void maLine(int startX, int startY, int endX, int endY) {
}

void maFillRect(int left, int top, int width, int height) {
}

void maDrawText(int left, int top, const char* str) {
}

void maUpdateScreen(void) {
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char* str) {
  return 0;
}

MAExtent maGetScrSize(void) {
  return 0;
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  return 0;
}

MAHandle maFontSetCurrent(MAHandle font) {
  return 0;
}

void maDrawImageRegion(MAHandle image, const MARect* srcRect, const MAPoint2d* dstPoint, int transformMode) {
}

int maCreateDrawableImage(MAHandle placeholder, int width, int height) {
  return 0;
}

MAHandle maCreatePlaceholder(void) {
  return 0;
}

void maDestroyPlaceholder(MAHandle handle) {
}

void maGetImageData(MAHandle image, void* dst, const MARect* srcRect, int scanlength) {
}

MAHandle maSetDrawTarget(MAHandle image) {
  return 0;
}

int maGetMilliSecondCount(void) {
  return 0;
}

int maShowVirtualKeyboard(void) {
  return 0;
}

void maOptionsBox(const wchar* title, const wchar* destructiveButtonTitle,
                  const wchar* cancelButtonTitle, const void* otherButtonTitles,
                  int otherButtonTitlesSize) {
}

int maGetEvent(MAEvent* event) {
  return 0;
}

void maWait(int timeout) {
}

void maAlert(const char* title, const char* message, const char* button1,
             const char* button2, const char* button3) {
}
