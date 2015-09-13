// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "common/sbapp.h"
#include "ui/system.h"
#include "ui/textedit.h"

#define SAVE_FILE()                \
  if (!editWidget->save(loadPath)) \
    alert("", "Failed to save file"); \
  else _modifiedTime = getModifiedTime();

void System::editSource(strlib::String &loadPath) {
  logEntered();

  strlib::String fileName;
  int i = loadPath.lastIndexOf('/', 0);
  if (i != -1) {
    fileName = loadPath.substring(i + 1);
  } else {
    fileName = loadPath;
  }

  const char *help = " Ctrl+h (C-h)=Help";
  strlib::String dirtyFile;
  dirtyFile.append(" * ");
  dirtyFile.append(fileName);
  dirtyFile.append(help);
  strlib::String cleanFile;
  cleanFile.append(" - ");
  cleanFile.append(fileName);
  cleanFile.append(help);

  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  int prevScreenId = _output->selectScreen(SOURCE_SCREEN);
  TextEditInput *editWidget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  TextEditHelpWidget *helpWidget = new TextEditHelpWidget(editWidget, charWidth, charHeight);
  TextEditInput *widget = editWidget;
  _modifiedTime = getModifiedTime();

  editWidget->updateUI(NULL, NULL);
  editWidget->setLineNumbers();
  editWidget->setFocus();
  if (strcmp(gsb_last_file, loadPath.c_str()) == 0) {
    editWidget->setCursorRow(gsb_last_line - 1);
  }
  if (gsb_last_error && !isBack()) {
    editWidget->setCursorRow(gsb_last_line - 1);
    helpWidget->setText(gsb_last_errmsg);
    widget = helpWidget;
    helpWidget->show();
  }
  _srcRendered = false;
  _output->clearScreen();
  _output->addInput(editWidget);
  _output->addInput(helpWidget);
  _output->setStatus(cleanFile);
  _output->redraw();
  _state = kEditState;

  maShowVirtualKeyboard();
  showCursor(kIBeam);

  while (_state == kEditState) {
    MAEvent event = getNextEvent();
    if (event.type == EVENT_TYPE_KEY_PRESSED && _userScreenId == -1) {
      dev_clrkb();
      int sw = _output->getScreenWidth();
      bool redraw = true;
      bool dirty = editWidget->isDirty();
      char *text;

      if (_modifiedTime != 0 && _modifiedTime != getModifiedTime()) {
        const char *msg = "Do you want to reload the file?";
        if (ask("File has changed on disk", msg, false) == 0) {
          loadSource(loadPath.c_str());
          editWidget->reload(_programSrc);
          dirty = !editWidget->isDirty();
        }
        _modifiedTime = getModifiedTime();
        event.key = 0;
      }

      switch (event.key) {
      case SB_KEY_F(2):
      case SB_KEY_F(3):
      case SB_KEY_F(8):
      case SB_KEY_F(10):
      case SB_KEY_F(11):
      case SB_KEY_F(12):
      case SB_KEY_MENU:
        redraw = false;
        break;
      case SB_KEY_ESCAPE:
        widget = editWidget;
        helpWidget->hide();
        dirty = !editWidget->isDirty();
        debugStop();
        break;
      case SB_KEY_F(9):
      case SB_KEY_CTRL('r'):
        _state = kRunState;
        if (editWidget->isDirty()) {
          SAVE_FILE();
        }
        break;
      case SB_KEY_CTRL('s'):
        SAVE_FILE();
        break;
      case SB_KEY_CTRL('c'):
      case SB_KEY_CTRL('x'):
        text = widget->copy(event.key == (int)SB_KEY_CTRL('x'));
        if (text) {
          setClipboardText(text);
          free(text);
        }
        break;
      case SB_KEY_F(1):
      case SB_KEY_ALT('h'):
        _output->setStatus("Keyword Help. Esc=Close");
        widget = helpWidget;
        helpWidget->createKeywordIndex();
        helpWidget->show();
        break;
      case SB_KEY_F(5):
        SAVE_FILE();
        _output->setStatus("Debug. F6=Step, F7=Continue, Esc=Close");
        widget = helpWidget;
        helpWidget->createMessage();
        helpWidget->show();
        debugStart(editWidget, loadPath.c_str());
        break;
      case SB_KEY_F(6):
        debugStep(editWidget, helpWidget, false);
        break;
      case SB_KEY_F(7):
        debugStep(editWidget, helpWidget, true);
        break;
      case SB_KEY_CTRL('h'):
        _output->setStatus("Keystroke help. Esc=Close");
        widget = helpWidget;
        helpWidget->createHelp();
        helpWidget->show();
        break;
      case SB_KEY_CTRL('l'):
        _output->setStatus("Outline. Esc=Close");
        widget = helpWidget;
        helpWidget->createOutline();
        helpWidget->show();
        break;
      case SB_KEY_CTRL('f'):
        _output->setStatus("Find in buffer. Esc=Close");
        widget = helpWidget;
        helpWidget->createSearch(false);
        helpWidget->show();
        break;
      case SB_KEY_CTRL('n'):
        _output->setStatus("Replace string. Esc=Close");
        widget = helpWidget;
        helpWidget->createSearch(true);
        helpWidget->show();
        break;
      case SB_KEY_ALT('g'):
        _output->setStatus("Goto line. Esc=Close");
        widget = helpWidget;
        helpWidget->createGotoLine();
        helpWidget->show();
        break;
      case SB_KEY_CTRL(' '):
        _output->setStatus("Auto-complete. Esc=Close");
        widget = helpWidget;
        helpWidget->createCompletionHelp();
        helpWidget->show();
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
      if (event.key == SB_KEY_ENTER) {
        if (helpWidget->replaceMode()) {
          _output->setStatus("Replace string with. Esc=Close");
          dirty = editWidget->isDirty();
        } else if (helpWidget->closeOnEnter() && helpWidget->isVisible()) {
          if (helpWidget->replaceDoneMode()) {
            _output->setStatus(dirtyFile);
          }
          widget = editWidget;
          helpWidget->hide();
          redraw = true;
          dirty = !editWidget->isDirty();
        }
      }
      if (editWidget->isDirty() && !dirty) {
        _output->setStatus(dirtyFile);
      } else if (!editWidget->isDirty() && dirty) {
        _output->setStatus(cleanFile);
      }
      if (redraw) {
        _output->redraw();
      }
    }

    if ((isBack() || isClosing()) && editWidget->isDirty()) {
      const char *message = "The current file has not been saved.\n"
                            "Would you like to save it now?";
      int choice = ask("Save changes?", message, isBack());
      if (choice == 0) {
        if (!editWidget->save(loadPath)) {
          alert("", "Failed to save file");
        }
      } else if (choice == 2) {
        // cancel
        _state = kEditState;
      }
    }
  }

  _output->removeInputs();
  if (!isClosing()) {
    _output->selectScreen(prevScreenId);
  }
  logLeaving();
}

