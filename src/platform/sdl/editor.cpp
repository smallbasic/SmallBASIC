// This file is part of SmallBASIC
//
// Copyright(C) 2001-2015 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "common/sbapp.h"
#include "common/fs_socket_client.h"
#include "ui/textedit.h"
#include "platform/sdl/runtime.h"
#include "platform/sdl/settings.h"

using namespace strlib;

String g_exportAddr;
String g_exportToken;
int cursorPos;

void onlineHelp(Runtime *runtime, TextEditInput *widget) {
  char path[100];
  const char *nodeId = widget->getNodeId();
  if (nodeId != NULL && nodeId[0] != '0') {
    sprintf(path, "http://smallbasic.sf.net/?q=node/%s", nodeId);
  } else {
    char *selection = widget->getWordBeforeCursor();
    if (selection != NULL) {
      sprintf(path, "http://smallbasic.sf.net/?q=search/node/%s", selection);
      free(selection);
    } else {
      sprintf(path, "http://smallbasic.sf.net");
    }
  }
  runtime->browseFile(path);
}

void setupStatus(String &dirtyFile, String &cleanFile, String &loadPath) {
  const char *help = " Ctrl+h (C-h)=Help";
  String fileName;
  int i = loadPath.lastIndexOf('/', 0);
  if (i != -1) {
    fileName = loadPath.substring(i + 1);
  } else {
    fileName = loadPath;
  }
  dirtyFile.empty();
  dirtyFile.append(" * ");
  dirtyFile.append(fileName);
  dirtyFile.append(help);
  cleanFile.empty();
  cleanFile.append(" - ");
  cleanFile.append(fileName);
  cleanFile.append(help);
}

void showRecentFiles(TextEditHelpWidget *helpWidget, String &loadPath) {
  helpWidget->createMessage();
  helpWidget->show();
  helpWidget->reload(NULL);
  String fileList;
  getRecentFileList(fileList, loadPath);
  helpWidget->setText(fileList);
}

void exportBuffer(AnsiWidget *out, const char *text, String &dest, String &token) {
  char buffer[PATH_MAX];
  dev_file_t f;
  memset(&f, 0, sizeof(dev_file_t));

  sprintf(f.name, "SOCL:%s\n", dest.c_str());
  if (dest.indexOf(':', 0) != -1 && sockcl_open(&f)) {
    sprintf(buffer, "# %s\n", token.c_str());
    sockcl_write(&f, (byte *)buffer, strlen(buffer));
    if (!sockcl_write(&f, (byte *)text, strlen(text))) {
      sprintf(buffer, "Failed to write: %s", dest.c_str());
    } else {
      sprintf(buffer, "Exported file to %s", dest.c_str());
    }
    sockcl_close(&f);
  } else {
    sprintf(buffer, "Failed to open: %s", dest.c_str());
  }
  out->setStatus(buffer);
}

void System::editSource(String loadPath) {
  logEntered();

  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  int prevScreenId = _output->selectScreen(SOURCE_SCREEN);
  TextEditInput *editWidget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  TextEditHelpWidget *helpWidget = new TextEditHelpWidget(editWidget, charWidth, charHeight);
  TextEditInput *widget = editWidget;
  String dirtyFile;
  String cleanFile;
  String recentFile;
  enum InputMode {
    kInit, kExportAddr, kExportToken
  } inputMode = kInit;

  setupStatus(dirtyFile, cleanFile, loadPath);
  _modifiedTime = getModifiedTime();
  editWidget->updateUI(NULL, NULL);
  editWidget->setLineNumbers();
  editWidget->setFocus(true);
  editWidget->setCursorPos(cursorPos);
  cursorPos = 0;

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
  if (gsb_last_error && !isBack()) {
    _output->setStatus("Error. Esc=Close");
  } else {
    _output->setStatus(cleanFile);
  }
  _output->redraw();
  _state = kEditState;

  showCursor(kIBeam);
  setRecentFile(loadPath.c_str());
  setWindowTitle(loadPath);

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
        cursorPos = editWidget->getCursorPos();
        if (editWidget->isDirty()) {
          saveFile(editWidget, loadPath);
        }
        break;
      case SB_KEY_CTRL('s'):
        saveFile(editWidget, loadPath);
        break;
      case SB_KEY_CTRL('c'):
      case SB_KEY_CTRL('x'):
      case SB_KEY_CTRL(SB_KEY_INSERT):
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
      case SB_KEY_F(2):
        redraw = false;
        onlineHelp((Runtime *)this, editWidget);
        break;
      case SB_KEY_F(4):
        if (editWidget->getTextLength() && g_exportAddr.length() && g_exportToken.length()) {
          exportBuffer(_output, editWidget->getText(), g_exportAddr, g_exportToken);
          break;
        }
        // else fallthru to F3 handler
      case SB_KEY_F(3):
        if (editWidget->getTextLength()) {
          saveFile(editWidget, loadPath);
          _output->setStatus("Export to mobile SmallBASIC. Enter <IP>:<Port>");
          widget = helpWidget;
          helpWidget->createLineEdit(g_exportAddr);
          helpWidget->show();
          inputMode = kExportAddr;
        }
        break;
      case SB_KEY_F(5):
        saveFile(editWidget, loadPath);
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
      case SB_KEY_SHIFT(SB_KEY_INSERT):
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
      case SB_KEY_ALT('0'):
        _output->setStatus("Recent files. Esc=Close");
        widget = helpWidget;
        showRecentFiles(helpWidget, loadPath);
        break;
      case SB_KEY_ALT('1'):
      case SB_KEY_ALT('2'):
      case SB_KEY_ALT('3'):
      case SB_KEY_ALT('4'):
      case SB_KEY_ALT('5'):
      case SB_KEY_ALT('6'):
      case SB_KEY_ALT('7'):
      case SB_KEY_ALT('8'):
      case SB_KEY_ALT('9'):
        if (editWidget->isDirty()) {
          saveFile(editWidget, loadPath);
        }
        if (getRecentFile(recentFile, event.key - SB_KEY_ALT('1')) &&
            loadSource(recentFile.c_str())) {
          editWidget->reload(_programSrc);
          dirty = !editWidget->isDirty();
          setupStatus(dirtyFile, cleanFile, loadPath);
          setLoadPath(recentFile);
          setWindowTitle(recentFile);
          loadPath = recentFile;
          if (helpWidget->messageMode() && helpWidget->isVisible()) {
            showRecentFiles(helpWidget, loadPath);
          }
        } else {
          _output->setStatus("Failed to load recent file");
        }
        _modifiedTime = getModifiedTime();
        break;
      default:
        redraw = widget->edit(event.key, sw, charWidth);
        break;
      }
      if (event.key == SB_KEY_ENTER) {
        if (helpWidget->replaceMode()) {
          _output->setStatus("Replace string with. Esc=Close");
          dirty = editWidget->isDirty();
        } else if (helpWidget->lineEditMode() && helpWidget->getTextLength()) {
          switch (inputMode) {
          case kExportAddr:
            g_exportAddr = helpWidget->getText();
            inputMode = kExportToken;
            helpWidget->createLineEdit(g_exportToken);
            _output->setStatus("Enter token. Esc=Close");
            break;
          case kExportToken:
            g_exportToken = helpWidget->getText();
            inputMode = kInit;
            widget = editWidget;
            exportBuffer(_output, editWidget->getText(), g_exportAddr, g_exportToken);
            helpWidget->hide();
            break;
          default:
            break;
          }
          redraw = true;
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

      helpWidget->setFocus(widget == helpWidget);
      editWidget->setFocus(widget == editWidget);

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

