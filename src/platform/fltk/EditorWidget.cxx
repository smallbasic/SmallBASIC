//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <fltk/Browser.h>
#include <fltk/ColorChooser.h>
#include <fltk/Item.h>
#include <fltk/TiledGroup.h>
#include <fltk/ask.h>
#include <fltk/damage.h>
#include <fltk/events.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "EditorWidget.h"
#include "FileWidget.h"
#include "common/smbas.h"

using namespace fltk;

#define TTY_ROWS 1000

// in MainWindow.cxx
extern String recentPath[];
extern Widget *recentMenu[];
extern const char *historyFile;
extern const char *untitledFile;

// in dev_fltk.cpp
void getHomeDir(char *filename, bool appendSlash = true);

// in BasicEditor.cxx
extern TextDisplay::StyleTableEntry styletable[];
extern Color defaultColor[];

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

//--EditorWidget----------------------------------------------------------------

EditorWidget::EditorWidget(int x, int y, int w, int h) :
  Group(x, y, w, h) {
  filename[0] = 0;
  dirty = false;
  loading = false;
  modifiedTime = 0;
  box(NO_BOX);

  begin();

  int tileHeight = h - (MNU_HEIGHT + 8);
  int ttyHeight = h / 8;
  int editHeight = tileHeight - ttyHeight;
  int browserWidth = w / 8;

  TiledGroup *tile = new TiledGroup(0, 0, w, tileHeight);
  tile->begin();

  editor = new BasicEditor(0, 0, w - browserWidth, editHeight, this);
  editor->linenumber_width(40);
  editor->wrap_mode(true, 0);
  editor->selection_color(fltk::color(190, 189, 188));
  editor->textbuf->add_modify_callback(changed_cb, this);
  editor->box(NO_BOX);
  editor->take_focus();

  // sub-func jump droplist
  funcList = new Browser(editor->w(), 0, browserWidth, editHeight);
  funcList->labelfont(HELVETICA);
  funcList->indented(1);
  funcList->when(WHEN_RELEASE);
  funcList->add(scanLabel);

  tty = new TtyWidget(0, editHeight, w, ttyHeight, TTY_ROWS);
  tty->color(WHITE);            // bg
  tty->labelcolor(BLACK);       // fg

  tile->end();

  // create the editor toolbar
  w -= 4;

  // editor status bar
  Group *statusBar = new Group(2, tty->b() + 2, w, MNU_HEIGHT);
  statusBar->box(NO_BOX);
  statusBar->begin();

  // widths become relative when the outer window is resized
  int st_w = 40;
  int bn_w = 18;
  int st_h = statusBar->h() - 2;

  logPrintBn = new ToggleButton(w - (bn_w + 2), 2, bn_w, st_h);
  lockBn = new ToggleButton(logPrintBn->x() - (bn_w + 2), 2, bn_w, st_h);
  hideIdeBn = new ToggleButton(lockBn->x() - (bn_w + 2), 2, bn_w, st_h);
  gotoLineBn = new ToggleButton(hideIdeBn->x() - (bn_w + 2), 2, bn_w, st_h);

#ifdef __MINGW32__
  // fixup alignment under windows
  logPrintBn->align(ALIGN_INSIDE | ALIGN_LEFT | ALIGN_CENTER);
  lockBn->align(ALIGN_INSIDE | ALIGN_LEFT | ALIGN_CENTER);
  hideIdeBn->align(ALIGN_INSIDE | ALIGN_LEFT | ALIGN_CENTER);
  gotoLineBn->align(ALIGN_INSIDE | ALIGN_LEFT | ALIGN_CENTER);
#endif

  colStatus = new Button(gotoLineBn->x() - (st_w + 2), 2, st_w, st_h);
  rowStatus = new Button(colStatus->x() - (st_w + 2), 2, st_w, st_h);
  runStatus = new Button(rowStatus->x() - (st_w + 2), 2, st_w, st_h);
  modStatus = new Button(runStatus->x() - (st_w + 2), 2, st_w, st_h);

  commandChoice = new Button(0, 2, 80, st_h);
  commandText = new Input(commandChoice->r() + 2, 2, modStatus->x() - commandChoice->r() - 4, st_h);
  commandText->align(ALIGN_LEFT | ALIGN_CLIP);
  commandText->when(WHEN_ENTER_KEY_ALWAYS);
  commandText->labelfont(HELVETICA);

  for (int n = 0; n < statusBar->children(); n++) {
    Widget *w = statusBar->child(n);
    w->labelfont(HELVETICA);
    w->box(BORDER_BOX);
  }

  statusBar->resizable(commandText);
  statusBar->end();

  resizable(tile);
  end();

  // command selection
  setCommand(cmd_find);
  runState(rs_ready);
  setModified(false);

  // button callbacks
  lockBn->callback(scroll_lock_cb);
  modStatus->callback(save_file_cb);
  runStatus->callback(MainWindow::run_cb);
  commandChoice->callback(command_cb, (void *)1);
  commandText->callback(command_cb, (void *)1);
  funcList->callback(func_list_cb, 0);
  logPrintBn->callback(un_select_cb, (void *)hideIdeBn);
  hideIdeBn->callback(un_select_cb, (void *)logPrintBn);
  colStatus->callback(goto_line_cb, 0);
  rowStatus->callback(goto_line_cb, 0);

  // setup icons
  logPrintBn->label("@i;@b;T"); // italic bold T
  lockBn->label("@||;");        // vertical bars
  hideIdeBn->label("@border_frame;");   // large dot
  gotoLineBn->label("@>;");     // right arrow (goto)

  // setup tooltips
  commandText->tooltip("Press Ctrl+f or Ctrl+Shift+f to find again");
  rowStatus->tooltip("Cursor row position");
  colStatus->tooltip("Cursor column position");
  runStatus->tooltip("Run or BREAK");
  modStatus->tooltip("Save file");
  logPrintBn->tooltip("Display PRINT statements in the log window");
  lockBn->tooltip("Prevent log window auto-scrolling");
  hideIdeBn->tooltip("Hide the editor while program is running");
  gotoLineBn->tooltip("Position the cursor to the last program line after BREAK");

  // setup defaults or restore settings
  if (wnd && wnd->profile) {
    wnd->profile->loadConfig(this);
  } else {
    setEditorColor(WHITE, true);
  }
  take_focus();
}

EditorWidget::~EditorWidget() {
  delete editor;
}

//--Event handler methods-------------------------------------------------------

/**
 * change the selected text to upper/lower/camel case
 */
void EditorWidget::change_case(Widget *w, void *eventData) {
  TextBuffer *tb = editor->buffer();
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
void EditorWidget::command_opt(Widget *w, void *eventData) {
  setCommand((CommandOpt) (intptr_t) eventData);
}

/**
 * cut selected text to the clipboard
 */
void EditorWidget::cut_text(Widget *w, void *eventData) {
  TextEditor::kf_cut(0, editor);
}

/**
 * delete selected text
 */
void EditorWidget::do_delete(Widget *w, void *eventData) {
  editor->textbuf->remove_selection();
}

/**
 * perform keyword completion
 */
void EditorWidget::expand_word(Widget *w, void *eventData) {
  int start, end;
  const char *fullWord = 0;
  unsigned fullWordLen = 0;

  TextBuffer *textbuf = editor->buffer();
  const char *text = textbuf->text();

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
    int pos = editor->insert_position();
    end = textbuf->word_end(pos);
    start = textbuf->word_start(end - 1);
    completionIndex = 0;
  }

  if (start >= end) {
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
      editor->insert_position(end + strlen(word + expandWordLen));
      free((void *)word);
      return;
    }
  }

  completionIndex = -1;         // no more buffer expansions

  strlib::List keywords;
  editor->getKeywords(keywords);

  // find the next replacement
  int firstIndex = -1;
  int lastIndex = -1;
  int curIndex = -1;
  int numWords = keywords.length();
  for (int i = 0; i < numWords; i++) {
    const char *keyword = ((String *)keywords.get(i))->toString();
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

    const char *keyword = ((String *)keywords.get(lastIndex))->toString();
    // updated the segment of the replacement text
    // that completes the current selection
    if (textbuf->selected()) {
      textbuf->replace_selection(keyword + expandWordLen);
    } else {
      textbuf->insert(end, keyword + expandWordLen);
    }
    textbuf->select(end, end + strlen(keyword + expandWordLen));
  }
}

/**
 * handler for find text command
 */
void EditorWidget::find(Widget *w, void *eventData) {
  setCommand(cmd_find);
}

/**
 * performs the current command
 */
void EditorWidget::command(Widget *w, void *eventData) {
  bool found = false;
  bool forward = (intptr_t) eventData;
  bool updatePos = (commandOpt != cmd_find_inc);

  if (event_button() == RightButton) {
    // right click
    forward = 0;
  }

  switch (commandOpt) {
  case cmd_find_inc:
  case cmd_find:
    found = editor->findText(commandText->value(), forward, updatePos);
    commandText->textcolor(found ? commandChoice->textcolor() : RED);
    commandText->redraw();
    break;

  case cmd_replace:
    commandBuffer.empty();
    commandBuffer.append(commandText->value());
    setCommand(cmd_replace_with);
    break;

  case cmd_replace_with:
    replace_next();
    break;

  case cmd_goto:
    gotoLine(atoi(commandText->value()));
    take_focus();
    break;

  case cmd_input_text:
    wnd->setModal(false);
    break;
  }
}

/**
 * font menu selection handler
 */
void EditorWidget::font_name(Widget *w, void *eventData) {
  setFont(fltk::font(w->label(), 0));
  wnd->updateConfig(this);
}

/**
 * sub/func selection list handler
 */
void EditorWidget::func_list(Widget *w, void *eventData) {
  if (funcList && funcList->item()) {
    const char *label = funcList->item()->label();
    if (label) {
      if (strcmp(label, scanLabel) == 0) {
        funcList->clear();
        createFuncList();
        funcList->add(scanLabel);
      } else {
        gotoLine(funcList->item()->argument());
        take_focus();
      }
    }
  }
}

/**
 * goto-line command handler
 */
void EditorWidget::goto_line(Widget *w, void *eventData) {
  setCommand(cmd_goto);
}

/**
 * paste clipboard text onto the buffer
 */
void EditorWidget::paste_text(Widget *w, void *eventData) {
  TextEditor::kf_paste(0, editor);
}

/**
 * rename the currently selected variable
 */
void EditorWidget::rename_word(Widget *w, void *eventData) {
  if (rename_active) {
    rename_active = false;
  } else {
    Rectangle rc;
    char *selection = getSelection(&rc);
    if (selection) {
      showFindText(selection);
      begin();
      LineInput *in = new LineInput(rc.x(), rc.y(), rc.w() + 10, rc.h());
      end();
      in->text(selection);
      in->callback(rename_word_cb);
      in->textfont(COURIER);
      in->textsize(getFontSize());

      rename_active = true;
      while (rename_active && in->focused()) {
        fltk::wait();
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
void EditorWidget::replace_next(Widget *w, void *eventData) {
  if (readonly()) {
    return;
  }

  const char *find = commandBuffer;
  const char *replace = commandText->value();

  TextBuffer *textbuf = editor->textbuf;
  int pos = editor->insert_position();
  int found = textbuf->search_forward(pos, find, &pos);

  if (found) {
    // found a match; update the position and replace text
    textbuf->select(pos, pos + strlen(find));
    textbuf->remove_selection();
    textbuf->insert(pos, replace);
    textbuf->select(pos, pos + strlen(replace));
    editor->insert_position(pos + strlen(replace));
    editor->show_insert_position();
  } else {
    setCommand(cmd_find);
    editor->take_focus();
  }
}

/**
 * save file menu command handler
 */
void EditorWidget::save_file(Widget *w, void *eventData) {
  if (filename[0] == '\0') {
    // no filename - get one!
    wnd->save_file_as();
    return;
  } else {
    doSaveFile(filename);
  }
}

/**
 * prevent the tty window from scrolling with new data
 */
void EditorWidget::scroll_lock(Widget *w, void *eventData) {
  tty->setScrollLock(w->flags() & STATE);
}

/**
 * select all text
 */
void EditorWidget::select_all(Widget *w, void *eventData) {
  TextEditor::kf_select_all(0, editor);
}

/**
 * set colour menu command handler
 */
void EditorWidget::set_color(Widget *w, void *eventData) {
  StyleField styleField = (StyleField) (intptr_t) eventData;
  if (styleField == st_background || styleField == st_background_def) {
    uchar r, g, b;
    split_color(editor->color(), r, g, b);
    if (color_chooser(w->label(), r, g, b)) {
      Color c = fltk::color(r, g, b);
      set_color_index(fltk::FREE_COLOR + styleField, c);
      setEditorColor(c, styleField == st_background_def);
      editor->styleChanged();
    }
    editor->take_focus();
  } else {
    setColor(w->label(), styleField);
  }
  wnd->updateConfig(this);
  wnd->show();
}

/**
 * replace text menu command handler
 */
void EditorWidget::show_replace(Widget *w, void *eventData) {
  const char *prime = editor->search;
  if (!prime || !prime[0]) {
    // use selected text when search not available
    prime = editor->textbuf->selection_text();
  }
  commandText->value(prime);
  setCommand(cmd_replace);
}

/**
 * undo any edit changes
 */
void EditorWidget::undo(Widget *w, void *eventData) {
  TextEditor::kf_undo(0, editor);
}

/**
 * de-select the button specified in the eventData
 */
void EditorWidget::un_select(Widget *w, void *eventData) {
  ((Button *)eventData)->value(false);
}

//--Public methods--------------------------------------------------------------

/**
 * handles saving the current buffer
 */
bool EditorWidget::checkSave(bool discard) {
  if (!dirty) {
    return true;                // continue next operation
  }

  const char *msg = "The current file has not been saved.\n" "Would you like to save it now?";
  int r = discard ? choice(msg, "Save", "Discard", "Cancel") : choice(msg, "Save", "Cancel", 0);
  if (r == 0) {
    save_file();                // Save the file
    return !dirty;
  }
  return (discard && r == 1);
}

/**
 * copy selection text to the clipboard
 */
void EditorWidget::copyText() {
  if (!tty->copySelection()) {
    TextEditor::kf_copy(0, editor);
  }
}

/**
 * saves the editor buffer to the given file name
 */
void EditorWidget::doSaveFile(const char *newfile) {
  if (!dirty && strcmp(newfile, filename) == 0) {
    // neither buffer or filename have changed
    return;
  }

  char basfile[PATH_MAX];
  TextBuffer *textbuf = editor->textbuf;

  if (wnd->profile->createBackups && access(newfile, 0) == 0) {
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
    alert("Error writing to file \'%s\':\n%s.", basfile, strerror(errno));
    return;
  }

  dirty = 0;
  strcpy(filename, basfile);
  modifiedTime = getModifiedTime();

  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  // store a copy in lastedit.bas
  if (wnd->profile->createBackups) {
    getHomeDir(basfile);
    strcat(basfile, "lastedit.bas");
    textbuf->savefile(basfile);
  }

  textbuf->call_modify_callbacks();
  showPath();
  fileChanged(true);
  addHistory(filename);
  editor->take_focus();
}

/**
 * called when the buffer has changed
 */
void EditorWidget::fileChanged(bool loadfile) {
  funcList->clear();
  if (loadfile) {
    // update the func/sub navigator
    createFuncList();
    funcList->redraw();

    const char *filename = getFilename();
    if (filename && filename[0]) {
      // update the last used file menu
      bool found = false;

      for (int i = 0; i < NUM_RECENT_ITEMS; i++) {
        if (strcmp(filename, recentPath[i].toString()) == 0) {
          found = true;
          break;
        }
      }

      if (found == false) {
        // shift items downwards
        for (int i = NUM_RECENT_ITEMS - 1; i > 0; i--) {
          recentMenu[i]->copy_label(recentMenu[i - 1]->label());
          recentPath[i].empty();
          recentPath[i].append(recentPath[i - 1]);
        }
        // create new item in first position
        const char *label = FileWidget::splitPath(filename, null);
        recentPath[0].empty();
        recentPath[0].append(filename);
        recentMenu[0]->copy_label(label);
      }
    }
  }

  funcList->add(scanLabel);
}

/**
 * keyboard shortcut handler
 */
bool EditorWidget::focusWidget() {
  switch (event_key()) {
  case 'b':
    setBreakToLine(!isBreakToLine());
    return true;

  case 'e':
    setCommand(cmd_find);
    take_focus();
    return true;

  case 'f':
    if (strlen(commandText->value()) > 0 && commandOpt == cmd_find) {
      // continue search - shift -> backward else forward
      command(0, (void *)((event_key_state(LeftShiftKey) || 
                           event_key_state(RightShiftKey)) ? 0 : 1));
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
  }
  return false;
}

/**
 * returns the current font size
 */
int EditorWidget::getFontSize() {
  return editor->getFontSize();
}

/**
 * use the input control as the INPUT basic command handler
 */
void EditorWidget::getInput(char *result, int size) {
  setCommand(cmd_input_text);
  wnd->setModal(true);
  while (wnd->isModal()) {
    fltk::wait();
  }
  if (wnd->isBreakExec()) {
    brun_break();
  } else {
    const char *value = commandText->value();
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
  return ((BasicEditor *)editor)->getRowCol(row, col);
}

/**
 * returns the selected text or the word around the cursor if there
 * is no current selection. caller must free the returned value
 */
char *EditorWidget::getSelection(int *start, int *end) {
  char *result = 0;

  TextBuffer *tb = editor->buffer();
  if (tb->selected()) {
    result = tb->selection_text();
    tb->selection_position(start, end);
  } else {
    int pos = editor->insert_position();
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
  return ((BasicEditor *)editor)->getSelEndRowCol(row, col);
}

/**
 * returns where text selection starts
 */
void EditorWidget::getSelStartRowCol(int *row, int *col) {
  return ((BasicEditor *)editor)->getSelStartRowCol(row, col);
}

/**
 * sets the cursor to the given line number
 */
void EditorWidget::gotoLine(int line) {
  ((BasicEditor *)editor)->gotoLine(line);
}

/**
 * FLTK event handler
 */
int EditorWidget::handle(int e) {
  switch (e) {
  case SHOW:
  case FOCUS:
    fltk::focus(editor);
    handleFileChange();
    return 1;
  case KEY:
    if (event_key() == EscapeKey) {
      take_focus();
      return 1;
    }
    break;
  case ENTER:
    if (rename_active) {
      // prevent drawing over the inplace editor child control
      return 0;
    }
  }

  return Group::handle(e);
}

/**
 * load the given filename into the buffer
 */
void EditorWidget::loadFile(const char *newfile) {
  // save the current filename
  char oldpath[PATH_MAX];
  strcpy(oldpath, filename);

  // convert relative path to full path
  getcwd(filename, sizeof(filename));
  strcat(filename, "/");
  strcat(filename, newfile);

  if (access(filename, R_OK) != 0) {
    // filename unreadable, try newfile
    strcpy(filename, newfile);
  }

  FileWidget::forwardSlash(filename);

  loading = true;
  if (editor->textbuf->loadfile(filename)) {
    // read failed
    alert("Error reading from file \'%s\':\n%s.", filename, strerror(errno));

    // restore previous file
    strcpy(filename, oldpath);
    editor->textbuf->loadfile(filename);
  }

  dirty = false;
  loading = false;

  editor->textbuf->call_modify_callbacks();
  editor->show_insert_position();
  modifiedTime = getModifiedTime();
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
  return ((BasicEditor *)editor)->readonly;
}

/**
 * sets the buffer readonly flag
 */
void EditorWidget::readonly(bool is_readonly) {
  if (!is_readonly && access(filename, W_OK) != 0) {
    // cannot set writable since file is readonly
    is_readonly = true;
  }
  modStatus->label(is_readonly ? "RO" : "@line");
  modStatus->redraw();
  editor->cursor_style(is_readonly ? TextDisplay::DIM_CURSOR : TextDisplay::NORMAL_CURSOR);
  ((BasicEditor *)editor)->readonly = is_readonly;
}

/**
 * displays the current run-mode flag
 */
void EditorWidget::runState(RunMessage runMessage) {
  runStatus->callback(MainWindow::run_cb);
  const char *msg = 0;
  switch (runMessage) {
  case rs_err:
    msg = "ERR";
    break;
  case rs_run:
    msg = "BRK";
    runStatus->callback(MainWindow::run_break_cb);
    break;
  default:
    msg = "RUN";
  }
  runStatus->copy_label(msg);
  runStatus->redraw();
}

/**
 * Saves the selected text to the given file path
 */
void EditorWidget::saveSelection(const char *path) {
  int err;
  FILE *fp = fopen(path, "w");
  if (fp) {
    Rectangle rc;
    char *selection = getSelection(&rc);
    if (selection) {
      err = fwrite(selection, strlen(selection), 1, fp);
      free((void *)selection);
    } else {
      // save as an empty file
      fputc(0, fp);
    }
    fclose(fp);
  }
}

/**
 * Sets the editor and editor toolbar color
 */
void EditorWidget::setEditorColor(Color c, bool defColor) {
  if (wnd && wnd->profile) {
    wnd->profile->color = c;
  }
  editor->color(c);

  Color bg = lerp(c, BLACK, .1f);       // same offset as editor line numbers
  Color fg = contrast(c, bg);
  int i;

  // set the colours on the command text bar
  for (i = commandText->parent()->children(); i > 0; i--) {
    Widget *child = commandText->parent()->child(i - 1);
    setWidgetColor(child, bg, fg);
  }

  // set the colours on the function list
  setWidgetColor(funcList, bg, fg);

  if (defColor) {
    // contrast the default colours against the background
    for (i = 0; i < st_background; i++) {
      styletable[i].color = contrast(defaultColor[i], c);
    }
  }
}

/**
 * sets the current display font
 */
void EditorWidget::setFont(Font *font) {
  if (font) {
    editor->setFont(font);
    tty->setFont(font);
    wnd->profile->font = font;
  }
}

/**
 * sets the current font size
 */
void EditorWidget::setFontSize(int size) {
  editor->setFontSize(size);
  tty->setFontSize(size);
  wnd->profile->fontSize = size;
}

/**
 * sets the indent level to the given amount
 */
void EditorWidget::setIndentLevel(int level) {
  ((BasicEditor *)editor)->indentLevel = level;

  // update environment var for running programs
  char path[MAX_PATH];
  sprintf(path, "INDENT_LEVEL=%d", level);
  putenv(path);
}

/**
 * displays the row/col in the editor toolbar
 */
void EditorWidget::setRowCol(int row, int col) {
  char rowcol[20];
  sprintf(rowcol, "%d", row);
  rowStatus->copy_label(rowcol);
  rowStatus->redraw();
  sprintf(rowcol, "%d", col);
  colStatus->copy_label(rowcol);
  colStatus->redraw();

  // sync the browser widget selection
  int len = funcList->children() - 1;
  for (int i = 0; i < len; i++) {
    int line = (int)funcList->child(i)->argument();
    int nextLine = (int)funcList->child(i + 1)->argument();
    if (row >= line && (i == len - 1 || row < nextLine)) {
      funcList->value(i);
      break;
    }
  }
}

/**
 * display the full pathname
 */
void EditorWidget::showPath() {
  commandChoice->tooltip(filename);
}

/**
 * prints a status message on the tty-widget
 */
void EditorWidget::statusMsg(const char *msg) {
  if (msg) {
    tty->print(msg);
    tty->print("\n");
  }
}

/**
 * sets the font face, size and colour
 */
void EditorWidget::updateConfig(EditorWidget *current) {
  setFont(font(current->editor->getFontName()));
  setFontSize(current->editor->getFontSize());
  setEditorColor(current->editor->color(), false);
}

//--Protected methods-----------------------------------------------------------

/**
 * add filename to the hiistory file
 */
void EditorWidget::addHistory(const char *filename) {
  FILE *fp;
  char buffer[MAX_PATH];
  char updatedfile[MAX_PATH];
  char path[MAX_PATH];

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
  getHomeDir(path);
  strcat(path, historyFile);

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
    int err;
    err = fwrite(filename, strlen(filename), 1, fp);
    err = fwrite("\n", 1, 1, fp);
    fclose(fp);
  }
}

/**
 * creates the sub/func selection list
 */
void EditorWidget::createFuncList() {
  TextBuffer *textbuf = editor->textbuf;
  const char *text = textbuf->text();
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
  Group *menuGroup = 0;

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
            menuGroup = funcList->add_group(s.toString(), 0, (void *)curLine);
          } else {
            funcList->add_leaf(s.toString(), menuGroup, (void *)curLine);
          }
        }
        break;
      }
    }
    if (text[i] == '\n') {
      i--;                      // avoid eating the entire next line
    }
  }
}

/**
 * called when the buffer has change - sets the modified flag
 */
void EditorWidget::doChange(int inserted, int deleted) {
  if (!loading) {
    // do nothing while file load in progress
    if (inserted || deleted) {
      dirty = 1;
    }

    if (!readonly()) {
      setModified(dirty);
    }
  }
}

/**
 * handler for the sub/func list selection event
 */
void EditorWidget::findFunc(const char *find) {
  const char *text = editor->textbuf->text();
  int findLen = strlen(find);
  int len = editor->textbuf->length();
  int lineNo = 1;
  for (int i = 0; i < len; i++) {
    if (strncasecmp(text + i, find, findLen) == 0) {
      gotoLine(lineNo);
      break;
    } else if (text[i] == '\n') {
      lineNo++;
    }
  }
}

/**
 * returns the current selection text
 */
char *EditorWidget::getSelection(Rectangle *rc) {
  return ((BasicEditor *)editor)->getSelection(rc);
}

/**
 * returns the current file modified time
 */
U32 EditorWidget::getModifiedTime() {
  struct stat st_file;
  U32 modified = 0;
  if (filename[0] && !stat(filename, &st_file)) {
    modified = st_file.st_mtime;
  }
  return modified;
}

/**
 * handler for the external file change event
 */
void EditorWidget::handleFileChange() {
  // handle outside changes to the file
  if (filename[0] && modifiedTime != 0 && modifiedTime != getModifiedTime()) {
    const char *msg = "File %s\nhas changed on disk.\n\n" "Do you want to reload the file?";
    if (ask(msg, filename)) {
      reloadFile();
    } else {
      modifiedTime = 0;
    }
  }
}

/**
 * prevent the tty and browser from growing when the outer window is resized
 */
void EditorWidget::layout() {
  Group *tile = editor->parent();
  tile->resizable(editor);
  Group::layout();

  // when set to editor the tile is not resizable using the mouse
  tile->resizable(null);
}

/**
 * create a new editor buffer
 */
void EditorWidget::newFile() {
  if (readonly()) {
    return;
  }

  if (!checkSave(true)) {
    return;
  }

  TextBuffer *textbuf = editor->textbuf;
  filename[0] = '\0';
  textbuf->select(0, textbuf->length());
  textbuf->remove_selection();
  dirty = 0;
  textbuf->call_modify_callbacks();
  fileChanged(false);
  modifiedTime = 0;
}

/**
 * reload the editor buffer
 */
void EditorWidget::reloadFile() {
  char buffer[PATH_MAX];
  strcpy(buffer, filename);
  loadFile(buffer);
}

/**
 * replace all occurances of the given text
 */
int EditorWidget::replaceAll(const char *find, const char *replace, bool restorePos, bool matchWord) {
  int times = 0;

  if (strcmp(find, replace) != 0) {
    TextBuffer *textbuf = editor->textbuf;
    int prevPos = editor->insert_position();

    // loop through the whole string
    int pos = 0;
    editor->insert_position(pos);

    while (textbuf->search_forward(pos, find, &pos)) {
      // found a match; update the position and replace text
      if (!matchWord ||
          ((pos == 0 || !isvar(textbuf->character(pos - 1))) &&
           !isvar(textbuf->character(pos + strlen(find))))) {
        textbuf->select(pos, pos + strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);
      }
      // advance beyond replace string
      pos += strlen(replace);
      editor->insert_position(pos);
      times++;
    }

    if (restorePos) {
      editor->insert_position(prevPos);
    }
    editor->show_insert_position();
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
 * sets the current display colour
 */
void EditorWidget::setColor(const char *label, StyleField field) {
  uchar r, g, b;
  split_color(styletable[field].color, r, g, b);
  if (color_chooser(label, r, g, b)) {
    Color c = fltk::color(r, g, b);
    set_color_index(fltk::FREE_COLOR + field, c);
    styletable[field].color = c;
    editor->styleChanged();
  }
}

/**
 * sets the current command
 */
void EditorWidget::setCommand(CommandOpt command) {
  if (commandOpt == cmd_input_text) {
    wnd->setModal(false);
  }

  commandOpt = command;
  switch (command) {
  case cmd_find:
    commandChoice->label("@search; Find:");
    break;
  case cmd_find_inc:
    commandChoice->label("Inc Find:");
    break;
  case cmd_replace:
    commandChoice->label("Replace:");
    break;
  case cmd_replace_with:
    commandChoice->label("With:");
    break;
  case cmd_goto:
    commandChoice->label("Goto:");
    break;
  case cmd_input_text:
    commandChoice->label("INPUT:");
    break;
  }
  commandChoice->redraw();
  commandText->textcolor(commandChoice->textcolor());
  commandText->redraw();
  commandText->take_focus();
  commandText->when(commandOpt == cmd_find_inc ? WHEN_CHANGED : WHEN_ENTER_KEY_ALWAYS);
}

/**
 * display the toolbar modified flag
 */
void EditorWidget::setModified(bool dirty) {
  this->dirty = dirty;
  modStatus->when(dirty ? WHEN_CHANGED : WHEN_NEVER);
  modStatus->label(dirty ? "MOD" : "@line");
  modStatus->redraw();
}

/**
 * sets the foreground and background colors on the given widget
 */
void EditorWidget::setWidgetColor(Widget *w, Color bg, Color fg) {
  w->color(bg);
  w->textcolor(fg);
  w->redraw();
}

/**
 * highlight the given search text
 */
void EditorWidget::showFindText(const char *text) {
  editor->showFindText(text);
}

