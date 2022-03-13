// This file is part of SmallBASIC
//
// Copyright(C) 2001-2022 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include <emscripten.h>
#include <emscripten/val.h>

#include "include/osd.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/device.h"
#include "common/keymap.h"
#include "common/inet.h"
#include "common/pproc.h"
#include "lib/maapi.h"
#include "ui/utils.h"
#include "ui/audio.h"
#include "platform/emcc/runtime.h"
#include "platform/emcc/main_bas.h"

#define MAIN_BAS "__main_bas__"

Runtime *runtime;

Runtime::Runtime() :
  System() {
  logEntered();
  runtime = this;

  MAExtent screenSize = maGetScrSize();
  _output = new AnsiWidget(EXTENT_X(screenSize), EXTENT_Y(screenSize));
  _output->construct();
  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(11);

  //_eventQueue = new Stack<MAEvent *>();
  _state = kActiveState;
}

Runtime::~Runtime() {
  logEntered();
}

void Runtime::alert(const char *title, const char *message) {
}

int Runtime::ask(const char *title, const char *prompt, bool cancel) {
  int result = 0;
  return result;
}

void Runtime::browseFile(const char *url) {
}

void Runtime::enableCursor(bool enabled) {
}

char *Runtime::getClipboardText() {
  return nullptr;
}

char *Runtime::loadResource(const char *fileName) {
  logEntered();
  char *buffer = System::loadResource(fileName);
  if (buffer == nullptr && strcmp(fileName, MAIN_BAS) == 0) {
    buffer = (char *)malloc(main_bas_len + 1);
    memcpy(buffer, main_bas, main_bas_len);
    buffer[main_bas_len] = '\0';
  }
  return buffer;
}

void Runtime::optionsBox(StringList *items) {
}

MAEvent Runtime::processEvents(int waitFlag) {
  emscripten_sleep(10);

  MAEvent event;
  event.type = 0;
  event.key = 0;
  event.nativeKey = 0;

  if (keymap_kbhit()) {
    event.type = EVENT_TYPE_KEY_PRESSED;
    event.key = keymap_kbpeek();
  }
  return event;
}

void Runtime::runShell() {
  logEntered();

  audio_open();
  runMain(MAIN_BAS);
  audio_close();
  _state = kDoneState;
  logLeaving();
}

void Runtime::setClipboardText(const char *text) {
}

void Runtime::setWindowRect(int x, int y, int width, int height) {
}

void Runtime::setWindowTitle(const char *title) {
}

void Runtime::showCursor(CursorType cursorType) {
}

//
// ma event handling
//
int maGetEvent(MAEvent *event) {
  int result = 0;
  return result;
}

void maWait(int timeout) {

}

//
// System platform methods
//
void System::editSource(strlib::String loadPath, bool restoreOnExit) {
  // empty
}

bool System::getPen3() {
  return false;
}

int osd_devinit() {
  return 1;
}

int osd_devrestore() {
  return 1;
}
