// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "platform/tizen/form.h"
#include "platform/tizen/application.h"
#include "ui/utils.h"

Application *TizenApp::createInstance() {
  logEntered();
  return new TizenApp();
}

TizenApp::TizenApp() : _appForm(NULL) {
  logEntered();
}

TizenApp::~TizenApp() {
  logEntered();
}

bool TizenApp::OnAppInitialized(void) {
  logEntered();
  return true;
}

bool TizenApp::OnAppInitializing(AppRegistry &appRegistry) {
  logEntered();
  bool result = false;
  Frame *appFrame = new (std::nothrow) Frame();
  if (appFrame && appFrame->Construct() == E_SUCCESS &&
      AddFrame(*appFrame) == E_SUCCESS) {
    Rectangle rc = appFrame->GetBounds();
    _appForm = new (std::nothrow) AppForm();
    if (_appForm &&
        _appForm->Construct(rc.width, rc.height) == E_SUCCESS &&
        appFrame->AddControl(_appForm) == E_SUCCESS &&
        appFrame->SetCurrentForm(_appForm) == E_SUCCESS) {
      result = true;
    }
  }

  if (!result) {
    AppLog("Application startup failed");
      delete _appForm;
    _appForm = NULL;
  } else {
    appFrame->Invalidate(true);
  }
  return result;
}

bool TizenApp::OnAppTerminating(AppRegistry &appRegistry, bool forcedTermination) {
  logEntered();
  return true;
}

bool TizenApp::OnAppWillTerminate(void) {
  logEntered();
  return true;
}

void TizenApp::OnBackground(void) {
  logEntered();
  pauseRuntime(true);
}

void TizenApp::OnBatteryLevelChanged(BatteryLevel batteryLevel) {
  logEntered();
}

void TizenApp::OnForeground(void) {
  logEntered();
  pauseRuntime(false);
}

void TizenApp::OnLowMemory(void) {
  logEntered();
}

void TizenApp::OnScreenBrightnessChanged(int brightness) {
  logEntered();
}

void TizenApp::OnScreenOff(void) {
  logEntered();
}

void TizenApp::OnScreenOn(void) {
  logEntered();
}

void TizenApp::OnUserEventReceivedN(RequestId requestId, IList *args) {
  logEntered();
  switch (requestId) {
  case MSG_ID_REDRAW:
    _appForm->redraw();
    break;
  case USER_MESSAGE_EXIT:
    Terminate();
    break;
  case MSG_ID_SHOW_KEYPAD:
    _appForm->showKeypad();
    break;
  case MSG_ID_SHOW_MENU:
    _appForm->showMenu((ArrayList *)args);
    args->RemoveAll(true);
    delete args;
    break;
  case MSG_ID_SHOW_ALERT:
    _appForm->showAlert((ArrayList *)args);
    args->RemoveAll(true);
    delete args;
    break;
  }
}

void TizenApp::pauseRuntime(bool pause) {
  if (_appForm) {
  }
}
