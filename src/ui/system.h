// This file is part of SmallBASIC
//
// Copyright(C) 2001-2017 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SYSTEM_H
#define SYSTEM_H

#include "config.h"
#include "ui/strlib.h"
#include "ui/ansiwidget.h"
#include "ui/textedit.h"

void create_func(var_p_t form, const char *name, method cb);
void reset_image_cache();

struct Cache : public strlib::Properties<String *> {
  Cache(int size) : Properties(size * 2), _index(0) {}
  void add(const char *key, const char *value);
  int _index;
};

struct System {
  System();
  virtual ~System();

  int getPen(int code);
  char *getText(char *dest, int maxSize);
  bool isActive() { return _state != kInitState && _state != kDoneState; }
  bool isBack() { return _state == kBackState; }
  bool isBreak() { return _state >= kBreakState; }
  bool isClosing() { return _state >= kClosingState; }
  bool isEditing() { return _state == kEditState; }
  bool isInitial() { return _state == kInitState; }
  bool isModal() { return _state == kModalState; }
  bool isRestart() { return _state == kRestartState; }
  bool isRunning() { return _state == kRunState || _state == kModalState; }
  bool isSystemScreen() { return _userScreenId != -1; }
  void logStack(const char *keyword, int type, int line);
  char *readSource(const char *fileName);
  void setBack();
  void setExit(bool quit);
  void setLoadBreak(const char *path);
  void setLoadPath(const char *path);
  void setRunning(bool running);
  void systemLog(const char *msg);
  void systemPrint(const char *msg, ...);
  AnsiWidget *getOutput() { return _output; }

  enum CursorType {
    kHand, kArrow, kIBeam
  };

  virtual void enableCursor(bool enabled) = 0;
  virtual void addShortcut(const char *path) = 0;
  virtual void alert(const char *title, const char *message) = 0;
  virtual int ask(const char *title, const char *prompt, bool cancel=true) = 0;
  virtual void browseFile(const char *url) = 0;
  virtual void debugStart(TextEditInput *edit, const char *file) = 0;
  virtual void debugStep(TextEditInput *edit, TextEditHelpWidget *help, bool cont) = 0;
  virtual void debugStop() = 0;
  virtual MAEvent processEvents(int waitFlag) = 0;
  virtual char *loadResource(const char *fileName);
  virtual void optionsBox(StringList *items) = 0;
  virtual void setWindowTitle(const char *title) = 0;
  virtual void share(const char *path) = 0;
  virtual void showCursor(CursorType cursorType) = 0;
  virtual void setClipboardText(const char *text) = 0;
  virtual char *getClipboardText() = 0;

protected:
  void editSource(strlib::String loadPath);
  bool execute(const char *bas);
  bool fileExists(strlib::String &path);
  MAEvent getNextEvent() { return processEvents(1); }
  uint32_t getModifiedTime();
  void handleEvent(MAEvent &event);
  void handleMenu(MAEvent &event);
  bool loadSource(const char *fileName);
  void resize();
  void runEdit(const char *startupBas);
  void runMain(const char *mainBasPath);
  void runOnce(const char *startupBas);
  void saveFile(TextEditInput *edit, strlib::String &path);
  void setupPath(String &loadpath);
  bool setParentPath();
  void setDimensions();
  void showCompletion(bool success);
  void printErrorLine();
  void printSource();
  void printSourceLine(char *text, int line, bool last);
  void setRestart();
  void showMenu();
  void showSystemScreen(bool showSrc);
  void waitForBack();
  AnsiWidget *_output;

  // platform static virtual
  bool getPen3();
  void completeKeyword(int index);

  enum {
    kInitState = 0,// thread not active
    kActiveState,  // thread activated
    kEditState,    // program editor is active
    kRunState,     // program is running
    kModalState,   // retrieving user input inside program
    kConnState,    // retrieving remote program source
    kBreakState,   // running program should abort
    kRestartState, // running program should restart
    kBackState,    // back button detected
    kClosingState, // thread is terminating
    kDoneState     // thread has terminated
  } _state;

  strlib::Stack<String *> _history;
  strlib::String _loadPath;
  strlib::String _activeFile;
  StackTrace _stackTrace;
  TextEditInput *_editor;
  Cache _cache;
  int _touchX;
  int _touchY;
  int _touchCurX;
  int _touchCurY;
  int _initialFontSize;
  int _fontScale;
  int _userScreenId;
  int *_systemMenu;
  bool _mainBas;
  bool _buttonPressed;
  bool _srcRendered;
  bool _menuActive;
  char *_programSrc;
  uint32_t _modifiedTime;
};

#endif

