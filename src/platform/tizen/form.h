// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef TIZEN_FORM_H
#define TIZEN_FORM_H

#include "config.h"
#include "platform/tizen/display.h"
#include "platform/tizen/runtime.h"

using namespace Tizen::Ui;
using namespace Tizen::Graphics;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui::Controls;

//
// AppForm
//
class AppForm :
  public Controls::Form,
  public IActionEventListener,
  public IFormBackEventListener,
  public IFormMenuEventListener,
  public IKeyEventListener,
  public IOrientationEventListener,
  public ITouchEventListener {

public:
  AppForm();
  virtual ~AppForm();
  result Construct(int w, int h);

  void showAlert(ArrayList *args);
  void showKeypad();
  void showMenu(ArrayList *args);
  void redraw() { _display->redraw(); }

private:
  void OnActionPerformed(const Control &source, int actionId);
  result OnDraw();
  void OnFormBackRequested(Form &source);
  void OnFormMenuRequested(Form &source);
  result OnInitializing(void);

  void OnKeyPressed(const Control &source, KeyCode keyCode);
  void OnKeyReleased(const Control &source, KeyCode keyCode) {};
  void OnKeyLongPressed(const Control &source, KeyCode keyCode) {}
  void OnOrientationChanged(const Control &source,
                            OrientationStatus orientationStatus);
  void OnTouchDoublePressed(const Control &source,
                            const Point &currentPosition,
                            const TouchEventInfo &touchInfo) {}
  void OnTouchFocusIn(const Control &source, 
                      const Point &currentPosition,
                      const TouchEventInfo &touchInfo) {}
  void OnTouchFocusOut(const Control &source,
                       const Point &currentPosition,
                       const TouchEventInfo &touchInfo) {}
  void OnTouchLongPressed(const Control &source,
                          const Point &currentPosition,
                          const TouchEventInfo &touchInfo) {}
  void OnTouchMoved(const Control &source,
                    const Point &currentPosition,
                    const TouchEventInfo &touchInfo);
  void OnTouchPressed(const Control &source,
                      const Point &currentPosition,
                      const TouchEventInfo &touchInfo);
  void OnTouchReleased(const Control &source,
                       const Point &currentPosition,
                       const TouchEventInfo &touchInfo);

  EditField *_editField;
  ContextMenu *_contextMenu;
  FormViewable *_display;
  RuntimeThread *_runtime;
};

#endif
