// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"

#include "platform/mosync/ansiwidget.h"

#define EVENT_CHECK_EVERY 2000
#define EVENT_MAX_BURN_TIME 30
#define EVENT_PAUSE_TIME 400
#define EVENT_TYPE_EXIT_ANY 0
#define EVENT_WAIT_INFINITE -1
#define EVENT_WAIT_NONE 0

struct Controller : IButtonListener {
  Controller();
  virtual ~Controller();

  bool construct();
  const char *getLoadPath();
  int getPen(int code);
  char *getText(char *dest, int maxSize);
  int handleEvents(int waitFlag);
  void handleMenu(int menuId);
  MAEvent processEvents(int ms, int untilType = -1);
  char *readSource(const char *fileName);
  bool isExit() { return _runMode == exit_state; }
  bool isBack() { return _runMode == back_state; }
  bool isModal() { return _runMode == modal_state; }
  bool isBreak() { return _runMode == exit_state || _runMode == back_state; }
  bool isRunning() { return _runMode == run_state || _runMode == modal_state; }
  void setExit(bool quit);
  void setRunning(bool running = true);
  void showError();
  void showCompletion(bool success);
  void showMenu();
  void logPrint(const char *format, ...);
  void buttonClicked(const char *url);
  AnsiWidget *_output;

private:
  void handleKey(int key);
  char *readConnection(const char *url);
  void showSystemScreen(bool showSrc);

  enum ExecState {
    init_state,
    run_state,
    restart_state,
    modal_state,
    break_state,
    conn_state,
    back_state,
    exit_state
  };

  ExecState _runMode;
  int _lastEventTime;
  int _eventTicks;
  int _touchX;
  int _touchY;
  int _touchCurX;
  int _touchCurY;
  int _initialFontSize;
  int _fontScale;
  bool _mainBas;
  bool _systemMenu;
  bool _systemScreen;
  bool _drainError;
  String _loadPath;
  char *_programSrc;
};
