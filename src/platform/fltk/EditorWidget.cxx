// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <FL/fl_ask.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Tile.H>
#include "platform/fltk/MainWindow.h"
#include "platform/fltk/EditorWidget.h"
#include "platform/fltk/FileWidget.h"
#include "common/smbas.h"

// in MainWindow.cxx
extern String recentPath[];
extern String recentLabel[];
extern int recentMenu[];
extern const char *historyFile;
extern const char *untitledFile;

// in BasicEditor.cxx
extern Fl_Text_Display::Style_Table_Entry styletable[];
extern Fl_Color defaultColor[];

int completionIndex = 0;

static bool rename_active = false;
const char scanLabel[] = "(Refresh)";

EditorWidget *get_editor() {
  EditorWidget *result = wnd->getEditor();
  if (!result) {
    result = wnd->getEditor(true);
  }
  return result;
}

struct LineInput : public Fl_Input {
  LineInput(int x, int y, int w, int h);
  void resize(int x, int y, int w, int h);
  int handle(int event);

private:
  int orig_x, orig_y, orig_w, orig_h;
};

//--EditorWidget----------------------------------------------------------------

EditorWidget::EditorWidget(Fl_Widget *rect, Fl_Menu_Bar *menuBar) :
  Fl_Group(rect->x(), rect->y(), rect->w(), rect->h()),
  _editor(NULL),
  _tty(NULL),
  _dirty(false),
  _loading(false),
  _modifiedTime(0),
  _commandText(NULL),
  _rowStatus(NULL),
  _colStatus(NULL),
  _runStatus(NULL),
  _modStatus(NULL),
  _funcList(NULL),
  _funcListEvent(false),
  _logPrintBn(NULL),
  _lockBn(NULL),
  _hideIdeBn(NULL),
  _gotoLineBn(NULL),
  _commandOpt(cmd_find),
  _commandChoice(NULL),
  _menuBar(menuBar) {
  _filename[0] = 0;
  box(FL_NO_BOX);
  begin();

  const int st_w = 40;
  const int bn_w = 28;
  const int st_h = STATUS_HEIGHT;
  const int choice_w = 80;
  const int tileHeight = rect->h() - st_h;
  const int ttyHeight = tileHeight / 8;
  const int browserWidth = rect->w() / 5;
  const int editHeight = tileHeight - ttyHeight;
  const int editWidth = rect->w() - browserWidth;
  const int st_y = rect->y() + editHeight + ttyHeight;

  Fl_Group *tile = new Fl_Tile(rect->x(), rect->y(), rect->w(), tileHeight);
  _editor = new BasicEditor(rect->x(), rect->y(), editWidth, editHeight, this);
  _editor->linenumber_width(LINE_NUMBER_WIDTH);
  _editor->wrap_mode(true, 0);
  _editor->selection_color(fl_rgb_color(190, 189, 188));
  _editor->_textbuf->add_modify_callback(changed_cb, this);
  _editor->box(FL_FLAT_BOX);
  _editor->take_focus();

  // sub-func jump droplist
  _funcList = new Fl_Tree(_editor->w(), rect->y(), browserWidth, editHeight);
  _funcList->labelfont(FL_HELVETICA);
  _funcList->when(FL_WHEN_RELEASE);
  _funcList->box(FL_FLAT_BOX);

  Fl_Tree_Item *scan = new Fl_Tree_Item(_funcList);
  scan->label(scanLabel);
  _funcList->showroot(0);
  _funcList->add(scanLabel, scan);

  _tty = new TtyWidget(rect->x(), rect->y() + editHeight, rect->w(), ttyHeight, TTY_ROWS);
  tile->end();

  // editor status bar
  _statusBar = new Fl_Group(rect->x(), st_y, rect->w(), st_h);
  _logPrintBn = new Fl_Toggle_Button(rect->w() - bn_w, st_y + 1, bn_w, st_h - 2);
  _lockBn = new Fl_Toggle_Button(_logPrintBn->x() - (bn_w + 2), st_y + 1, bn_w, st_h - 2);
  _hideIdeBn = new Fl_Toggle_Button(_lockBn->x() - (bn_w + 2), st_y + 1, bn_w, st_h - 2);
  _gotoLineBn = new Fl_Toggle_Button(_hideIdeBn->x() - (bn_w + 2), st_y + 1, bn_w, st_h - 2);
  _colStatus = new Fl_Button(_gotoLineBn->x() - (st_w + 2), st_y, st_w, st_h);
  _rowStatus = new Fl_Button(_colStatus->x() - (st_w + 2), st_y, st_w, st_h);
  _runStatus = new Fl_Button(_rowStatus->x() - (st_w + 2), st_y, st_w, st_h);
  _modStatus = new Fl_Button(_runStatus->x() - (st_w + 2), st_y, st_w, st_h);
  _commandChoice = new Fl_Button(rect->x(), st_y, choice_w, st_h);
  _commandText = new Fl_Input(rect->x() + choice_w + 2, st_y + 1, _modStatus->x() - choice_w - 4, st_h - 2);
  _commandText->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
  _commandText->when(FL_WHEN_ENTER_KEY_ALWAYS);
  _commandText->labelfont(FL_HELVETICA);

  for (int n = 0; n < _statusBar->children(); n++) {
    Fl_Widget *w = _statusBar->child(n);
    w->labelfont(FL_HELVETICA);
    w->box(FL_NO_BOX);
  }
  _commandText->box(FL_THIN_DOWN_BOX);
  _logPrintBn->box(FL_THIN_UP_BOX);
  _lockBn->box(FL_THIN_UP_BOX);
  _hideIdeBn->box(FL_THIN_UP_BOX);
  _gotoLineBn->box(FL_THIN_UP_BOX);

  _statusBar->resizable(_commandText);
  _statusBar->end();
  resizable(tile);
  end();

  // command selection
  setCommand(cmd_find);
  runState(rs_ready);
  setModified(false);

  // button callbacks
  _lockBn->callback(scroll_lock_cb);
  _modStatus->callback(save_file_cb);
  _runStatus->callback(MainWindow::run_cb);
  _commandChoice->callback(command_cb, (void *)1);
  _commandText->callback(command_cb, (void *)1);
  _funcList->callback(func_list_cb, 0);
  _logPrintBn->callback(un_select_cb, (void *)_hideIdeBn);
  _hideIdeBn->callback(un_select_cb, (void *)_logPrintBn);
  _colStatus->callback(goto_line_cb, 0);
  _rowStatus->callback(goto_line_cb, 0);

  // setup icons
  _gotoLineBn->label("B");  // right arrow (goto)
  _hideIdeBn->label("W");   // large dot
  _lockBn->label("J");      // vertical bars
  _logPrintBn->label("T");  // italic bold T

  // setup tooltips
  _commandText->tooltip("Press Ctrl+f or Ctrl+Shift+f to find again");
  _rowStatus->tooltip("Cursor row position");
  _colStatus->tooltip("Cursor column position");
  _runStatus->tooltip("Run or BREAK");
  _modStatus->tooltip("Save file");
  _logPrintBn->tooltip("Display PRINT statements in the log window");
  _lockBn->tooltip("Prevent log window auto-scrolling");
  _hideIdeBn->tooltip("Hide the editor while program is running");
  _gotoLineBn->tooltip("Position the cursor to the last program line after BREAK");
  statusMsg(SB_STR_VER);

  // setup defaults or restore settings
  if (wnd && wnd->_profile) {
    wnd->_profile->loadConfig(this);
  }
  take_focus();
}

EditorWidget::~EditorWidget() {
  delete _editor;
}

//--Event handler methods-------------------------------------------------------

/**
 * change the selected text to upper/lower/camel case
 */
void EditorWidget::change_case(Fl_Widget *w, void *eventData) {
  Fl_Text_Buffer *tb = _editor->_textbuf;
  int start, end;
  char *selection = getSelection(&start, &end);
  int len = strlen(selection);
  enum { up, down, mixed } curcase = isupper(selection[0]) ? up : down;

  for (int i = 1; i < len; i++) {
    if (isalpha(selection[i])) {
      bool isup = isupper(selection[i]);
      if ((curcase == up && isup == false) || (curcase == down && isup)) {
        curcase = mixed;
        break;
      }
    }
  }

  // transform pattern: Foo -> FOO, FOO -> foo, foo -> Foo
  for (int i = 0; i < len; i++) {
    selection[i] = curcase == mixed ? toupper(selection[i]) : tolower(selection[i]);
  }
  if (curcase == down) {
    selection[0] = toupper(selection[0]);
    // upcase chars following non-alpha chars
    for (int i = 1; i < len; i++) {
      if (isalpha(selection[i]) == false && i + 1 < len) {
        selection[i + 1] = toupper(selection[i + 1]);
      }
    }
  }

  if (selection[0]) {
    tb->replace_selection(selection);
    tb->select(start, end);
  }
  free((void *)selection);
}

/**
 * command handler
 */
void EditorWidget::command_opt(Fl_Widget *w, void *eventData) {
  setCommand((CommandOpt) (intptr_t) eventData);
}

/**
 * cut selected text to the clipboard
 */
void EditorWidget::cut_text(Fl_Widget *w, void *eventData) {
  Fl_Text_Editor::kf_cut(0, _editor);
}

/**
 * delete selected text
 */
void EditorWidget::do_delete(Fl_Widget *w, void *eventData) {
  _editor->_textbuf->remove_selection();
}

/**
 * perform keyword completion
 */
void EditorWidget::expand_word(Fl_Widget *w, void *eventData) {
  int start, end;
  const char *fullWord = 0;
  unsigned fullWordLen = 0;

  Fl_Text_Buffer *textbuf = _editor->_textbuf;
  char *text = textbuf->text();

  if (textbuf->selected()) {
    // get word before selection
    int pos1, pos2;
    textbuf->selection_position(&pos1, &pos2);
    start = textbuf->word_start(pos1 - 1);
    end = pos1;
    // get word from before selection to end of selection
    fullWord = text + start;
    fullWordLen = pos2 - start - 1;
  } else {
    // nothing selected - get word to left of cursor position
    int pos = _editor->insert_position();
    end = textbuf->word_end(pos);
    start = textbuf->word_start(end - 1);
    completionIndex = 0;
  }

  if (start >= end) {
    free(text);
    return;
  }

  const char *expandWord = text + start;
  unsigned expandWordLen = end - start;
  int wordPos = 0;

  // scan for expandWord from within the current text buffer
  if (completionIndex != -1 && searchBackward(text, start - 1,
                                              expandWord, expandWordLen,
                                              &wordPos)) {
    int matchPos = -1;
    if (textbuf->selected() == 0) {
      matchPos = wordPos;
      completionIndex = 1;      // find next word on next call
    } else {
      // find the next word prior to the currently selected word
      int index = 1;
      while (wordPos > 0) {
        if (strncasecmp(text + wordPos, fullWord, fullWordLen) != 0 ||
            isalpha(text[wordPos + fullWordLen + 1])) {
          // isalpha - matches fullWord but word has more chars
          matchPos = wordPos;
          if (completionIndex == index) {
            completionIndex++;
            break;
          }
          // count index for non-matching fullWords only
          index++;
        }

        if (searchBackward(text, wordPos - 1, expandWord,
                           expandWordLen, &wordPos) == 0) {
          matchPos = -1;
          break;                // no more partial matches
        }
      }
      if (index == completionIndex) {
        // end of expansion sequence
        matchPos = -1;
      }
    }
    if (matchPos != -1) {
      char *word = textbuf->text_range(matchPos, textbuf->word_end(matchPos));
      if (textbuf->selected()) {
        textbuf->replace_selection(word + expandWordLen);
      } else {
        textbuf->insert(end, word + expandWordLen);
      }
      textbuf->select(end, end + strlen(word + expandWordLen));
      _editor->insert_position(end + strlen(word + expandWordLen));
      free((void *)word);
      free(text);
      return;
    }
  }

  completionIndex = -1;         // no more buffer expansions

  strlib::List<String *> keywords;
  _editor->getKeywords(keywords);

  // find the next replacement
  int firstIndex = -1;
  int lastIndex = -1;
  int curIndex = -1;
  int numWords = keywords.size();
  for (int i = 0; i < numWords; i++) {
    const char *keyword = ((String *)keywords.get(i))->c_str();
    if (strncasecmp(expandWord, keyword, expandWordLen) == 0) {
      if (firstIndex == -1) {
        firstIndex = i;
      }
      if (fullWordLen == 0) {
        if (expandWordLen == strlen(keyword)) {
          // nothing selected and word to left of cursor matches
          curIndex = i;
        }
      } else if (strncasecmp(fullWord, keyword, fullWordLen) == 0) {
        // selection+word to left of selection matches
        curIndex = i;
      }
      lastIndex = i;
    } else if (lastIndex != -1) {
      // moved beyond matching words
      break;
    }
  }

  if (lastIndex != -1) {
    if (lastIndex == curIndex || curIndex == -1) {
      lastIndex = firstIndex;   // wrap to first in subset
    } else {
      lastIndex = curIndex + 1;
    }

    const char *keyword = ((String *)keywords.get(lastIndex))->c_str();
    // updated the segment of the replacement text
    // that completes the current selection
    if (textbuf->selected()) {
      textbuf->replace_selection(keyword + expandWordLen);
    } else {
      textbuf->insert(end, keyword + expandWordLen);
    }
    textbuf->select(end, end + strlen(keyword + expandWordLen));
  }
  free(text);
}

/**
 * handler for find text command
 */
void EditorWidget::find(Fl_Widget *w, void *eventData) {
  setCommand(cmd_find);
}

/**
 * performs the current command
 */
void EditorWidget::command(Fl_Widget *w, void *eventData) {
  bool found = false;
  bool forward = (intptr_t) eventData;
  bool updatePos = (_commandOpt != cmd_find_inc);

  if (Fl::event_button() == FL_RIGHT_MOUSE) {
    // right click
    forward = 0;
  }

  switch (_commandOpt) {
  case cmd_find_inc:
  case cmd_find:
    found = _editor->findText(_commandText->value(), forward, updatePos);
    _commandText->textcolor(found ? _commandChoice->color() : FL_RED);
    _commandText->redraw();
    break;

  case cmd_replace:
    _commandBuffer.clear();
    _commandBuffer.append(_commandText->value());
    setCommand(cmd_replace_with);
    break;

  case cmd_replace_with:
    replace_next();
    break;

  case cmd_goto:
    gotoLine(atoi(_commandText->value()));
    take_focus();
    break;

  case cmd_input_text:
    wnd->setModal(false);
    break;
  }
}

/**
 * sub/func selection list handler
 */
void EditorWidget::func_list(Fl_Widget *w, void *eventData) {
  if (_funcList && _funcList->callback_item()) {
    Fl_Tree_Item *item = (Fl_Tree_Item*)_funcList->callback_item();
    const char *label = item->label();
    if (label) {
      _funcListEvent = true;
      if (strcmp(label, scanLabel) == 0) {
        resetList();
      } else {
        gotoLine((int)(intptr_t)item->user_data());
        take_focus();
      }
    }
  }
}

/**
 * goto-line command handler
 */
void EditorWidget::goto_line(Fl_Widget *w, void *eventData) {
  setCommand(cmd_goto);
}

/**
 * paste clipboard text onto the buffer
 */
void EditorWidget::paste_text(Fl_Widget *w, void *eventData) {
  Fl_Text_Editor::kf_paste(0, _editor);
}

/**
 * rename the currently selected variable
 */
void EditorWidget::rename_word(Fl_Widget *w, void *eventData) {
  if (rename_active) {
    rename_active = false;
  } else {
    Fl_Rect rc;
    char *selection = getSelection(&rc);
    if (selection) {
      showFindText(selection);
      begin();
      LineInput *in = new LineInput(rc.x(), rc.y(), rc.w() + 10, rc.h());
      end();

      in->value(selection);
      in->callback(rename_word_cb);
      in->textfont(FL_COURIER);
      in->textsize(getFontSize());

      rename_active = true;
      while (rename_active && in == Fl::focus()) {
        Fl::wait();
      }

      showFindText("");
      replaceAll(selection, in->value(), true, true);
      remove(in);
      take_focus();
      delete in;
      free((void *)selection);
    }
  }
}

/**
 * replace the next find occurance
 */
void EditorWidget::replace_next(Fl_Widget *w, void *eventData) {
  if (readonly()) {
    return;
  }

  const char *find = _commandBuffer;
  const char *replace = _commandText->value();

  Fl_Text_Buffer *textbuf = _editor->_textbuf;
  int pos = _editor->insert_position();
  int found = textbuf->search_forward(pos, find, &pos);

  if (found) {
    // found a match; update the position and replace text
    textbuf->select(pos, pos + strlen(find));
    textbuf->remove_selection();
    textbuf->insert(pos, replace);
    textbuf->select(pos, pos + strlen(replace));
    _editor->insert_position(pos + strlen(replace));
    _editor->show_insert_position();
  } else {
    setCommand(cmd_find);
    _editor->take_focus();
  }
}

/**
 * save file menu command handler
 */
void EditorWidget::save_file(Fl_Widget *w, void *eventData) {
  if (_filename[0] == '\0') {
    // no filename - get one!
    wnd->save_file_as();
    return;
  } else {
    doSaveFile(_filename);
  }
}

/**
 * prevent the tty window from scrolling with new data
 */
void EditorWidget::scroll_lock(Fl_Widget *w, void *eventData) {
  _tty->setScrollLock(_lockBn->value());
}

/**
 * select all text
 */
void EditorWidget::select_all(Fl_Widget *w, void *eventData) {
  Fl_Text_Editor::kf_select_all(0, _editor);
}

/**
 * set colour menu command handler
 */
void EditorWidget::set_color(Fl_Widget *w, void *eventData) {
  StyleField field = (StyleField)(intptr_t)eventData;
  uint8_t r, g, b;
  Fl::get_color(styletable[field].color, r, g, b);
  if (fl_color_chooser(w->label(), r, g, b)) {
    styletable[field].color = fl_rgb_color(r, g, b);
    _editor->styleChanged();
    wnd->_profile->updateTheme();
    wnd->_profile->setEditTheme(this);
    wnd->updateConfig(this);
    wnd->show();
  }
}

/**
 * replace text menu command handler
 */
void EditorWidget::show_replace(Fl_Widget *w, void *eventData) {
  const char *prime = _editor->_search;
  if (!prime || !prime[0]) {
    // use selected text when search not available
    prime = _editor->_textbuf->selection_text();
  }
  _commandText->value(prime);
  setCommand(cmd_replace);
}

/**
 * undo any edit changes
 */
void EditorWidget::undo(Fl_Widget *w, void *eventData) {
  Fl_Text_Editor::kf_undo(0, _editor);
}

/**
 * de-select the button specified in the eventData
 */
void EditorWidget::un_select(Fl_Widget *w, void *eventData) {
  ((Fl_Button *)eventData)->value(false);
}

//--Public methods--------------------------------------------------------------

/**
 * handles saving the current buffer
 */
bool EditorWidget::checkSave(bool discard) {
  if (!_dirty) {
    // continue next operation
    return true;
  }

  const char *msg = "The current file has not been saved\n\nWould you like to save it now?";
  int r;
  if (discard) {
    r = fl_choice(msg, "Save", "Discard", "Cancel", NULL);
  } else {
    r = fl_choice(msg, "Save", "Cancel", NULL, NULL);
  }
  fprintf(stderr, "selected %d\n", r);
  if (r == 0) {
    // save selected
    save_file();
    return !_dirty;
  }

  // continue operation when discard selected
  return (discard && r == 1);
}

/**
 * copy selection text to the clipboard
 */
void EditorWidget::copyText() {
  if (!_tty->copySelection()) {
    Fl_Text_Editor::kf_copy(0, _editor);
  }
}

/**
 * saves the editor buffer to the given file name
 */
void EditorWidget::doSaveFile(const char *newfile) {
  if (!_dirty && strcmp(newfile, _filename) == 0) {
    // neither buffer or filename have changed
    return;
  }

  char basfile[PATH_MAX];
  Fl_Text_Buffer *textbuf = _editor->_textbuf;

  if (wnd->_profile->createBackups() && access(newfile, 0) == 0) {
    // rename any existing file as a backup
    strcpy(basfile, newfile);
    strcat(basfile, "~");
    rename(newfile, basfile);
  }

  strcpy(basfile, newfile);
  if (strchr(basfile, '.') == 0) {
    strcat(basfile, ".bas");
  }

  if (textbuf->savefile(basfile)) {
    fl_alert("Error writing to file \'%s\':\n%s.", basfile, strerror(errno));
    return;
  }

  _dirty = 0;
  strcpy(_filename, basfile);
  _modifiedTime = getModifiedTime();

  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  // store a copy in lastedit.bas
  if (wnd->_profile->createBackups()) {
    getHomeDir(basfile, sizeof(basfile));
    strlcat(basfile, "lastedit.bas", sizeof(basfile));
    textbuf->savefile(basfile);
  }

  textbuf->call_modify_callbacks();
  showPath();
  fileChanged(true);
  addHistory(_filename);
  _editor->take_focus();
}

/**
 * called when the buffer has changed
 */
void EditorWidget::fileChanged(bool loadfile) {
  _funcList->clear();
  if (loadfile) {
    // update the func/sub navigator
    createFuncList();
    _funcList->redraw();

    const char *filename = getFilename();
    if (filename && filename[0]) {
      // update the last used file menu
      bool found = false;

      for (int i = 0; i < NUM_RECENT_ITEMS; i++) {
        if (recentPath[i].c_str() !=  NULL &&
            strcmp(filename, recentPath[i].c_str()) == 0) {
          found = true;
          break;
        }
      }

      if (found == false) {
        const char *label = FileWidget::splitPath(filename, NULL);

        // shift items downwards
        for (int i = NUM_RECENT_ITEMS - 1; i > 0; i--) {
          _menuBar->replace(recentMenu[i], recentLabel[i - 1]);
          recentLabel[i].clear();
          recentLabel[i].append(recentLabel[i - 1]);
          recentPath[i].clear();
          recentPath[i].append(recentPath[i - 1]);
        }
        // create new item in first position
        recentLabel[0].clear();
        recentLabel[0].append(label);
        recentPath[0].clear();
        recentPath[0].append(filename);
        _menuBar->replace(recentMenu[0], recentLabel[0]);
      }
    }
  }

  _funcList->add(scanLabel);
}

/**
 * keyboard shortcut handler
 */
bool EditorWidget::focusWidget() {
  switch (Fl::event_key()) {
  case 'b':
    setBreakToLine(!isBreakToLine());
    return true;

  case 'e':
    setCommand(cmd_find);
    take_focus();
    return true;

  case 'f':
    if (strlen(_commandText->value()) > 0 && _commandOpt == cmd_find) {
      // continue search - shift -> backward else forward
      command(0, (void *)(intptr_t)(Fl::event_state(FL_SHIFT) ? 0 : 1));
    }
    setCommand(cmd_find);
    return true;

  case 'i':
    setCommand(cmd_find_inc);
    return true;

  case 't':
    setLogPrint(!isLogPrint());
    return true;

  case 'w':
    setHideIde(!isHideIDE());
    return true;

  case 'j':
    setScrollLock(!isScrollLock());
    return true;
  }
  return false;
}

/**
 * returns the current font size
 */
int EditorWidget::getFontSize() {
  return _editor->getFontSize();
}

/**
 * use the input control as the INPUT basic command handler
 */
void EditorWidget::getInput(char *result, int size) {
  if (result && result[0]) {
    _commandText->value(result);
  }
  setCommand(cmd_input_text);
  wnd->setModal(true);
  while (wnd->isModal()) {
    Fl::wait();
  }
  if (wnd->isBreakExec()) {
    brun_break();
  } else {
    const char *value = _commandText->value();
    int valueLen = strlen(value);
    int len = (valueLen < size) ? valueLen : size;
    strncpy(result, value, len);
    result[len] = 0;
  }
  setCommand(cmd_find);
}

/**
 * returns the row and col position for the current cursor position
 */
void EditorWidget::getRowCol(int *row, int *col) {
  return ((BasicEditor *)_editor)->getRowCol(row, col);
}

/**
 * returns the selected text or the word around the cursor if there
 * is no current selection. caller must free the returned value
 */
char *EditorWidget::getSelection(int *start, int *end) {
  char *result = 0;

  Fl_Text_Buffer *tb = _editor->_textbuf;
  if (tb->selected()) {
    result = tb->selection_text();
    tb->selection_position(start, end);
  } else {
    int pos = _editor->insert_position();
    *start = tb->word_start(pos);
    *end = tb->word_end(pos);
    result = tb->text_range(*start, *end);
  }

  return result;
}

/**
 * returns where text selection ends
 */
void EditorWidget::getSelEndRowCol(int *row, int *col) {
  return ((BasicEditor *)_editor)->getSelEndRowCol(row, col);
}

/**
 * returns where text selection starts
 */
void EditorWidget::getSelStartRowCol(int *row, int *col) {
  return ((BasicEditor *)_editor)->getSelStartRowCol(row, col);
}

/**
 * sets the cursor to the given line number
 */
void EditorWidget::gotoLine(int line) {
  ((BasicEditor *)_editor)->gotoLine(line);
}

/**
 * FLTK event handler
 */
int EditorWidget::handle(int e) {
  switch (e) {
  case FL_SHOW:
  case FL_FOCUS:
    Fl::focus(_editor);
    handleFileChange();
    return 1;
  case FL_KEYBOARD:
    if (Fl::event_key() == FL_Escape) {
      take_focus();
      return 1;
    }
    break;
  case FL_ENTER:
    if (rename_active) {
      // prevent drawing over the inplace editor child control
      return 0;
    }
  }

  return Fl_Group::handle(e);
}

/**
 * load the given filename into the buffer
 */
void EditorWidget::loadFile(const char *newfile) {
  // save the current filename
  char oldpath[PATH_MAX];
  strcpy(oldpath, _filename);

  // convert relative path to full path
  getcwd(_filename, sizeof(_filename));
  strcat(_filename, "/");
  strcat(_filename, newfile);

  if (access(_filename, R_OK) != 0) {
    // filename unreadable, try newfile
    strcpy(_filename, newfile);
  }

  FileWidget::forwardSlash(_filename);

  _loading = true;
  if (_editor->_textbuf->loadfile(_filename)) {
    // read failed
    fl_alert("Error reading from file \'%s\':\n%s.", _filename, strerror(errno));

    // restore previous file
    strcpy(_filename, oldpath);
    _editor->_textbuf->loadfile(_filename);
  }

  _dirty = false;
  _loading = false;

  _editor->_textbuf->call_modify_callbacks();
  _editor->show_insert_position();
  _modifiedTime = getModifiedTime();
  readonly(false);

  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  showPath();
  fileChanged(true);
  setRowCol(1, 1);
}

/**
 * returns the buffer readonly flag
 */
bool EditorWidget::readonly() {
  return ((BasicEditor *)_editor)->_readonly;
}

/**
 * sets the buffer readonly flag
 */
void EditorWidget::readonly(bool is_readonly) {
  if (!is_readonly && access(_filename, W_OK) != 0) {
    // cannot set writable since file is readonly
    is_readonly = true;
  }
  _modStatus->label(is_readonly ? "RO" : "@line");
  _modStatus->redraw();
  _editor->cursor_style(is_readonly ? Fl_Text_Display::DIM_CURSOR : Fl_Text_Display::NORMAL_CURSOR);
  ((BasicEditor *)_editor)->_readonly = is_readonly;
}

/**
 * displays the current run-mode flag
 */
void EditorWidget::runState(RunMessage runMessage) {
  _runStatus->callback(MainWindow::run_cb);
  const char *msg = 0;
  switch (runMessage) {
  case rs_err:
    msg = "ERR";
    break;
  case rs_run:
    msg = "BRK";
    _runStatus->callback(MainWindow::run_break_cb);
    break;
  default:
    msg = "RUN";
  }
  _runStatus->copy_label(msg);
  _runStatus->redraw();
}

/**
 * Saves the selected text to the given file path
 */
void EditorWidget::saveSelection(const char *path) {
  FILE *fp = fopen(path, "w");
  if (fp) {
    Fl_Rect rc;
    char *selection = getSelection(&rc);
    if (selection) {
      fwrite(selection, strlen(selection), 1, fp);
      free((void *)selection);
    } else {
      // save as an empty file
      fputc(0, fp);
    }
    fclose(fp);
  }
}

/**
 * Sets the editor and editor toolbar color from the selected theme
 */
void EditorWidget::setTheme(EditTheme *theme) {
  _editor->color(get_color(theme->_background));
  _editor->linenumber_bgcolor(get_color(theme->_background));
  _editor->linenumber_fgcolor(get_color(theme->_number_color));
  _editor->cursor_color(get_color(theme->_cursor_color));
  _editor->selection_color(get_color(theme->_selection_color));
  _funcList->color(fl_color_average(get_color(theme->_background), get_color(theme->_color), .92f));
  _funcList->item_labelfgcolor(get_color(theme->_color));
  _tty->color(_editor->color());
  _tty->labelcolor(fl_contrast(_tty->color(), get_color(theme->_background)));
  _tty->selection_color(_editor->selection_color());
  resetList();
}

/**
 * sets the current display font
 */
void EditorWidget::setFont(Fl_Font font) {
  if (font) {
    _editor->setFont(font);
    _tty->setFont(font);
    wnd->_profile->setFont(font);
  }
}

/**
 * sets the current font size
 */
void EditorWidget::setFontSize(int size) {
  _editor->setFontSize(size);
  _tty->setFontSize(size);
  wnd->_profile->setFontSize(size);
}

/**
 * sets the indent level to the given amount
 */
void EditorWidget::setIndentLevel(int level) {
  ((BasicEditor *)_editor)->_indentLevel = level;
}

/**
 * displays the row/col in the editor toolbar
 */
void EditorWidget::setRowCol(int row, int col) {
  char rowcol[20];
  sprintf(rowcol, "%d", row);
  _rowStatus->copy_label(rowcol);
  _rowStatus->redraw();
  sprintf(rowcol, "%d", col);
  _colStatus->copy_label(rowcol);
  _colStatus->redraw();
  if (!_funcListEvent) {
    selectRowInBrowser(row);
  } else {
    _funcListEvent = false;
  }
}

/**
 * display the full pathname
 */
void EditorWidget::showPath() {
  _commandChoice->tooltip(_filename);
}

/**
 * prints a status message on the tty-widget
 */
void EditorWidget::statusMsg(const char *msg) {
  if (msg) {
    _tty->print(msg);
    _tty->print("\n");
  }
}

/**
 * sets the font face, size and colour
 */
void EditorWidget::updateConfig(EditorWidget *current) {
  setFont(current->_editor->getFont());
  setFontSize(current->_editor->getFontSize());
}

//--Protected methods-----------------------------------------------------------

/**
 * add filename to the hiistory file
 */
void EditorWidget::addHistory(const char *filename) {
  FILE *fp;
  char buffer[PATH_MAX];
  char updatedfile[PATH_MAX];
  char path[PATH_MAX];

  int len = strlen(filename);
  if (strcasecmp(filename + len - 4, ".sbx") == 0 || access(filename, R_OK) != 0) {
    // don't remember bas exe or invalid files
    return;
  }

  len -= strlen(untitledFile);
  if (len > 0 && strcmp(filename + len, untitledFile) == 0) {
    // don't remember the untitled file
    return;
  }
  // save paths with unix path separators
  strcpy(updatedfile, filename);
  FileWidget::forwardSlash(updatedfile);
  filename = updatedfile;

  // save into the history file
  getHomeDir(path, sizeof(path));
  strlcat(path, historyFile, sizeof(path));

  fp = fopen(path, "r");
  if (fp) {
    // don't add the item if it already exists
    while (feof(fp) == 0) {
      if (fgets(buffer, sizeof(buffer), fp) && strncmp(filename, buffer, strlen(filename) - 1) == 0) {
        fclose(fp);
        return;
      }
    }
    fclose(fp);
  }

  fp = fopen(path, "a");
  if (fp) {
    fwrite(filename, strlen(filename), 1, fp);
    fwrite("\n", 1, 1, fp);
    fclose(fp);
  }
}

/**
 * creates the sub/func selection list
 */
void EditorWidget::createFuncList() {
  Fl_Text_Buffer *textbuf = _editor->_textbuf;
  char *text = textbuf->text();
  int len = textbuf->length();
  int curLine = 1;
  const char *keywords[] = {
    "sub ", "func ", "def ", "label ", "const ", "local ", "dim "
  };
  int keywords_length = sizeof(keywords) / sizeof(keywords[0]);
  int keywords_len[keywords_length];
  for (int j = 0; j < keywords_length; j++) {
    keywords_len[j] = strlen(keywords[j]);
  }
  Fl_Tree_Item *menuGroup = NULL;

  for (int i = 0; i < len; i++) {
    // skip to the newline start
    while (i < len && i != 0 && text[i] != '\n') {
      i++;
    }

    // skip any successive newlines
    while (i < len && text[i] == '\n') {
      curLine++;
      i++;
    }

    // skip any leading whitespace
    while (i < len && (text[i] == ' ' || text[i] == '\t')) {
      i++;
    }

    for (int j = 0; j < keywords_length; j++) {
      if (!strncasecmp(text + i, keywords[j], keywords_len[j])) {
        i += keywords_len[j];
        int i_begin = i;
        while (i < len && text[i] != '=' && text[i] != '\r' && text[i] != '\n') {
          i++;
        }
        if (i > i_begin) {
          String s(text + i_begin, i - i_begin);
          if (j < 2) {
            menuGroup = new Fl_Tree_Item(_funcList);
            menuGroup->label(s.c_str());
            menuGroup->user_data((void *)(intptr_t)curLine);
            _funcList->add(s.c_str(), menuGroup);
          } else if (menuGroup != NULL) {
            Fl_Tree_Item *leaf = new Fl_Tree_Item(_funcList);
            leaf->label(s.c_str());
            leaf->user_data((void *)(intptr_t)curLine);
            menuGroup->add(_funcList->prefs(), s.c_str(), leaf);
          }
        }
        break;
      }
    }
    if (text[i] == '\n') {
      i--;                      // avoid eating the entire next line
    }
  }
  free(text);
}

/**
 * called when the buffer has change - sets the modified flag
 */
void EditorWidget::doChange(int inserted, int deleted) {
  if (!_loading) {
    // do nothing while file load in progress
    if (inserted || deleted) {
      _dirty = 1;
    }

    if (!readonly()) {
      setModified(_dirty);
    }
  }
}

/**
 * handler for the sub/func list selection event
 */
void EditorWidget::findFunc(const char *find) {
  char *text = _editor->_textbuf->text();
  int findLen = strlen(find);
  int len = _editor->_textbuf->length();
  int lineNo = 1;
  for (int i = 0; i < len; i++) {
    if (strncasecmp(text + i, find, findLen) == 0) {
      gotoLine(lineNo);
      break;
    } else if (text[i] == '\n') {
      lineNo++;
    }
  }
  free(text);
}

/**
 * returns the current selection text
 */
char *EditorWidget::getSelection(Fl_Rect *rc) {
  return ((BasicEditor *)_editor)->getSelection(rc);
}

/**
 * returns the current file modified time
 */
uint32_t EditorWidget::getModifiedTime() {
  struct stat st_file;
  uint32_t modified = 0;
  if (_filename[0] && !stat(_filename, &st_file)) {
    modified = st_file.st_mtime;
  }
  return modified;
}

/**
 * handler for the external file change event
 */
void EditorWidget::handleFileChange() {
  // handle outside changes to the file
  if (_filename[0] && _modifiedTime != 0 && _modifiedTime != getModifiedTime()) {
    String st;
    st.append("File")
      .append(_filename)
      .append("has changed on disk.\n\n")
      .append("Do you want to reload the file?");
    if (fl_choice(st.c_str(), "Yes", "No", NULL, NULL) == 0) {
      reloadFile();
    } else {
      _modifiedTime = 0;
    }
  }
}

/**
 * reset the function list
 */
void EditorWidget::resetList() {
  _funcList->clear();
  createFuncList();
  _funcList->add(scanLabel);
}

/**
 * resize the heights
 */
void EditorWidget::resize(int x, int y, int w, int h) {
  Fl_Group *tile = _editor->parent();
  int edit_scale_w = 1 + (_editor->w() * 100 / tile->w());
  int edit_scale_h = 1 + (_editor->h() * 100 / tile->h());
  edit_scale_w = MIN(80, edit_scale_w);
  edit_scale_h = MIN(80, edit_scale_h);

  tile->resizable(_editor);
  Fl_Group::resize(x, y, w, h);
  tile->resize(tile->x(), y, w, h - STATUS_HEIGHT);
  tile->resizable(NULL);

  int status_y = y + h - STATUS_HEIGHT;
  int edit_w = tile->w() * edit_scale_w / 100;
  int edit_h = tile->h() * edit_scale_h / 100;
  int tty_y = tile->y() + edit_h;
  int tty_h = status_y - tty_y;
  int func_x = tile->x() + edit_w;
  int func_w = tile->w() - edit_w;

  _editor->resize(_editor->x(), _editor->y(), edit_w, edit_h);
  _funcList->resize(func_x, _funcList->y(), func_w, _editor->h());
  _tty->resize(_tty->x(), tty_y, _tty->w(), tty_h);
  _statusBar->resize(_statusBar->x(), status_y, _statusBar->w(), STATUS_HEIGHT);
}

/**
 * create a new editor buffer
 */
void EditorWidget::newFile() {
  if (!readonly() && checkSave(true)) {
    Fl_Text_Buffer *textbuf = _editor->_textbuf;
    _filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    _dirty = 0;
    textbuf->call_modify_callbacks();
    fileChanged(false);
    _modifiedTime = 0;
  }
}

/**
 * reload the editor buffer
 */
void EditorWidget::reloadFile() {
  char buffer[PATH_MAX];
  strcpy(buffer, _filename);
  loadFile(buffer);
}

/**
 * replace all occurances of the given text
 */
int EditorWidget::replaceAll(const char *find, const char *replace, bool restorePos, bool matchWord) {
  int times = 0;

  if (strcmp(find, replace) != 0) {
    Fl_Text_Buffer *textbuf = _editor->_textbuf;
    int prevPos = _editor->insert_position();

    // loop through the whole string
    int pos = 0;
    _editor->insert_position(pos);

    while (textbuf->search_forward(pos, find, &pos)) {
      // found a match; update the position and replace text
      if (!matchWord ||
          ((pos == 0 || !isvar(textbuf->char_at(pos - 1))) &&
           !isvar(textbuf->char_at(pos + strlen(find))))) {
        textbuf->select(pos, pos + strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
      }
      // advance beyond replace string
      pos += strlen(replace);
      _editor->insert_position(pos);
      times++;
    }

    if (restorePos) {
      _editor->insert_position(prevPos);
    }
    _editor->show_insert_position();
  }

  return times;
}

/**
 * handler for searching backwards
 */
bool EditorWidget::searchBackward(const char *text, int startPos,
                                  const char *find, int findLen, int *foundPos) {
  int matchIndex = findLen - 1;
  for (int i = startPos; i >= 0; i--) {
    bool equals = toupper(text[i]) == toupper(find[matchIndex]);
    if (equals == false && matchIndex < findLen - 1) {
      // partial match now fails - reset search at current index
      matchIndex = findLen - 1;
      equals = toupper(text[i]) == toupper(find[matchIndex]);
    }
    matchIndex = (equals ? matchIndex - 1 : findLen - 1);
    if (matchIndex == -1 && (i == 0 || isalpha(text[i - 1]) == 0)) {
      // char prior to word is non-alpha
      *foundPos = i;
      return true;
    }
  }
  return false;
}

/**
 * sync the browser widget selection
 */
void EditorWidget::selectRowInBrowser(int row) {
  Fl_Tree_Item *root = _funcList->root();
  int len = root->children() - 1;
  bool found = false;
  for (int i = 0; i < len; i++) {
    int line = (int)(intptr_t)root->child(i)->user_data();
    int nextLine = (int)(intptr_t)root->child(i + 1)->user_data();
    if (row >= line && (i == len - 1 || row < nextLine)) {
      int y = _funcList->vposition() + root->child(i)->y();
      int bottom = _funcList->y() + _funcList->h();
      int pos = bottom - y - (_funcList->h() / 2);
      _funcList->select_only(root->child(i), 0);
      _funcList->vposition(-pos);
      found = true;
      break;
    }
  }
  if (!found) {
    _funcList->select_only(root->child(0), 0);
    _funcList->vposition(0);
  }
}

/**
 * sets the current command
 */
void EditorWidget::setCommand(CommandOpt command) {
  if (_commandOpt == cmd_input_text) {
    wnd->setModal(false);
  }

  _commandOpt = command;
  switch (command) {
  case cmd_find:
    _commandChoice->label("@search  Find:");
    break;
  case cmd_find_inc:
    _commandChoice->label("Inc Find:");
    break;
  case cmd_replace:
    _commandChoice->label("Replace:");
    break;
  case cmd_replace_with:
    _commandChoice->label("With:");
    break;
  case cmd_goto:
    _commandChoice->label("Goto:");
    break;
  case cmd_input_text:
    _commandChoice->label("INPUT:");
    break;
  }
  _commandChoice->redraw();

  _commandText->color(_commandChoice->color());
  _commandText->redraw();
  _commandText->take_focus();
  _commandText->when(_commandOpt == cmd_find_inc ? FL_WHEN_CHANGED : FL_WHEN_ENTER_KEY_ALWAYS);
}

/**
 * display the toolbar modified flag
 */
void EditorWidget::setModified(bool dirty) {
  _dirty = dirty;
  _modStatus->when(dirty ? FL_WHEN_CHANGED : FL_WHEN_NEVER);
  _modStatus->label(dirty ? "MOD" : "@line");
  _modStatus->redraw();
}

/**
 * highlight the given search text
 */
void EditorWidget::showFindText(const char *text) {
  _editor->showFindText(text);
}

LineInput::LineInput(int x, int y, int w, int h) :
  Fl_Input(x, y, w, h),
  orig_x(x),
  orig_y(y),
  orig_w(w),
  orig_h(h) {
  when(FL_WHEN_ENTER_KEY);
  box(FL_BORDER_BOX);
  fl_color(fl_rgb_color(220, 220, 220));
  take_focus();
}

/**
 * veto the layout changes
 */
void LineInput::resize(int x, int y, int w, int h) {
  Fl_Input::resize(orig_x, orig_y, orig_w, orig_h);
}

int LineInput::handle(int event) {
  int result;
  if (event == FL_KEYBOARD) {
    if (Fl::event_state(FL_CTRL) && Fl::event_key() == 'b') {
      if (!wnd->isEdit()) {
        wnd->setBreak();
      }
    } else if (Fl::event_key(FL_Escape)) {
      do_callback();
    } else {
      // grow the input box width as text is entered
      const char *text = value();
      int strw = fl_width(text) + fl_width(value()) + 4;
      if (strw > w()) {
        w(strw);
        orig_w = strw;
        redraw();
      }
    }
    result = Fl_Input::handle(event);
  } else {
    result = Fl_Input::handle(event);
  }
  return result;
}
