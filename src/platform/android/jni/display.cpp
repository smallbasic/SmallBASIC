// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <time.h>
#include "platform/android/jni/display.h"
#include "platform/common/form_ui.h"
#include "platform/common/utils.h"

#define SIZE_LIMIT 4
#define FONT_FACE_NAME "Envy Code R.ttf"

//
// form_ui implementation
//
void form_ui::optionsBox(StringList *items) {
}

//
// maapi implementation
//
int maFontDelete(MAHandle maHandle) {
//  if (maHandle != -1) {
//    Font *font = (Font *) maHandle;
//    if (font == activeFont) {
//      activeFont = NULL;
//    }
//    AppLog("Delete font %x", font);
//    delete font;
//  }
  return RES_FONT_OK;
}

int maSetColor(int c) {
  //  int r = (c >> 16) & 0xFF;
  //  int g = (c >> 8) & 0xFF;
  //  int b = (c) & 0xFF;
  //  drawColor = Color(r, g, b);
  //  drawColorRaw = c;
  return c;
}

void maSetClipRect(int left, int top, int width, int height) {
  //  if (drawTarget) {
  //    drawTarget->setClip(left, top, width, height);
  //  }
}

void maPlot(int posX, int posY) {
  //  if (drawTarget) {
  //    drawTarget->drawPixel(posX, posY);
  //  }
}

void maLine(int startX, int startY, int endX, int endY) {
  //  if (drawTarget) {
  //    drawTarget->drawLine(startX, startY, endX, endY);
  //  }
}

void maFillRect(int left, int top, int width, int height) {
  //  if (drawTarget) {
  //    drawTarget->drawRectFilled(left, top, width, height);
  //  }
}

void maDrawText(int left, int top, const char *str) {
  //  if (drawTarget && str && str[0]) {
  //    drawTarget->drawText(left, top, str);
  //  }
}

void maUpdateScreen(void) {
  //  Application::GetInstance()->SendUserEvent(MSG_ID_REDRAW, NULL);
}

void maResetBacklight(void) {
}

MAExtent maGetTextSize(const char *str) {
  MAExtent result;
  //  if (activeFont && str && str[0]) {
  //Dimension dim;
  //    activeFont->GetTextExtent(str, strlen(str), dim);
  //    result = (MAExtent)((dim.width << 16) + dim.height);
  //  } else {
  //    result = 0;
  //  }
  return result;
}

MAExtent maGetScrSize(void) {
  //  short width = widget->getWidth();
  //  short height = widget->getHeight();
  //return (MAExtent)((width << 16) + height);
  return 0;
}

MAHandle maFontLoadDefault(int type, int style, int size) {
  // return (MAHandle) widget->createFont(style, size);
  return 0;
}

MAHandle maFontSetCurrent(MAHandle maHandle) {
  //  activeFont = (Font *) maHandle;
  //  return maHandle;
  return 0;
}

void maDrawImageRegion(MAHandle maHandle, const MARect *srcRect,
                       const MAPoint2d *dstPoint, int transformMode) {
  //  Drawable *drawable = (Drawable *)maHandle;
  //  if (drawTarget != drawable) {
  //    drawable->drawImageRegion(drawTarget, dstPoint, srcRect);
  //  }
}

int maCreateDrawableImage(MAHandle maHandle, int width, int height) {
  int result = RES_OK;
  //  if (height > widget->GetHeight() * SIZE_LIMIT) {
  //    result -= 1;
  //  } else {
  //    Drawable *canvas = (Drawable *)maHandle;
  //    canvas->create(width, height);
  //  }
  return result;
}

MAHandle maCreatePlaceholder(void) {
  //  MAHandle maHandle = (MAHandle) new Drawable();
  //  return maHandle;
  return 0;
}

void maDestroyPlaceholder(MAHandle maHandle) {
  //  Drawable *holder = (Drawable *)maHandle;
  //  delete holder;
}

void maGetImageData(MAHandle maHandle, void *dst, const MARect *srcRect, int scanlength) {
  //  Drawable *holder = (Drawable *)maHandle;
  //  // maGetImageData is only used for getPixel()
  //  *((int *)dst) = holder->getPixel(srcRect->left, srcRect->top);
}

MAHandle maSetDrawTarget(MAHandle maHandle) {
  //  return widget->setDrawTarget(maHandle);
  return 0;
}

int maGetMilliSecondCount(void) {
  long long result, ticks = 0;
  //Tizen::System::SystemTime::GetTicks(ticks);
  //result = ticks - epoch;
  return result;
}

int maShowVirtualKeyboard(void) {
  //Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_KEYPAD, NULL);
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
//  ArrayList *args = new ArrayList();
//  args->Construct();
//  args->Add(new Tizen::Base::String(title));
//  args->Add(new Tizen::Base::String(message));
//  Application::GetInstance()->SendUserEvent(MSG_ID_SHOW_ALERT, args);
}

