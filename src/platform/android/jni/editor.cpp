// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "ui/textedit.h"
#include "platform/android/jni/runtime.h"
#include "common/device.h"
#include "ui/keypad.h"

void showHelpLineInput(TextEditHelpWidget *helpWidget, int width = 35) {
  helpWidget->showPopup(width, 1);
}

void Runtime::editSource(strlib::String loadPath, bool restoreOnExit) {
  logEntered();

  strlib::String fileName;
  int i = loadPath.lastIndexOf('/', 0);
  if (i != -1) {
    fileName = loadPath.substring(i + 1);
  } else {
    fileName = loadPath;
  }

  strlib::String dirtyFile;
  dirtyFile.append(" * ");
  dirtyFile.append(fileName);
  strlib::String cleanFile;
  cleanFile.append(" - ");
  cleanFile.append(fileName);

  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  int prevScreenId = _output->selectScreen(SOURCE_SCREEN);
  TextEditInput *editWidget;
  if (_editor != nullptr) {
    editWidget = _editor;
    editWidget->_width = w;
    editWidget->_height = h;
  } else {
    editWidget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  }
  auto *helpWidget = new TextEditHelpWidget(editWidget, charWidth, charHeight, false);
  auto *widget = editWidget;
  _modifiedTime = getModifiedTime();
  editWidget->updateUI(nullptr, nullptr);
  editWidget->setLineNumbers();
  editWidget->setFocus(true);

  _output->clearScreen();
  _output->addInput(editWidget);
  _output->addInput(helpWidget);

  if (_keypad != nullptr) {
    _output->addInput(_keypad);
  } else {
    _keypad = new KeypadInput(false, false, charWidth, charHeight);
    _output->addInput(_keypad);
  }

  // to layout inputs
  _output->resize(w, h);

  if (gsb_last_line && isBreak()) {
    String msg = "Break at line: ";
    msg.append(gsb_last_line);
    alert(msg);
  } else if (gsb_last_error && !isBack()) {
    // program stopped with an error
    editWidget->setCursorRow(gsb_last_line + editWidget->getSelectionRow() - 1);
    alert(gsb_last_errmsg);
  }

  bool showStatus = !editWidget->getScroll();
  _srcRendered = false;
  _output->setStatus(showStatus ? cleanFile : "");
  _output->redraw();
  _state = kEditState;

  while (_state == kEditState) {
    MAEvent event = getNextEvent();
    bool exitHelp = false;
    switch (event.type) {
    case EVENT_TYPE_POINTER_PRESSED:
      if (!showStatus && widget == editWidget && event.point.x < editWidget->getMarginWidth()) {
        _output->setStatus(editWidget->isDirty() ? dirtyFile : cleanFile);
        _output->redraw();
        showStatus = true;
      }
      break;
    case EVENT_TYPE_POINTER_RELEASED:
      if (showStatus && event.point.x < editWidget->getMarginWidth() && editWidget->getScroll()) {
        _output->setStatus("");
        _output->redraw();
        showStatus = false;
      }
      break;
    case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
      if (editWidget->isDirty() && !editWidget->getScroll()) {
        _output->setStatus(dirtyFile);
        _output->redraw();
      }
      break;
    case EVENT_TYPE_KEY_PRESSED:
      if (_userScreenId == -1) {
        dev_clrkb();
        int sw = _output->getScreenWidth();
        bool redraw = true;
        bool dirty = editWidget->isDirty();
        char *text;

        switch (event.key) {
        case SB_KEY_F(2):
        case SB_KEY_F(3):
        case SB_KEY_F(4):
        case SB_KEY_F(5):
        case SB_KEY_F(6):
        case SB_KEY_F(7):
        case SB_KEY_F(8):
        case SB_KEY_F(10):
        case SB_KEY_F(11):
        case SB_KEY_F(12):
        case SB_KEY_MENU:
        case SB_KEY_ESCAPE:
        case SB_KEY_BREAK:
          // unhandled keys
          redraw = false;
          break;
        case SB_KEY_F(1):
          if (widget == helpWidget) {
            exitHelp = true;
          } else {
            widget = helpWidget;
            helpWidget->showPopup(-4, -2);
            helpWidget->createKeywordIndex();
            helpWidget->setFocus(true);
          }
          break;
        case SB_KEY_F(9):
        case SB_KEY_CTRL('r'):
          _state = kRunState;
          if (editWidget->isDirty()) {
            saveFile(editWidget, loadPath);
          }
          break;
        case SB_KEY_CTRL('f'):
          if (widget == helpWidget) {
            exitHelp = true;
          } else {
            widget = helpWidget;
            helpWidget->createSearch(false);
            showHelpLineInput(helpWidget);
          }
          break;
        case SB_KEY_CTRL('s'):
          saveFile(editWidget, loadPath);
          break;
        case SB_KEY_CTRL('c'):
        case SB_KEY_CTRL('x'):
          text = widget->copy(event.key == (int)SB_KEY_CTRL('x'));
          if (text) {
            setClipboardText(text);
            free(text);
          }
          break;
        case SB_KEY_CTRL('v'):
          text = getClipboardText();
          widget->paste(text);
          free(text);
          break;
        case SB_KEY_CTRL('o'):
          _output->selectScreen(USER_SCREEN1);
          showCompletion(true);
          _output->redraw();
          _state = kActiveState;
          waitForBack();
          _output->selectScreen(SOURCE_SCREEN);
          _state = kEditState;
          break;
        default:
          redraw = widget->edit(event.key, sw, charWidth);
          break;
        }
        if (editWidget->isDirty() != dirty && !editWidget->getScroll()) {
          _output->setStatus(editWidget->isDirty() ? dirtyFile : cleanFile);
        }
        if (redraw) {
          _output->redraw();
        }
      }
    }

    if ((exitHelp || isBack()) && widget == helpWidget) {
      widget = editWidget;
      helpWidget->hide();
      editWidget->setFocus(true);
      _state = kEditState;
      _output->redraw();
    }

    if (widget->isDirty()) {
      int choice = -1;
      if (isClosing()) {
        choice = 0;
      } else if (isBack()) {
        const char *message = "The current file has not been saved.\n"
                              "Would you like to save it now?";
        choice = ask("Save changes?", message, isBack());
      }
      if (choice == 0) {
        widget->save(loadPath);
      } else if (choice == 2) {
        // cancel
        _state = kEditState;
      }
    }
  }

  if (_state == kRunState) {
    // allow the editor to be restored on return
    if (!_output->removeInput(editWidget)) {
      trace("Failed to remove editor input");
    }
    if (!_output->removeInput(_keypad)) {
      trace("Failed to remove keypad input");
    }
    _editor = editWidget;
    _editor->setFocus(false);
  } else {
    _editor = nullptr;
    _keypad = nullptr;
  }

  // deletes editWidget and _keypad unless it has been removed
  _output->removeInputs();
  if (!isClosing() && restoreOnExit) {
    _output->selectScreen(prevScreenId);
  }
  logLeaving();
}
