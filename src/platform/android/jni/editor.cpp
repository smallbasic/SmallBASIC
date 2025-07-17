// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "platform/android/jni/runtime.h"
#include "common/device.h"
#include "ui/textedit.h"
#include "ui/keypad.h"

// whether to hide the status message
bool statusEnabled = true;

struct StatusMessage {
  explicit StatusMessage(const TextEditInput *editor, String &loadPath) :
    _dirty(!editor->isDirty()),
    _find(false),
    _scroll(-1),
    _row(editor->getRow()),
    _col(editor->getCol()) {
    int i = loadPath.lastIndexOf('/', 0);
    if (i != -1) {
      _fileName = loadPath.substring(i + 1);
    } else {
      _fileName = loadPath;
    }
  }

  void resetCursor(const TextEditInput *editor) {
    _row = editor->getRow();
    _col = editor->getCol();
    _scroll = editor->getScroll();
  }

  void toggleStatus(const TextEditInput *editor) {
    statusEnabled = !statusEnabled;
    setDirty(editor);
  }

  void setDirty(const TextEditInput *editor) {
    _dirty = !editor->isDirty();
  }

  void setFind(bool find, const TextEditInput *editor) {
    if (_find != find) {
      _find = find;
      setDirty(editor);
    }
  }

  bool update(TextEditInput *editor, const AnsiWidget *out) {
    bool result;
    bool dirty = editor->isDirty();
    if (_dirty != dirty
        || _scroll != editor->getScroll()
        || _row != editor->getRow()
        || _col != editor->getCol()) {
      if (statusEnabled) {
        setMessage(editor, out, dirty);
      } else {
        out->setStatus("");
      }
      resetCursor(editor);
      result = true;
    } else {
      result = false;
    }
    _dirty = dirty;
    return result;
  }

  void setMessage(TextEditInput *editor, const AnsiWidget *out, bool dirty) const {
    String message;
    if (_find) {
      message.append(" Search ");
    } else {
      if (dirty) {
        message.append(" * ");
      } else {
        message.append(" - ");
      }
      message.append(_fileName);
    }
    message.append(" (")
      .append(editor->getRow())
      .append(",")
      .append(editor->getCol())
      .append(") ");
    if (!editor->getScroll()) {
      message.append("Top");
    } else if (editor->getLines() - editor->getScroll() < editor->getPageRows()) {
      message.append("Bot");
    } else {
      const int pos = editor->getRow() * 100 / editor->getLines();
      message.append(pos).append("%");
    }
    out->setStatus(message);
  }

private:
  bool _dirty;
  bool _find;
  int _scroll;
  int _row;
  int _col;
  String _fileName;
};

void showFind(TextEditHelpWidget *helpWidget) {
  helpWidget->showPopup(35, 1);
  helpWidget->createSearch(false);
  helpWidget->setFocus(true);
}

void showHelp(TextEditHelpWidget *helpWidget) {
  helpWidget->showPopup(-4, -2);
  helpWidget->createKeywordIndex();
  helpWidget->setFocus(true);
}

void Runtime::editSource(strlib::String loadPath, bool restoreOnExit) {
  logEntered();

  showKeypad(false);
  int w = _output->getWidth();
  int h = _output->getHeight();
  int charWidth = _output->getCharWidth();
  int charHeight = _output->getCharHeight();
  int prevScreenId = _output->selectScreen(FORM_SCREEN);
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
  StatusMessage statusMessage(editWidget, loadPath);

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

  statusMessage.update(editWidget, _output);

  // layout inputs and redraw
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

  _srcRendered = false;
  _state = kEditState;

  while (_state == kEditState) {
    MAEvent event = getNextEvent();
    bool exitHelp = false;
    switch (event.type) {
    case EVENT_TYPE_POINTER_RELEASED:
    case EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED:
      if (widget == editWidget && statusMessage.update(editWidget, _output)) {
        _output->redraw();
      }
      break;
    case EVENT_TYPE_KEY_PRESSED:
      if (_userScreenId == -1) {
        dev_clrkb();
        bool redraw = true;
        char *text;

        switch (event.key) {
        case SB_KEY_F(2): case SB_KEY_F(3): case SB_KEY_F(4): case SB_KEY_F(5): case SB_KEY_F(6):
        case SB_KEY_F(7): case SB_KEY_F(8): case SB_KEY_F(10): case SB_KEY_F(11): case SB_KEY_F(12):
        case SB_KEY_MENU: case SB_KEY_ESCAPE: case SB_KEY_BREAK: case SB_KEY_CTRL('o'):
          // unhandled keys
          redraw = false;
          break;
        case SB_KEY_F(1):
          if (widget == helpWidget) {
            exitHelp = true;
          } else {
            widget = helpWidget;
            showHelp(helpWidget);
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
            statusMessage.setFind(false, editWidget);
          } else {
            widget = helpWidget;
            showFind(helpWidget);
            statusMessage.setFind(true, editWidget);
          }
          redraw = true;
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
        case SB_KEY_CTRL('t'):
          statusMessage.toggleStatus(editWidget);
          break;
        default:
          redraw = widget->edit(event.key, _output->getScreenWidth(), charWidth);
          break;
        }
        redraw |= statusMessage.update(editWidget, _output);
        if (redraw) {
          _output->redraw();
        }
      }
    }

    if ((exitHelp || isBack()) && widget == helpWidget) {
      widget = editWidget;
      helpWidget->hide();
      editWidget->setFocus(true);
      statusMessage.setFind(false, editWidget);
      statusMessage.update(editWidget, _output);
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
