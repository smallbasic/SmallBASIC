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
#include "platform/common/utils.h"

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

bool TizenApp::OnAppWillTerminate(void) {
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
    _appForm = new (std::nothrow) TizenAppForm();
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
  }
  return result;
}

bool TizenApp::OnAppTerminating(AppRegistry &appRegistry, bool forcedTermination) {
  logEntered();
  return true;
}

void TizenApp::OnUserEventReceivedN(RequestId requestId, IList *args) {
  logEntered();
  /*
    MessageBox messageBox;
    int modalResult;

    switch (requestId) {
    case USER_MESSAGE_EXIT:
    // normal program termination
    Terminate();
    break;

    case USER_MESSAGE_EXIT_ERR:
    // assertion failure termination
    Terminate();
    break;

    case USER_MESSAGE_EXIT_ERR_CONFIG:
    // the config file was corrupted
    messageBox.Construct(L"Config file corrupted",
    L"Settings have been reverted, please restart.", MSGBOX_STYLE_OK);
    messageBox.ShowAndWait(modalResult);
    Terminate();
    break;
    }
  */
}

void TizenApp::OnForeground(void) {
  logEntered();
  pauseRuntime(false);
}

void TizenApp::OnBackground(void) {
  logEntered();
  pauseRuntime(true);
}

void TizenApp::OnBatteryLevelChanged(BatteryLevel batteryLevel) {
  logEntered();
}

void TizenApp::OnLowMemory(void) {
  logEntered();
}

void TizenApp::OnScreenOn(void) {
  logEntered();
}

void TizenApp::OnScreenOff(void) {
  logEntered();
}

void TizenApp::OnScreenBrightnessChanged(int brightness) {
  logEntered();
}

void TizenApp::pauseRuntime(bool pause) {
  if (_appForm) {
    //    if (pause && g_engine && !g_engine->isPaused()) {
    //  _appForm->pushKey(Common::KEYCODE_SPACE);
    //}
    //if (g_system) {
    //    ((TizenSystem *)g_system)->setMute(pause);
    //  }
  }
}
