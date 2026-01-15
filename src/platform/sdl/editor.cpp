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
#include "ui/keypad.h"
#include "platform/sdl/runtime.h"
#include "platform/sdl/settings.h"
#include "platform/sdl/syswm.h"

using namespace strlib;

#define MAX_MACRO 20

String g_exportAddr;
String g_exportToken;
int g_macro[MAX_MACRO];
int g_macro_size;
bool g_macro_record;
bool g_returnToLine;

struct StatusMessage {
  explicit StatusMessage(const TextEditInput *editor) :
    _dirty(editor->isDirty()),
    _insert(true),
    _row(editor->getRow()),
    _col(editor->getCol()) {
  }

  void setFilename(const String &loadPath) {
    int i = loadPath.lastIndexOf('/', 0);
    if (i != -1) {
      _fileName = loadPath.substring(i + 1);
    } else {
      _fileName = loadPath;
    }
  }

  bool update(TextEditInput *editor, const AnsiWidget *out, const bool force=false) {
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
        .append(") ");
      if (!_insert) {
        message.append("Ovwrt");
      }
      int digits = snprintf(nullptr, 0, "%d%d",
                            editor->getRow(), editor->getCol());
      int spaces = 6 - digits;
      for (int i = 0; i < spaces; i++) {
        message.append(' ');
      }
      if (!editor->getScroll()) {
        message.append("Top");
      } else if (editor->getLines() - editor->getScroll() < editor->getPageRows()) {
        message.append("Bot");
      } else {
        const int pos = editor->getRow() * 100 / editor->getLines();
        message.append(pos).append("%");
      }
      out->setStatus(message);
      _dirty = dirty;
      resetCursor(editor);
    } else {
      result = false;
    }
    return result;
  }

  void resetCursor(const TextEditInput *editor) {
    _row = editor->getRow();
    _col = editor->getCol();
  }

  void setDirty(const TextEditInput *editor) {
    _dirty = !editor->isDirty();
  }

  void setInsert() {
    _insert = !_insert;
  }

  bool _dirty;
  bool _insert;
  int _row;
  int _col;
  String _fileName;
};

void onlineHelp(Runtime *runtime, TextEditInput *widget) {
  char path[100];
  const char *nodeId = widget->getNodeId();
  if (nodeId != nullptr && nodeId[0] != '0') {
    sprintf(path, "http://smallbasic.github.io/reference/%s.html", nodeId);
  } else {
    sprintf(path, "https://smallbasic.github.io");
  }
  runtime->browseFile(path);
}

void showHelpPopup(TextEditHelpWidget *helpWidget) {
  helpWidget->showPopup(-8, -2);
}

void showHelpLineInput(TextEditHelpWidget *helpWidget, int width = 35) {
  helpWidget->showPopup(width, 1);
}

void showHelpSidebar(TextEditHelpWidget *helpWidget) {
  helpWidget->showSidebar();
}

void showRecentFiles(TextEditHelpWidget *helpWidget, String &loadPath) {
  String fileList;
  helpWidget->createMessage();
  helpWidget->reload(nullptr);
  getRecentFileList(fileList, loadPath);
  helpWidget->setText(fileList);
  showHelpPopup(helpWidget);
}

void showSelectionCount(const AnsiWidget *out, TextEditInput *widget) {
  int lines, chars;
  widget->getSelectionCounts(&lines, &chars);
  String label = "Region has ";
  label.append(lines).append(" line");
  if (lines > 1) {
    label.append("s, ");
  } else {
    label.append(", ");
  }
  label.append(chars).append(" character");
  if (chars > 1) {
    label.append("s.");
  } else {
    label.append(".");
  }
  out->setStatus(label);
}

void exportBuffer(const AnsiWidget *out, const char *text, const String &dest, const String &token) {
  char buffer[PATH_MAX];
  dev_file_t f = {};

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

bool externalExec(const AnsiWidget *out, const TextEditInput *editWidget, const String &loadPath) {
  bool result;
  if (editWidget->getTextLength() && !g_exportAddr.empty() && !g_exportToken.empty()) {
    exportBuffer(out, editWidget->getText(), g_exportAddr, g_exportToken);
    result = true;
  } else {
    result = false;
  }
  return result;
}

void Runtime::editSource(String loadPath, bool restoreOnExit) {
  logEntered();

  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  _output->selectScreen(FORM_SCREEN);

  TextEditInput *editWidget;
  if (_editor != nullptr) {
    editWidget = _editor;
    editWidget->_width = w;
    editWidget->_height = h;
  } else {
    editWidget = new TextEditInput(_programSrc, charWidth, charHeight, 0, 0, w, h);
  }

  auto *helpWidget = new TextEditHelpWidget(editWidget, charWidth, charHeight);
  TextEditInput *widget = editWidget;
  String recentFile;
  StatusMessage statusMessage(editWidget);

  enum InputMode {
    kInit, kExportAddr, kExportToken, kCommand
  } inputMode = kInit;

  _modifiedTime = getModifiedTime();
  editWidget->updateUI(nullptr, nullptr);
  editWidget->setLineNumbers();
  editWidget->setFocus(true);
  statusMessage.setFilename(loadPath);

  _output->clearScreen();
  _output->addInput(editWidget);
  _output->addInput(helpWidget);

  if (_keypad != nullptr) {
    _output->addInput(_keypad);
  } else {
    _keypad = new KeypadInput(w, false, true, charWidth, charHeight);
    _output->addInput(_keypad);
  }

  // to layout inputs
  _output->resize(w, h);

  if (isBreak() && g_returnToLine) {
    // break running program - position to last program line
    editWidget->setCursorRow(gsb_last_line);
    statusMessage.update(editWidget, _output, true);
  } else if (gsb_last_error && !isBack()) {
    if (gsb_last_errmsg[0]) {
      // trim any trailing new-line character
      char *nl = strrchr(gsb_last_errmsg, '\n');
      if (nl) {
        *nl = '\0';
      }
    }
    const String lastFile(gsb_last_file);
    if (lastFile.endsWith(".sbu")) {
      _output->setStatus(!gsb_last_errmsg[0] ? "Unit error" : gsb_last_errmsg);
    } else {
      // program stopped with an error
      editWidget->setCursorRow(gsb_last_line + editWidget->getSelectionRow() - 1);
      if (!_stackTrace.empty()) {
        helpWidget->setText(gsb_last_errmsg);
        helpWidget->createStackTrace(gsb_last_errmsg, gsb_last_line, _stackTrace);
        widget = helpWidget;
        showHelpSidebar(helpWidget);
        _output->setStatus("Error. Esc=Close, Up/Down=Caller");
      } else {
        _output->setStatus(!gsb_last_errmsg[0] ? "Error" : gsb_last_errmsg);
      }
    }
  } else if (editWidget->getErrorAtLine() != -1) {
    String message("Error at line: ");
    message.append(editWidget->getErrorAtLine());
    _output->setStatus(message);
    editWidget->setCursorRow(editWidget->getErrorAtLine() - 1);
    editWidget->setErrorAtLine(-1);
  } else {
    statusMessage.update(editWidget, _output, true);
  }

  _srcRendered = false;
  _stackTrace.removeAll();
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
        case SB_KEY_F(12):
        case SB_KEY_MENU:
          redraw = false;
          break;
        case SB_KEY_ESCAPE:
          widget = editWidget;
          helpWidget->hide();
          helpWidget->cancelMode();
          statusMessage.setDirty(editWidget);
          debugStop();
          break;
        case SB_KEY_CTRL('s'):
          saveFile(editWidget, loadPath);
          saveRecentPosition(loadPath, editWidget->getCursorPos());
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
          showHelpPopup(helpWidget);
          break;
        case SB_KEY_F(2):
          redraw = false;
          onlineHelp((Runtime *)this, editWidget);
          break;
        case SB_KEY_F(4):
          if (externalExec(_output, editWidget, loadPath)) {
            break;
          }
          // else fallthrough to F3 handler
        case SB_KEY_F(3):
          if (editWidget->getTextLength()) {
            saveFile(editWidget, loadPath);
            saveRecentPosition(loadPath, editWidget->getCursorPos());
            _output->setStatus("Export to SmallBASIC. Enter <IP>:<Port>");
            widget = helpWidget;
            helpWidget->createLineEdit(g_exportAddr);
            showHelpLineInput(helpWidget, 60);
            inputMode = kExportAddr;
          }
          break;
        case SB_KEY_F(5):
          saveFile(editWidget, loadPath);
          saveRecentPosition(loadPath, editWidget->getCursorPos());
          _output->setStatus("Debug. F6=Step, F7=Continue, Esc=Close");
          widget = helpWidget;
          helpWidget->createMessage();
          showHelpSidebar(helpWidget);
          debugStart(editWidget, loadPath.c_str());
          statusMessage.resetCursor(editWidget);
          break;
        case SB_KEY_F(6):
          debugStep(editWidget, helpWidget, false);
          break;
        case SB_KEY_F(7):
          debugStep(editWidget, helpWidget, true);
          break;
#if !defined(_FLATPAK)
        case SB_KEY_F(8):
          exportRun(loadPath);
          break;
#endif
        case SB_KEY_F(9):
        case SB_KEY_CTRL('r'):
          _state = kRunState;
          break;
        case SB_KEY_F(10):
          _output->setStatus("Enter program command line, Esc=Close");
          widget = helpWidget;
          helpWidget->createLineEdit(opt_command);
          showHelpLineInput(helpWidget);
          inputMode = kCommand;
          break;
        case SB_KEY_CTRL('h'):
          _output->setStatus("Keystroke help. Esc=Close");
          widget = helpWidget;
          helpWidget->createHelp();
          showHelpSidebar(helpWidget);
          break;
        case SB_KEY_CTRL('l'):
          _output->setStatus("Outline. Esc=Close");
          widget = helpWidget;
          helpWidget->createOutline();
          showHelpSidebar(helpWidget);
          break;
        case SB_KEY_CTRL('f'):
          _output->setStatus("Find in buffer. Esc=Close");
          widget = helpWidget;
          helpWidget->createSearch(false);
          showHelpLineInput(helpWidget);
          statusMessage.resetCursor(editWidget);
          break;
        case SB_KEY_CTRL('n'):
          _output->setStatus("Replace string. Esc=Close");
          widget = helpWidget;
          helpWidget->createSearch(true);
          showHelpLineInput(helpWidget);
          statusMessage.resetCursor(editWidget);
          break;
        case SB_KEY_ALT('g'):
          _output->setStatus("Goto line. Esc=Close");
          widget = helpWidget;
          helpWidget->createGotoLine();
          showHelpLineInput(helpWidget, 5);
          break;
        case SB_KEY_CTRL(' '):
          _output->setStatus("Auto-complete. Esc=Close");
          widget = helpWidget;
          helpWidget->createCompletionHelp();
          showHelpSidebar(helpWidget);
          break;
        case SB_KEY_INSERT:
          statusMessage.setInsert();
          widget->edit(event.key, sw, charWidth);
          statusMessage.update(editWidget, _output, true);
          redraw = true;
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
          _output->selectScreen(FORM_SCREEN);
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
        case SB_KEY_ALT('='):
          showSelectionCount(_output, editWidget);
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
              saveRecentPosition(loadPath, editWidget->getCursorPos());
              editWidget->reload(_programSrc);
              editWidget->setCursorPos(getRecentPosition(recentFile));
              statusMessage.setFilename(recentFile);
              statusMessage.update(editWidget, _output, true);
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
          if (helpWidget->replaceModeWith()) {
            _output->setStatus("Replace string with. Esc=Close");
          } else if (helpWidget->replaceMode()) {
            _output->setStatus("Replace string. Enter=replace, Space=skip, Esc=Close");
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
              strlcpy(opt_command, helpWidget->getText(), sizeof(opt_command));
              statusMessage._dirty = !editWidget->isDirty();
              inputMode = kInit;
              widget = editWidget;
              helpWidget->hide();
              break;
            default:
              break;
            }
          } else if (helpWidget->closeOnEnter() && helpWidget->isVisible()) {
            statusMessage._dirty = !editWidget->isDirty();
            widget = editWidget;
            helpWidget->hide();
            helpWidget->cancelMode();
          }
          redraw = true;
        } else if (helpWidget->replaceDoneMode()) {
          statusMessage._dirty = !editWidget->isDirty();
          widget = editWidget;
          helpWidget->hide();
          helpWidget->cancelMode();
        }
        helpWidget->setFocus(widget == helpWidget);
        editWidget->setFocus(widget == editWidget);
        if (helpWidget->searchMode()) {
          redraw = true;
        } else {
          redraw |= statusMessage.update(editWidget, _output);
        }
        if (redraw) {
          _output->redraw();
        }
      }
    }
    if ((isBack() || isClosing()) && editWidget->isDirty()) {
      const auto message = "The current file has not been saved.\nWould you like to save it now?";
      const int choice = ask("Save changes?", message, isBack());
      if (choice == 0) {
        saveFile(editWidget, loadPath);
        saveRecentPosition(loadPath, editWidget->getCursorPos());
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
  logLeaving();
}

