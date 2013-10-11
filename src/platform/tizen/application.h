// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef TIZEN_APPLICATION_H
#define TIZEN_APPLICATION_H

#include <FBase.h>
#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>

#include "platform/tizen/form.h"

using namespace Tizen::App;
using namespace Tizen::System;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::Base::Collection;

class TizenApp :
  public UiApp,
  public IScreenEventListener {

public:
  TizenApp();
  ~TizenApp();

  static UiApp *createInstance(void);

private:
  bool OnAppInitializing(AppRegistry &appRegistry);
  bool OnAppInitialized(void);
  bool OnAppWillTerminate(void);
  bool OnAppTerminating(AppRegistry &appRegistry, bool forcedTermination = false);
  void OnLowMemory(void);
  void OnBatteryLevelChanged(BatteryLevel batteryLevel);
  void OnUserEventReceivedN(RequestId requestId, IList *args);
  void OnForeground(void);
  void OnBackground(void);
  void OnScreenOn(void);
  void OnScreenOff(void);
  void OnScreenBrightnessChanged(int brightness);

  void pauseRuntime(bool pause);
  AppForm *_appForm;
};

#endif
