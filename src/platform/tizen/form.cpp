// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/tizen/form.h"
#include "platform/common/utils.h"

using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui::Controls;

// block for up to 2.5 seconds during shutdown to
// allow the runtime thread to exit gracefully.
#define EXIT_SLEEP_STEP 10
#define EXIT_SLEEP 250

//
// TizenAppForm
//
TizenAppForm::TizenAppForm() :
  _display(NULL),
  _runtime(NULL) {
}

result TizenAppForm::Construct() {
  logEntered();
  result r = Form::Construct(FORM_STYLE_NORMAL);

  if (!IsFailed(r)) {
    _display = new FormViewable();
    if (_display) {
      AddControl(_display);
      SetOrientation(ORIENTATION_AUTOMATIC);
      AddOrientationEventListener(*this);
    } else {
      r = E_OUT_OF_MEMORY;
    }
  }
  if (!IsFailed(r)) {
    Rectangle rc = GetClientAreaBounds();
    _runtime = new Runtime(rc.width, rc.height);
    r = _runtime != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;
  }
  if (!IsFailed(r)) {
    r = _runtime->Construct();
  }

  if (IsFailed(r)) {
    AppLog("Form startup failed");
    delete _display;
    delete _runtime;
    _runtime = NULL;
    _display = NULL;
  }

  logLeaving();
  return r;
}

TizenAppForm::~TizenAppForm() {
  logEntered();

  if (_runtime) {
    if (_runtime->isActive()) {
      // push an exit message onto the thread event queue
      _runtime->exitSystem();

      // block while thread ends
      AppLog("waiting for shutdown");
      for (int i = 0; i < EXIT_SLEEP_STEP && _runtime->isClosing(); i++) {
        Thread::Sleep(EXIT_SLEEP);
      }

      _runtime->Stop();
      _runtime->Join();
    }
    delete _runtime;
    _runtime = NULL;
  }

  logLeaving();
}

result TizenAppForm::OnInitializing(void) {
  logEntered();

  AddOrientationEventListener(*this);
  AddTouchEventListener(*this);
  SetMultipointTouchEnabled(true);
  SetFormBackEventListener(this);
  SetFormMenuEventListener(this);

  // set focus to enable receiving key events
  SetEnabled(true);
  SetFocusable(true);
  SetFocus();

  logLeaving();
  return E_SUCCESS;
}

void TizenAppForm::OnOrientationChanged(const Control &source,
                                        OrientationStatus orientationStatus) {
  logEntered();
  if (_runtime->isInitial()) {
    _runtime->Start();
  }
}

void TizenAppForm::OnTouchDoublePressed(const Control &source,
                                        const Point &currentPosition,
                                        const TouchEventInfo &touchInfo) {
  //pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
  //          currentPosition);
}

void TizenAppForm::OnTouchFocusIn(const Control &source,
                                  const Point &currentPosition,
                                  const TouchEventInfo &touchInfo) {
}

void TizenAppForm::OnTouchFocusOut(const Control &source,
                                   const Point &currentPosition,
                                   const TouchEventInfo &touchInfo) {
}

void TizenAppForm::OnTouchLongPressed(const Control &source,
                                      const Point &currentPosition,
                                      const TouchEventInfo &touchInfo) {
  logEntered();
  //pushKey(Common::KEYCODE_RETURN);
}

void TizenAppForm::OnTouchMoved(const Control &source,
                                const Point &currentPosition,
                                const TouchEventInfo &touchInfo) {
  //pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
}

void TizenAppForm::OnTouchPressed(const Control &source,
                                  const Point &currentPosition,
                                  const TouchEventInfo &touchInfo) {
  //pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
  //              currentPosition);
}

void TizenAppForm::OnTouchReleased(const Control &source,
                                   const Point &currentPosition,
                                   const TouchEventInfo &touchInfo) {
  //pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
  //          currentPosition);
  // flick to skip dialog
  if (touchInfo.IsFlicked()) {
    //pushKey(Common::KEYCODE_PERIOD);
  }
}

void TizenAppForm::OnFormBackRequested(Form &source) {
  logEntered();
}

void TizenAppForm::OnFormMenuRequested(Form &source) {
  logEntered();
}
