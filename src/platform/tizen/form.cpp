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

using namespace Tizen::App;
using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Base::Utility;
using namespace Tizen::Ui::Controls;

// block for up to 2.5 seconds during shutdown to
// allow the runtime thread to exit gracefully.
#define EXIT_SLEEP_STEP 10
#define EXIT_SLEEP 250

//
// converts a Tizen (wchar) String into a StringLib (char) string
//
String fromString(const Tizen::Base::String &in) {
  Tizen::Base::ByteBuffer *buf = StringUtil::StringToUtf8N(in);
  String result((const char*)buf->GetPointer());
  delete buf;
  return result;
}

//
// TizenAppForm
//
TizenAppForm::TizenAppForm() :
  _display(NULL),
  _runtime(NULL) {
}

TizenAppForm::~TizenAppForm() {
  logEntered();

  if (_runtime) {
    if (_runtime->isActive()) {
      // push an exit message onto the thread event queue
      _runtime->setExit(true);

      // block while thread ends
      AppLog("Waiting for runtime shutdown");
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

result TizenAppForm::Construct(int w, int h) {
  logEntered();
  result r = Form::Construct(FORM_STYLE_NORMAL);
  String resourcePath = fromString(App::GetInstance()->GetAppResourcePath());

  if (!IsFailed(r)) {
    _display = new FormViewable();
    if (_display && _display->Construct(resourcePath, w, h) == E_SUCCESS &&
        AddControl(_display) == E_SUCCESS) {
      SetOrientation(ORIENTATION_AUTOMATIC);
    } else {
      AppLog("Failed to create FormViewable");
      r = E_OUT_OF_MEMORY;
    }
  }

  if (!IsFailed(r)) {
    _runtime = new RuntimeThread(w, h);
    r = _runtime != NULL ? _runtime->Construct(resourcePath) : E_OUT_OF_MEMORY;
  }

  if (IsFailed(r)) {
    AppLog("Form startup failed");
    delete _display;
    delete _runtime;
    _runtime = NULL;
    _display = NULL;
  }
  return r;
}

result TizenAppForm::OnDraw() {
  logEntered();
  return _display->OnDraw();
}

void TizenAppForm::OnFormBackRequested(Form &source) {
  logEntered();
  _runtime->setBack();
}

void TizenAppForm::OnFormMenuRequested(Form &source) {
  logEntered();
}

result TizenAppForm::OnInitializing(void) {
  logEntered();

  AddOrientationEventListener(*this);
  SetFormBackEventListener(this);
  SetFormMenuEventListener(this);

  // set focus to enable receiving key events
  _display->AddTouchEventListener(*this);
  SetEnabled(true);
  SetFocusable(true);
  SetFocus();

  if (_runtime->isInitial()) {
    _runtime->Start();
  }

  logLeaving();
  return E_SUCCESS;
}

void TizenAppForm::OnOrientationChanged(const Control &source,
                                        OrientationStatus orientationStatus) {
  logEntered();
}

void TizenAppForm::OnTouchDoublePressed(const Control &source,
                                        const Point &currentPosition,
                                        const TouchEventInfo &touchInfo) {
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
}

void TizenAppForm::OnTouchMoved(const Control &source,
                                const Point &currentPosition,
                                const TouchEventInfo &touchInfo) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_POINTER_DRAGGED;
  maEvent.point.x = touchInfo.GetCurrentPosition().x;
  maEvent.point.y = touchInfo.GetCurrentPosition().y;
  _runtime->pushEvent(maEvent);
}

void TizenAppForm::OnTouchPressed(const Control &source,
                                  const Point &currentPosition,
                                  const TouchEventInfo &touchInfo) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_POINTER_PRESSED;
  maEvent.point.x = touchInfo.GetCurrentPosition().x;
  maEvent.point.y = touchInfo.GetCurrentPosition().y;
  _runtime->pushEvent(maEvent);
}

void TizenAppForm::OnTouchReleased(const Control &source,
                                   const Point &currentPosition,
                                   const TouchEventInfo &touchInfo) {
  MAEvent maEvent;
  maEvent.type = EVENT_TYPE_POINTER_RELEASED;
  maEvent.point.x = touchInfo.GetCurrentPosition().x;
  maEvent.point.y = touchInfo.GetCurrentPosition().y;
  _runtime->pushEvent(maEvent);
}

