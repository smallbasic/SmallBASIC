// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef TIZEN_FORM_H
#define TIZEN_FORM_H

#include <FApp.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FUiITouchEventListener.h>
#include <FUiITextEventListener.h>
#include <FUiCtrlIFormBackEventListener.h>
#include <FUiCtrlIFormMenuEventListener.h>

#include "config.h"
#include "platform/tizen/display.h"

using namespace Tizen::Ui;
using namespace Tizen::Graphics;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui::Controls;

//
// TizenAppForm
//
class TizenAppForm :
  public Controls::Form,
  public IOrientationEventListener,
  public ITouchEventListener,
  public IFormBackEventListener,
  public IFormMenuEventListener {

public:
  TizenAppForm();
  virtual ~TizenAppForm();

  result Construct();
  bool isClosing() { return _state == kClosingState; }
  bool isStarting() { return _state == kInitState; }
  void exitSystem();
  void showKeypad();

private:
  result OnInitializing(void);
  void OnOrientationChanged(const Control &source,
      OrientationStatus orientationStatus);
  void OnTouchDoublePressed(const Control &source,
                            const Point &currentPosition,
                            const TouchEventInfo &touchInfo);
  void OnTouchFocusIn(const Control &source, 
                      const Point &currentPosition,
                      const TouchEventInfo &touchInfo);
  void OnTouchFocusOut(const Control &source,
                       const Point &currentPosition,
                       const TouchEventInfo &touchInfo);
  void OnTouchLongPressed(const Control &source,
                          const Point &currentPosition,
                          const TouchEventInfo &touchInfo);
  void OnTouchMoved(const Control &source,
                    const Point &currentPosition,
                    const TouchEventInfo &touchInfo);
  void OnTouchPressed(const Control &source,
                      const Point &currentPosition,
                      const TouchEventInfo &touchInfo);
  void OnTouchReleased(const Control &source,
                       const Point &currentPosition,
                       const TouchEventInfo &touchInfo);
  void OnFormBackRequested(Form &source);
  void OnFormMenuRequested(Form &source);
  void terminate();
  int  getTouchCount();

  // event handling
  DisplayWidget *_display;
  Runtime *_execThread;
  enum { kInitState, kActiveState, kClosingState, kDoneState, kErrorState } _state;
  enum { kLeftButton, kRightButtonOnce, kRightButton, kMoveOnly } _buttonState;
  enum { kControlMouse, kEscapeKey, kGameMenu, kShowKeypad } _shortcut;
};

#endif
