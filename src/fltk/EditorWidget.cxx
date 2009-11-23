// $Id$
//
// Based on test/editor.cxx - A simple text editor program for the Fast
// Light Tool Kit (FLTK). This program is described in Chapter 4 of the FLTK
// Programmer's Guide.
// Copyright 1998-2003 by Bill Spitzak and others.
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

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
#include "smbas.h"

using namespace fltk;

// in MainWindow.cxx
extern String recentPath[];
extern Widget* recentMenu[];

// in dev_fltk.cpp
void getHomeDir(char *filename);

// in BasicEditor.cxx
extern TextDisplay::StyleTableEntry styletable[];
extern Color defaultColor[];

int completionIndex = 0;

static bool rename_active = false;
const char configFile[] = "config.txt";
const char fontConfigRead[] = "name=%[^;];size=%d\n";
const char fontConfigSave[] = "name=%s;size=%d\n";

EditorWidget* get_editor() {
  EditorWidget* result = wnd->getEditor();
  if (!result) {
    result = wnd->getEditor(true);
  }
  return result;
}

//--EditorWidget----------------------------------------------------------------

EditorWidget::EditorWidget(int x, int y, int w, int h) : Group(x, y, w, h)
{
  filename[0] = 0;
  dirty = false;
  loading = false;
  modifiedTime = 0;
  box(NO_BOX);

  begin();

  int tileHeight = h - (MNU_HEIGHT + 8);
  int ttyHeight = h / 8;
  int editHeight = tileHeight - ttyHeight;

  TiledGroup* tile = new TiledGroup(0, 0, w, tileHeight);
  tile->begin();

  editor = new BasicEditor(0, 0, w, editHeight, this);
  editor->linenumber_width(40);
  editor->wrap_mode(true, 0);
  editor->selection_color(fltk::color(190, 189, 188));
  editor->textbuf->add_modify_callback(changed_cb, this);
  editor->box(NO_BOX);
  editor->take_focus();

  tty = new TtyWidget(0, editHeight, w, ttyHeight, 1000);
  tty->color(WHITE); // bg
  tty->labelcolor(BLACK); // fg
  tile->end();

  // create the editor toolbar
  w -= 4;

  // command selection
  commandOpt = cmd_find;

  // editor status bar
  Group *statusBar = new Group(2, tty->b() + 2, w, MNU_HEIGHT);
  statusBar->box(NO_BOX);
  statusBar->begin();

  // widths become relative when the outer window is resized
  int func_bn_w = 140;
  int st_w = 40;
  int bn_w = 18;
  int st_h = statusBar->h() - 2;

  logPrintBn = new ToggleButton(w - (bn_w + 2), 2, bn_w, st_h);
  lockBn = new ToggleButton(logPrintBn->x() - (bn_w + 2), 2, bn_w, st_h);
  hideIdeBn = new ToggleButton(lockBn->x() - (bn_w + 2), 2, bn_w, st_h);
  breakLineBn = new ToggleButton(hideIdeBn->x() - (bn_w + 2), 2, bn_w, st_h);

  // sub-func jump droplist
  funcList = new Choice(breakLineBn->x() - (func_bn_w + 2), 2,
                        func_bn_w, st_h);
  funcList->callback(func_list_cb, 0);
  funcList->labelfont(HELVETICA);
  funcList->begin();
  new Item();
  new Item(SCAN_LABEL);
  funcList->end();

  colStatus = new Widget(funcList->x() - (st_w + 2), 2, st_w, st_h);
  rowStatus = new Widget(colStatus->x() - (st_w + 2), 2, st_w, st_h);
  runStatus = new Widget(rowStatus->x() - (st_w + 2), 2, st_w, st_h);
  modStatus = new Widget(runStatus->x() - (st_w + 2), 2, st_w, st_h);

  commandChoice = new Widget(0, 2, 80, st_h);
  commandText = new Input(commandChoice->r() + 2, 2,
                          modStatus->x() - commandChoice->r() - 4, st_h);
  commandText->align(ALIGN_LEFT | ALIGN_CLIP);
  commandText->callback(command_cb, (void*) 1);
  commandText->when(WHEN_ENTER_KEY_ALWAYS);
  commandText->labelfont(HELVETICA);

  for (int n = 0; n < statusBar->children(); n++) {
    Widget *w = statusBar->child(n);
    w->labelfont(HELVETICA);
    w->box(BORDER_BOX);
  }

  statusBar->resizable(commandText);
  statusBar->end();

  resizable(editor);
  end();

  setEditorColor(WHITE, true);
  loadConfig();
  setCommand(cmd_find);

  // set button callbacks
  logPrintBn->callback(log_print_cb);
  lockBn->callback(scroll_lock_cb);

  // setup icons
  logPrintBn->label("@i;@b;T");
  lockBn->label("@||;");
  hideIdeBn->label("@circle;");
  breakLineBn->label("@->;");

  // setup tooltips
  commandText->tooltip("Press Ctrl+f or Ctrl+Shift+f to find again");
  rowStatus->tooltip("Row");
  colStatus->tooltip("Column");
  runStatus->tooltip("Run status");
  modStatus->tooltip("Modified status");
  funcList->tooltip("Position the cursor to a FUNC/SUB");
  logPrintBn->tooltip("Display PRINT statements in the log window");
  lockBn->tooltip("Prevent log window auto-scrolling");
  hideIdeBn->tooltip("Hide the editor while program is running");
  breakLineBn->tooltip("Position to the last program line after BREAK");
}

EditorWidget::~EditorWidget()
{
  delete editor;
}

//--Event handler methods-------------------------------------------------------

void EditorWidget::change_case(Widget* w, void* eventData)
{
  int start, end;
  TextBuffer *tb = editor->buffer();
  char *selection;

  if (tb->selected()) {
    selection = (char *)tb->selection_text();
    tb->selection_position(&start, &end);
  }
  else {
    int pos = editor->insert_position();
    start = tb->word_start(pos);
    end = tb->word_end(pos);
    selection = (char *)tb->text_range(start, end);
  }
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
        selection[i+1] = toupper(selection[i+1]);
      }
    }
  }

  if (selection[0]) {
    tb->replace_selection(selection);
    tb->select(start, end);
  }
  free((void *)selection);
}

void EditorWidget::command_opt(Widget* w, void* eventData)
{
  setCommand((CommandOpt) (int) eventData);
}

void EditorWidget::cut_text(Widget* w, void* eventData)
{
  TextEditor::kf_cut(0, editor);
}

void EditorWidget::do_delete(Widget* w, void* eventData)
{
  editor->textbuf->remove_selection();
}

void EditorWidget::expand_word(Widget* w, void* eventData)
{
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
  }
  else {
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
  if (completionIndex != -1 &&
      searchBackward(text, start - 1, expandWord, expandWordLen, &wordPos)) {

    int matchPos = -1;
    if (textbuf->selected() == 0) {
      matchPos = wordPos;
      completionIndex = 1;      // find next word on next call
    }
    else {
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
      }
      else {
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
    const char *keyword = ((String *) keywords.get(i))->toString();
    if (strncasecmp(expandWord, keyword, expandWordLen) == 0) {
      if (firstIndex == -1) {
        firstIndex = i;
      }
      if (fullWordLen == 0) {
        if (expandWordLen == strlen(keyword)) {
          // nothing selected and word to left of cursor matches
          curIndex = i;
        }
      }
      else if (strncasecmp(fullWord, keyword, fullWordLen) == 0) {
        // selection+word to left of selection matches
        curIndex = i;
      }
      lastIndex = i;
    }
    else if (lastIndex != -1) {
      // moved beyond matching words
      break;
    }
  }

  if (lastIndex != -1) {
    if (lastIndex == curIndex || curIndex == -1) {
      lastIndex = firstIndex;   // wrap to first in subset
    }
    else {
      lastIndex = curIndex + 1;
    }

    const char *keyword = ((String *) keywords.get(lastIndex))->toString();
    // updated the segment of the replacement text
    // that completes the current selection
    if (textbuf->selected()) {
      textbuf->replace_selection(keyword + expandWordLen);
    }
    else {
      textbuf->insert(end, keyword + expandWordLen);
    }
    textbuf->select(end, end + strlen(keyword + expandWordLen));
  }
}

void EditorWidget::find(Widget* w, void* eventData)
{
  setCommand(cmd_find);
}

void EditorWidget::log_print(Widget* w, void* eventData)
{
  tty->clearScreen();
}

void EditorWidget::command(Widget* w, void* eventData)
{
  if (!readonly()) {
    bool found = false;
    bool forward = (int) eventData;
    bool updatePos = (commandOpt != cmd_find_inc);

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
    }
  }
}

void EditorWidget::font_name(Widget* w, void* eventData)
{
  setFont(fltk::font(w->label(), 0));
  wnd->updateConfig(this);
}

void EditorWidget::func_list(Widget* w, void* eventData)
{
  if (funcList && funcList->item()) {
    const char *label = funcList->item()->label();
    if (label) {
      if (strcmp(label, SCAN_LABEL) == 0) {
        funcList->clear();
        funcList->begin();
        createFuncList();
        new Item(SCAN_LABEL);
        funcList->end();
      }
      else {
        gotoLine((int) funcList->item()->user_data());
        take_focus();
      }
    }
  }
}

void EditorWidget::goto_line(Widget* w, void* eventData)
{
  setCommand(cmd_goto);  
}

void EditorWidget::paste_text(Widget* w, void* eventData)
{
  TextEditor::kf_paste(0, editor);
}

/**
 * rename the currently selected variable
 */
void EditorWidget::rename_word(Widget* w, void* eventData) {
  if (rename_active) {
    rename_active = false;
  }
  else {
    Rectangle rc;
    char* selection = getSelection(&rc);
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

void EditorWidget::replace_next(Widget* w, void* eventData)
{
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
  }
  else {
    setCommand(cmd_find);
    editor->take_focus();
  }
}

void EditorWidget::save_file(Widget* w, void* eventData)
{
  if (filename[0] == '\0') {
    // no filename - get one!
    wnd->save_file_as();
    return;
  }
  else {
    doSaveFile(filename);
  }
}

void EditorWidget::scroll_lock(Widget* w, void* eventData)
{
  tty->setScrollLock(w->flags() & STATE);
}

void EditorWidget::set_color(Widget* w, void* eventData)
{
  StyleField styleField = (StyleField) (int) eventData;
  if (styleField == st_background || styleField == st_background_def) {
    uchar r,g,b;
    split_color(editor->color(),r,g,b);
    if (color_chooser(w->label(), r,g,b)) {
      Color c = fltk::color(r,g,b);
      set_color_index(fltk::FREE_COLOR + styleField, c);
      setEditorColor(c, styleField == st_background_def);
      editor->styleChanged();
    }
  }
  else {
    setColor(w->label(), styleField);
  }
  wnd->updateConfig(this);
}

void EditorWidget::show_replace(Widget* w, void* eventData)
{
  const char* prime = editor->search;
  if (!prime || !prime[0]) {
    // use selected text when search not available
    prime = editor->textbuf->selection_text();
  }
  commandText->value(prime);
  setCommand(cmd_replace);
}

void EditorWidget::undo(Widget* w, void* eventData)
{
  TextEditor::kf_undo(0, editor);
}

//--Public methods--------------------------------------------------------------

int EditorWidget::handle(int e)
{
  switch (e) {
  case FOCUS:
    fltk::focus(editor);
    handleFileChange();
    return 1;
  case ENTER:
    if (rename_active) {
      // prevent drawing over the inplace editor child control
      return 0;
    }
  }

  return Group::handle(e);
}

bool EditorWidget::readonly()
{
  return ((BasicEditor *) editor)->readonly;
}

void EditorWidget::readonly(bool is_readonly)
{
  if (!is_readonly && access(filename, W_OK) != 0) {
    // cannot set writable since file is readonly
    is_readonly = true;
  }
  modStatus->label(is_readonly ? "RO" : "");
  modStatus->redraw();
  editor->cursor_style(is_readonly ? TextDisplay::DIM_CURSOR :
                       TextDisplay::NORMAL_CURSOR);
  ((BasicEditor *) editor)->readonly = is_readonly;
}

/**
 * copy selection text to the clipboard
 */
void EditorWidget::copyText() {
  if (!tty->copySelection()) {
    TextEditor::kf_copy(0, editor);
  }
}

bool EditorWidget::checkSave(bool discard)
{
  if (!dirty) {
    return true;  // continue next operation
  }

  const char *msg = "The current file has not been saved.\n"
                    "Would you like to save it now?";
  int r = discard ? choice(msg, "Save", "Discard", "Cancel") :
          choice(msg, "Save", "Cancel", 0);
  if (r == 0) {
    save_file();     // Save the file
    return !dirty;
  }
  return (discard && r == 1);
}

void EditorWidget::loadFile(const char *newfile)
{
  TextBuffer *textbuf = editor->textbuf;
  loading = true;
  int r = textbuf->loadfile(newfile);
  if (r) {
    // restore previous
    textbuf->loadfile(filename);
    alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
  }
  else {
    dirty = false;
    strcpy(filename, newfile);
  }

  loading = false;
  textbuf->call_modify_callbacks();
  editor->show_insert_position();
  modifiedTime = getModifiedTime();
  readonly(false);

  FileWidget::forwardSlash(filename);
  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  showPath();
  fileChanged(true);
  setRowCol(1, 1);
}

void EditorWidget::doSaveFile(const char *newfile)
{
  if (!dirty && strcmp(newfile, filename) == 0) {
    // neither buffer or filename have changed
    return;
  }

  char basfile[PATH_MAX];
  TextBuffer *textbuf = editor->textbuf;
  
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

  if (filename[0] == 0) {
    // naming a previously unnamed buffer
    wnd->addHistory(basfile);
  }
  wnd->updateEditTabName(this);
  wnd->showEditTab(this);

  textbuf->call_modify_callbacks();
  showPath();
  fileChanged(true);
  editor->take_focus();
}

void EditorWidget::gotoLine(int line)
{
  ((BasicEditor *) editor)->gotoLine(line);
}

void EditorWidget::getRowCol(int *row, int *col)
{
  return ((BasicEditor *) editor)->getRowCol(row, col);
}

void EditorWidget::getSelStartRowCol(int *row, int *col)
{
  return ((BasicEditor *) editor)->getSelStartRowCol(row, col);
}

void EditorWidget::getSelEndRowCol(int *row, int *col)
{
  return ((BasicEditor *) editor)->getSelEndRowCol(row, col);
}

void EditorWidget::saveConfig() {
  FILE *fp = wnd->openConfig(configFile);
  if (fp) {
    char buffer[MAX_PATH];
    int err;
    uchar r,g,b;

    sprintf(buffer, fontConfigSave, 
            editor->getFontName(), editor->getFontSize());
    err = fwrite(buffer, strlen(buffer), 1, fp);

    for (int i = 0; i <= st_background; i++) {
      split_color(i == st_background ? editor->color() : styletable[i].color, r,g,b);
      sprintf(buffer, "%02d=#%02x%02x%02x\n", i, r,g,b);
      err = fwrite(buffer, strlen(buffer), 1, fp);
    }
    
    fclose(fp);
  }
}

/**
 * Saves the selected text to the given file path
 */
void EditorWidget::saveSelection(const char* path) {
  int err;
  FILE *fp = fopen(path, "w");
  if (fp) {
    Rectangle rc;
    char* selection = getSelection(&rc);
    if (selection) {
      err = fwrite(selection, strlen(selection), 1, fp);
      free((void *)selection);
    }
    else {
      // save as an empty file
      fputc(0, fp);
    }
    fclose(fp);
  }
}

void EditorWidget::setHideIde() {
  hideIdeBn->value(true);
}

void EditorWidget::setFontSize(int size)
{
  editor->setFontSize(size);
  tty->setFontSize(size);
}

int EditorWidget::getFontSize()
{
  return editor->getFontSize();
}

void EditorWidget::setIndentLevel(int level)
{
  ((BasicEditor *) editor)->indentLevel = level;
}

void EditorWidget::focusWidget() {
  switch (event_key()) {
  case 'i':
    setCommand(cmd_find_inc);
    break;

  case 'f':
    if (strlen(commandText->value()) > 0 && commandOpt == cmd_find) {
      // continue search - shift -> backward else forward
      command(0, (void*)((event_key_state(LeftShiftKey) ||
                          event_key_state(RightShiftKey)) ? 0 : 1));
    }
    setCommand(cmd_find);
    break;
  }
}

// display the full pathname
void EditorWidget::showPath()
{
  commandChoice->tooltip(filename);
}

void EditorWidget::statusMsg(const char *msg)
{
  if (msg) {
    tty->print(msg);
    tty->print("\n");
  }
}

void EditorWidget::updateConfig(EditorWidget* current) {
  setFont(font(current->editor->getFontName()));
  setFontSize(current->editor->getFontSize());
  setEditorColor(current->editor->color(), false);
}

void EditorWidget::setRowCol(int row, int col)
{
  char rowcol[20];
  sprintf(rowcol, "%d", row);
  rowStatus->copy_label(rowcol);
  rowStatus->redraw();
  sprintf(rowcol, "%d", col);
  colStatus->copy_label(rowcol);
  colStatus->redraw();
}

void EditorWidget::runMsg(RunMessage runMessage)
{
  const char* msg = 0;
  switch (runMessage) {
  case msg_err:
    msg = "ERR";
    break;
  case msg_run:
    msg = "RUN";
    break;
  default:
    msg = "";
  }
  runStatus->copy_label(msg);
  runStatus->redraw();
}

void EditorWidget::fileChanged(bool loadfile)
{
  FILE *fp;

  funcList->clear();
  funcList->begin();
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
        char *c = strrchr(filename, '/');
        if (c == 0) {
          c = strrchr(filename, '\\');
        }
        recentPath[0].empty();
        recentPath[0].append(filename);
        recentMenu[0]->copy_label(c ? c + 1 : filename);
      }
    }
  }
  else {
    // empty the last edited file
    char path[MAX_PATH];
    getHomeDir(path);
    strcat(path, LASTEDIT_FILE);
    fp = fopen(path, "w");
    if (!fwrite("\n", 1, 1, fp)) {
      // write error
    }
    fclose(fp);
  }

  new Item(SCAN_LABEL);
  funcList->end();
}

void EditorWidget::restoreEdit()
{
  FILE *fp;
  char path[MAX_PATH];

  // continue editing the previous file
  getHomeDir(path);
  strcat(path, LASTEDIT_FILE);
  fp = fopen(path, "r");
  if (fp) {
    if (fgets(path, sizeof(path), fp)) {
      path[strlen(path) - 1] = 0; // trim new-line
    }
    fclose(fp);
    if (access(path, 0) == 0) {
      loadFile(path);
      return;
    }
  }

  // continue editing scratch buffer
  getHomeDir(path);
  strcat(path, UNTITLED_FILE);
  if (access(path, 0) == 0) {
    loadFile(path);
  }
}

//--Protected methods-----------------------------------------------------------

void EditorWidget::createFuncList()
{
  TextBuffer *textbuf = editor->textbuf;
  const char *text = textbuf->text();
  int len = textbuf->length();
  int curLine = 0;

  for (int i = 0; i < len; i++) {
    if (text[i] == '\n' || i == 0) {
      curLine++;
    }

    // skip ahead to the next line when comments found
    if (text[i] == '#' || text[i] == '\'' || 
        strncasecmp(text + i, "rem", 3) == 0) {
      while (i < len && text[i] != '\n') {
        i++;
      }
      curLine++;
    }

    // avoid seeing "gosub" etc
    int offs = ((strncasecmp(text + i, "\nsub ", 5) == 0 ||
                 strncasecmp(text + i, " sub ", 5) == 0) ? 4 :
                (strncasecmp(text + i, "\nfunc ", 6) == 0 ||
                 strncasecmp(text + i, " func ", 6) == 0) ? 5 : 0);
    if (offs != 0) {
      char *c = strchr(text + i + offs, '\n');
      if (c) {
        if (text[i] == '\n') {
          i++;    // skip initial newline
        }
        int itemLen = c - (text + i);
        String s(text + i, itemLen);
        Item *item = new Item();
        item->copy_label(s.toString());
        item->user_data((void*) curLine);
        i += itemLen;
        curLine++;
      }
    }
  }
}

void EditorWidget::doChange(int inserted, int deleted)
{
  if (loading) {
    return;  // do nothing while file load in progress
  }

  if (inserted || deleted) {
    dirty = 1;
  }

  setModified(dirty);
}

void EditorWidget::findFunc(const char *find)
{
  const char *text = editor->textbuf->text();
  int findLen = strlen(find);
  int len = editor->textbuf->length();
  int lineNo = 1;
  for (int i = 0; i < len; i++) {
    if (strncasecmp(text + i, find, findLen) == 0) {
      gotoLine(lineNo);
      break;
    }
    else if (text[i] == '\n') {
      lineNo++;
    }
  }
}

char* EditorWidget::getSelection(Rectangle* rc)
{
  return ((BasicEditor *) editor)->getSelection(rc);
}

U32 EditorWidget::getModifiedTime() {
  struct stat st_file;
  U32 modified = 0;
  if (filename[0] && !stat(filename, &st_file)) {
    modified = st_file.st_mtime;
  }
  return modified;
}

void EditorWidget::handleFileChange() {
  // handle outside changes to the file
  if (filename[0] && modifiedTime != 0 &&
      modifiedTime != getModifiedTime()) {
    const char *msg = "File %s\nhas changed on disk.\n\n"
      "Do you want to reload the file?";
    if (ask(msg, filename)) {
      reloadFile();
    }
    else {
      modifiedTime = 0;
    }
  }
}

/**
 * load any stored font or color settings
 */
void EditorWidget::loadConfig() {
  FILE *fp = wnd->openConfig(configFile, "r");
  if (fp) {
    char buffer[MAX_PATH];
    int size = 0;
    int i = 0;

    if (fscanf(fp, fontConfigRead, buffer, &size) == 2) {
      setFont(font(buffer));
      setFontSize(size);
    }

    while (feof(fp) == 0 && fgets(buffer, sizeof(buffer), fp)) {
      buffer[strlen(buffer) - 1] = 0; // trim new-line
      Color c = fltk::color(buffer + 3); // skip nn=#xxxxxx
      if (c != NO_COLOR) {
        if (i == st_background) {
          setEditorColor(c, false);
          break; // found final StyleField element
        }
        else {
          styletable[i].color = c;
        }
      }
      i++;
    }

    fclose(fp);
  }
}

void EditorWidget::newFile()
{
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

void EditorWidget::reloadFile() {
  char buffer[PATH_MAX];
  strcpy(buffer, filename);
  loadFile(buffer);
}

int EditorWidget::replaceAll(const char* find, const char* replace, 
                             bool restorePos, bool matchWord)
{
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

bool EditorWidget::searchBackward(const char *text, int startPos,
                                  const char *find, int findLen, int *foundPos)
{
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

void EditorWidget::setColor(const char* label, StyleField field) {
  uchar r,g,b;
  split_color(styletable[field].color,r,g,b);
  if (color_chooser(label, r,g,b)) {
    Color c = fltk::color(r,g,b);
    set_color_index(fltk::FREE_COLOR + field, c);
    styletable[field].color = c;
    editor->styleChanged();
  }
} 

void EditorWidget::setCommand(CommandOpt command) {
  commandOpt = command;
  switch (command) {
  case cmd_find:
    commandChoice->label("Find:");
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
  }
  commandChoice->redraw();
  commandText->textcolor(commandChoice->textcolor());
  commandText->redraw();
  commandText->take_focus();
  commandText->when(commandOpt == cmd_find_inc ? 
                    WHEN_CHANGED : WHEN_ENTER_KEY_ALWAYS);
}

/**
 * Sets the editor and editor toolbar color
 */
void EditorWidget::setEditorColor(Color c, bool defColor) {
  editor->color(c);

  Color bg = lerp(c, BLACK, .1f); // same offset as editor line numbers
  Color fg = contrast(c, bg);
  int i;
  Widget* child;

  // set the colours on the command text bar
  for (i = commandText->parent()->children(); i > 0; i--) {
    child = commandText->parent()->child(i - 1);
    child->color(bg);
    child->textcolor(fg);
    child->redraw();
  }
  // set the colours on the status bar
  for (i = rowStatus->parent()->children(); i > 0; i--) {
    child = rowStatus->parent()->child(i - 1);
    child->color(bg);
    child->labelcolor(fg);
    child->redraw();
  }
  if (defColor) {
    // contrast the default colours against the background
    for (i = 0; i < st_background; i++) {
      styletable[i].color = contrast(defaultColor[i], c);
    }
  }
}

void EditorWidget::setFont(Font* font)
{
  if (font) {
    editor->setFont(font);
    tty->setFont(font);
  }
}

void EditorWidget::setModified(bool dirty)
{
  this->dirty = dirty;
  modStatus->label(dirty ? "MOD" : "");
  modStatus->redraw();
}

void EditorWidget::showFindText(const char *text) {
  editor->showFindText(text);
}

//--EndOfFile-------------------------------------------------------------------
