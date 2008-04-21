// $Id: MainWindow.h,v 1.26 2007-05-31 11:03:16 zeeb90au Exp $
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
// 

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <fltk/Window.h>
#include <fltk/TabGroup.h>
#include <fltk/ValueInput.h>
#include <fltk/AnsiWidget.h>

#include "EditorWindow.h"
#include "HelpWidget.h"

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

#ifndef max
#define max(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef min
#define min(a,b) ((a>b) ? (b) : (a))
#endif

extern "C" void trace(const char* format, ...);

struct BaseWindow : public Window {
  BaseWindow(int w, int h) : Window(w, h, "SmallBASIC") {}
  virtual ~BaseWindow() {};
  int handle(int e);

  int penDownX;
  int penDownY;
  int penState;                 // PUSH/RELEASE events
  int penMode;                  // PEN ON/OFF
};

struct MainWindow : public BaseWindow {
  MainWindow(int w, int h);
  virtual ~MainWindow() {};

  bool isBreakExec(void);
  bool isModal();
  bool isEdit();
  void setModal(bool modal);
  void setBreak();
  void resetPen();
  void execLink(const char* file);

  bool isTurbo;
  bool isFullScreen;
  String siteHome;

  // main output
  AnsiWidget* out;
  EditorWindow* editWnd;
  HelpWidget* helpWnd;

  // tabs
  TabGroup* tabGroup;
  Group* editGroup;
  Group* outputGroup;
  Group* helpGroup;

  // tool-bar
  Input* findText;
  Input* gotoLine;
  Choice* funcList;

  // status bar
  Widget* fileStatus;
  Widget* rowStatus;
  Widget* colStatus;
  Widget* runStatus;
  Widget* modStatus;
};

#endif
