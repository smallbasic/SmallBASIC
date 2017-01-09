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

#define MAX_MACRO 20

String g_exportAddr;
String g_exportToken;
int g_macro[MAX_MACRO];
int g_macro_size;
bool g_macro_record;
bool g_returnToLine;

struct StatusMessage {
  StatusMessage(TextEditInput *editor) :
    _dirty(editor->isDirty()),
    _row(editor->getRow()),
    _col(editor->getCol()) {
  }

  void setFilename(String &loadPath) {
    int i = loadPath.lastIndexOf('/', 0);
    if (i != -1) {
      _fileName = loadPath.substring(i + 1);
    } else {
      _fileName = loadPath;
    }
  }

  bool update(TextEditInput *editor, AnsiWidget *out, bool force=false) {
    bool result;
    bool dirty = editor->isDirty();
    if (force
        || _dirty != dirty
        || _row != editor->getRow()
        || _col != editor->getCol()) {
      String message;
      result = true;
      if (dirty) {
        message.append(" * ");
      } else {
        message.append(" - ");
      }
      message.append(_fileName)
        .append(" Ctrl+h (C-h)=Help (")
        .append(editor->getRow())
        .append(",")
        .append(editor->getCol())
        .append(")");
      int digits = snprintf(NULL, 0, "%d%d",
                            editor->getRow(), editor->getCol());
      int spaces = 4 - digits;
      for (int i = 0; i < spaces; i++) {
        message.append(' ');
      }
      out->setStatus(message);
      _dirty = dirty;
      _row = editor->getRow();
      _col = editor->getCol();
    } else {
      result = false;
    }
    return result;
  }

  bool _dirty;
  int _row;
  int _col;
  String _fileName;
};

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

void showRecentFiles(TextEditHelpWidget *helpWidget, String &loadPath) {
  String fileList;
  helpWidget->createMessage();
  helpWidget->show();
  helpWidget->reload(NULL);
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
  TextEditInput *editWidget;
  if (_editor != NULL) {
    editWidget = _editor;
    editWidget->_width = w;
    editWidget->_height = h;
  } else {
    editWidget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  }
  TextEditHelpWidget *helpWidget = new TextEditHelpWidget(editWidget, charWidth, charHeight);
  TextEditInput *widget = editWidget;
  String recentFile;
  StatusMessage statusMessage(editWidget);
  enum InputMode {
    kInit, kExportAddr, kExportToken, kCommand
  } inputMode = kInit;

  _modifiedTime = getModifiedTime();
  editWidget->updateUI(NULL, NULL);
  editWidget->setLineNumbers();
  editWidget->setFocus(true);
  statusMessage.setFilename(loadPath);

  if (isBreak() && g_returnToLine) {
    editWidget->setCursorRow(gsb_last_line);
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
  if (gsb_last_error && !isBack()) {
    _output->setStatus("Error. Esc=Close");
  } else {
    statusMessage.update(editWidget, _output, true);
  }
  _output->redraw();
  _state = kEditState;

  showCursor(kIBeam);
  setRecentFile(loadPath.c_str());
  setWindowTitle(statusMessage._fileName);

  while (_state == kEditState) {
    MAEvent event = getNextEvent();
    switch (event.type) {
    case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
      if (statusMessage.update(editWidget, _output)) {
        _output->redraw();
      }
      break;
    case EVENT_TYPE_KEY_PRESSED:
      if (_userScreenId == -1) {
        dev_clrkb();
        int sw = _output->getScreenWidth();
        bool redraw = true;
        char *text;

        if (_modifiedTime != 0 && _modifiedTime != getModifiedTime()) {
          const char *msg = "Do you want to reload the file?";
          if (ask("File has changed on disk", msg, false) == 0) {
            loadSource(loadPath.c_str());
            editWidget->reload(_programSrc);
          }
          _modifiedTime = getModifiedTime();
          event.key = 0;
        }

        switch (event.key) {
        case SB_KEY_CTRL('5'):
          _output->setStatus("Recording keyboard macro");
          g_macro_record = true;
          g_macro_size = 0;
          break;
        case SB_KEY_CTRL('6'):
          g_macro_record = false;
          break;
        case SB_KEY_CTRL('7'):
          g_macro_record = false;
          for (int i = 0; i < g_macro_size; i++) {
            redraw |= widget->edit(g_macro[i], sw, charWidth);
          }
          break;
        case SB_KEY_F(8):
        case SB_KEY_F(11):
        case SB_KEY_F(12):
        case SB_KEY_MENU:
          redraw = false;
        break;
        case SB_KEY_ESCAPE:
          widget = editWidget;
          helpWidget->hide();
          debugStop();
          break;
        case SB_KEY_F(9):
        case SB_KEY_CTRL('r'):
          _state = kRunState;
        break;
        case SB_KEY_F(10):
          _output->setStatus("Enter program command line, Esc=Close");
          widget = helpWidget;
          helpWidget->createLineEdit(opt_command);
          helpWidget->show();
          inputMode = kCommand;
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
          _output->setStatus("Keyword Help. F2=online, Esc=Close");
        widget = helpWidget;
        helpWidget->createKeywordIndex();
        helpWidget->show();
        break;
        case SB_KEY_F(2):
          redraw = false;
          onlineHelp((Runtime *)this, editWidget);
          break;
        case SB_KEY_F(4):
          if (editWidget->getTextLength() && !g_exportAddr.empty() && !g_exportToken.empty()) {
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
        case SB_KEY_ALT('.'):
          g_returnToLine = !g_returnToLine;
          _output->setStatus(g_returnToLine ?
                             "Position the cursor to the last program line after BREAK" :
                             "BREAK restores current cursor position");
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
        if (getRecentFile(recentFile, event.key - SB_KEY_ALT('1'))) {
          if (loadSource(recentFile.c_str())) {
            editWidget->reload(_programSrc);
            statusMessage.setFilename(loadPath);
            setLoadPath(recentFile);
            setWindowTitle(statusMessage._fileName);
            loadPath = recentFile;
            if (helpWidget->messageMode() && helpWidget->isVisible()) {
              showRecentFiles(helpWidget, loadPath);
            }
          } else {
            String message("Failed to load recent file: ");
            message.append(recentFile);
            _output->setStatus(message);
          }
        }
        _modifiedTime = getModifiedTime();
        break;
        default:
          redraw = widget->edit(event.key, sw, charWidth);
          break;
        }
        if (g_macro_record && g_macro_size < MAX_MACRO &&
            event.key != (int)SB_KEY_CTRL('5')) {
          g_macro[g_macro_size++] = event.key;
        }
        if (event.key == SB_KEY_ENTER) {
          if (helpWidget->replaceMode()) {
            _output->setStatus("Replace string with. Esc=Close");
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
            case kCommand:
              strcpy(opt_command, helpWidget->getText());
              inputMode = kInit;
              widget = editWidget;
              helpWidget->hide();
              break;
            default:
              break;
            }
          } else if (helpWidget->closeOnEnter() && helpWidget->isVisible()) {
            widget = editWidget;
            helpWidget->hide();
          }
          redraw = true;
        }

        helpWidget->setFocus(widget == helpWidget);
        editWidget->setFocus(widget == editWidget);
        redraw |= statusMessage.update(editWidget, _output);
        if (redraw) {
          _output->redraw();
        }
      }
    }
    if ((isBack() || isClosing()) && editWidget->isDirty()) {
      const char *message = "The current file has not been saved.\n"
                            "Would you like to save it now?";
      int choice = ask("Save changes?", message, isBack());
      if (choice == 0) {
        saveFile(editWidget, loadPath);
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
    _editor = editWidget;
    _editor->setFocus(false);
  } else {
    _editor = NULL;
  }

  _output->removeInputs();
  if (!isClosing()) {
    _output->selectScreen(prevScreenId);
  }
  logLeaving();
}

