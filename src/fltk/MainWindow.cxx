// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "sys.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>

#include <fltk/Choice.h>
#include <fltk/Group.h>
#include <fltk/Item.h>
#include <fltk/MenuBar.h>
#include <fltk/TabGroup.h>
#include <fltk/TiledGroup.h>
#include <fltk/ValueInput.h>
#include <fltk/Window.h>
#include <fltk/ask.h>
#include <fltk/error.h>
#include <fltk/events.h>
#include <fltk/filename.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "EditorWindow.h"
#include "HelpWidget.h"
#include "sbapp.h"
#include "StringLib.h"

#if defined(WIN32)
#include <fltk/win32.h>
#include <shellapi.h>
#endif

extern "C" {
#include "fs_socket_client.h"
}
#ifndef MAX_PATH
#define MAX_PATH 256
#endif
using namespace fltk;

#define DEF_FONT_SIZE 12
#define SCAN_LABEL "-[ Refresh ]-"
#define NUM_RECENT_ITEMS 9

char path[MAX_PATH];
char* packageHome;
char* runfile = 0;
int completionIndex = 0;
int recentIndex = 0;
int restart = 0;
Widget* recentMenu[NUM_RECENT_ITEMS];
String recentPath[NUM_RECENT_ITEMS];
int recentPosition[NUM_RECENT_ITEMS];
MainWindow* wnd;
ExecState runMode = init_state;

const char* basHome = "BAS_HOME=";
const char* pluginHome = "plugins";
const char untitledFile[] = "untitled.bas";
const char lasteditFile[] = "lastedit.txt";
const char historyFile[] = "history.txt";
const char keywordsFile[] = "keywords.txt";
const char aboutText[] =
  "<b>About SmallBASIC...</b><br><br>"
  "Copyright (c) 2000-2006 Nicholas Christopoulos.<br><br>"
  "FLTK Version " SB_STR_VER "<br>"
  "Copyright (c) 2002-2008 Chris Warren-Smith.<br><br>"
  "<a href=http://smallbasic.sourceforge.net>"
  "http://smallbasic.sourceforge.net</a><br><br>"
  "SmallBASIC comes with ABSOLUTELY NO WARRANTY. "
  "This program is free software; you can use it "
  "redistribute it and/or modify it under the terms of the "
  "GNU General Public License version 2 as published by "
  "the Free Software Foundation.<br><br>" "<i>Press F1 for help";

// in dev_fltk.cpp
void getHomeDir(char *filename);
bool cacheLink(dev_file_t * df, char *localFile);
void updateForm(const char *s);
void closeForm();
bool isFormActive();

//--EditWindow functions--------------------------------------------------------
void MainWindow::setRowCol(int row, int col)
{
  char rowcol[20];
  sprintf(rowcol, "%d", row);
  rowStatus->copy_label(rowcol);
  rowStatus->redraw();
  sprintf(rowcol, "%d", col);
  colStatus->copy_label(rowcol);
  colStatus->redraw();
}

void MainWindow::setModified(bool dirty)
{
  modStatus->label(dirty ? "MOD" : "");
  modStatus->redraw();
}

void MainWindow::statusMsg(const char *msg)
{
  const char *filename = editWnd->getFilename();
  fileStatus->copy_label(msg && msg[0] ? msg :
                         filename && filename[0] ? filename : untitledFile);
  fileStatus->labelcolor(rowStatus->labelcolor());
  fileStatus->redraw();
}

void MainWindow::runMsg(RunMessage runMessage)
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

void MainWindow::busyMessage()
{
  statusMsg("Selection unavailable while program is running.");
}

void MainWindow::pathMessage(const char *file)
{
  sprintf(path, "File not found: %s", file);
  statusMsg(path);
}

void MainWindow::showEditTab()
{
  tabGroup->selected_child(editGroup);
  editWnd->take_focus();
}

void MainWindow::showHelpTab()
{
  tabGroup->selected_child(helpGroup);
}

void MainWindow::showOutputTab()
{
  tabGroup->selected_child(outputGroup);
}

void MainWindow::updatePath(char *filename)
{
  int len = filename ? strlen(filename) : 0;
  for (int i = 0; i < len; i++) {
    if (filename[i] == '\\') {
      filename[i] = '/';
    }
  }
}

void MainWindow::execInit()
{
  // execute the initialisation program
  getHomeDir(path);
  strcat(path, "init.bas");
  if (access(path, 0) == 0) {
    int success = sbasic_main(path);
    runMsg(success ? msg_none : msg_err);
  }
  editWnd->take_focus();
}

void MainWindow::restoreEdit()
{
  FILE *fp;

  // continue editing the previous file
  getHomeDir(path);
  strcat(path, lasteditFile);
  fp = fopen(path, "r");
  if (fp) {
    fgets(path, sizeof(path), fp);
    fclose(fp);
    path[strlen(path) - 1] = 0; // trim new-line
    if (access(path, 0) == 0) {
      editWnd->loadFile(path, -1, false);
      statusMsg(path);
      fileChanged(true);
      return;
    }
  }

  // continue editing scratch buffer
  getHomeDir(path);
  strcat(path, untitledFile);
  if (access(path, 0) == 0) {
    editWnd->loadFile(path, -1, false);
    statusMsg(path);
    fileChanged(true);
  }
}

void MainWindow::saveLastEdit(const char *filename)
{
  // remember the last edited file
  getHomeDir(path);
  strcat(path, lasteditFile);
  FILE *fp = fopen(path, "w");
  if (fp) {
    fwrite(filename, strlen(filename), 1, fp);
    fwrite("\n", 1, 1, fp);
    fclose(fp);
  }
}

void MainWindow::setHideEditor() {
  isHideEditor = true;

  // update the menu
  ((Menu*) ((Menu*) child(0))->find("Program/Toggle/Hide Editor"))->set();
}

void MainWindow::addHistory(const char *filename)
{
  FILE *fp;
  char buffer[MAX_PATH];
  char updatedfile[MAX_PATH];

  int len = strlen(filename);
  if (strcasecmp(filename + len - 4, ".sbx") == 0) {
    // don't remember bas exe files
    return;
  }

  // save paths with unix path separators
  strcpy(updatedfile, filename);
  updatePath(updatedfile);
  filename = updatedfile;

  // remember the last edited file
  saveLastEdit(filename);

  // save into the history file
  getHomeDir(path);
  strcat(path, historyFile);

  fp = fopen(path, "r");
  if (fp) {
    // don't add the item if it already exists
    while (feof(fp) == 0) {
      if (fgets(buffer, sizeof(buffer), fp) &&
          strncmp(filename, buffer, strlen(filename) - 1) == 0) {
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

void MainWindow::fileChanged(bool loadfile)
{
  FILE *fp;

  funcList->clear();
  funcList->begin();
  if (loadfile) {
    // update the func/sub navigator
    editWnd->createFuncList();
    funcList->redraw();

    const char *filename = editWnd->getFilename();
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
    getHomeDir(path);
    strcat(path, lasteditFile);
    fp = fopen(path, "w");
    fwrite("\n", 1, 1, fp);
    fclose(fp);
  }

  new Item(SCAN_LABEL);
  funcList->end();
}

// run the give file. returns whether break was hit
bool MainWindow::basicMain(const char *filename, bool toolExec)
{
  int len = strlen(filename);
  if (strcasecmp(filename + len - 4, ".htm") == 0 ||
      strcasecmp(filename + len - 5, ".html") == 0) {
    // render html edit buffer
    sprintf(path, "file:%s", filename);
    updateForm(path);
    editWnd->take_focus();
    return false;
  }
  if (access(filename, 0) != 0) {
    pathMessage(filename);
    runMode = edit_state;
    runMsg(msg_err);
    return false;
  }

  editWnd->readonly(true);
  runMsg(msg_run);

  opt_pref_width = 0;
  opt_pref_height = 0;

  Window* fullScreen = NULL;
  Group* oldOutputGroup = outputGroup;
  int old_w = out->w();
  int old_h = out->h();

  if (!toolExec && isHideEditor) {
    fullScreen = new BaseWindow(w(), h());
    fullScreen->copy_label(filename);
    fullScreen->callback(quit_cb);
    fullScreen->shortcut(0);
    fullScreen->add(out);
    fullScreen->resizable(fullScreen);
    outputGroup = fullScreen;
    out->resize(w(), h());
    hide();
  }
  else {
    copy_label("SmallBASIC");
  }

  int success;
  do {
    restart = false;
    runMode = run_state;
    success = sbasic_main(filename);
  }
  while (restart);
  
  bool was_break = (runMode == break_state);

  if (fullScreen != NULL) {
    fullScreen->remove(out);
    delete fullScreen;

    outputGroup = oldOutputGroup;
    outputGroup->add(out);
    out->resize(old_w, old_h);
    show();
  }

  if (runMode == quit_state) {
    exit(0);
  }

  if (!success || was_break) {
    if (!toolExec) {
      editWnd->gotoLine(gsb_last_line);
    }
    int len = strlen(gsb_last_errmsg);
    if (gsb_last_errmsg[len - 1] == '\n') {
      gsb_last_errmsg[len - 1] = 0;
    }
    closeForm();  // unhide the error
    showEditTab();
    statusMsg(gsb_last_errmsg);
    fileStatus->labelcolor(RED);
    runMsg(was_break ? msg_none : msg_err);
  }
  else {
    statusMsg(editWnd->getFilename());
    runMsg(msg_none);
  }

  editWnd->readonly(false);
  runMode = edit_state;
  return was_break;
}

bool searchBackward(const char *text, int startPos,
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

//--Menu callbacks--------------------------------------------------------------

void MainWindow::restart_run(Widget* w, void* eventData) {
  brun_break();
  restart = true;
  runMode = break_state;
}

void MainWindow::quit(Widget* w, void* eventData)
{
  if (runMode == edit_state || runMode == quit_state) {

    // auto-save scratchpad
    const char *filename = editWnd->getFilename();
    int offs = strlen(filename) - strlen(untitledFile);
    if (filename[0] == 0 ||
        (offs > 0 && strcasecmp(filename + offs, untitledFile) == 0)) {
      getHomeDir(path);
      strcat(path, untitledFile);
      editWnd->doSaveFile(path, 0);
      exit(0);
    }

    if (editWnd->checkSave(true)) {
      exit(0);
    }
  }
  else {
    switch (choice("Terminate running program?", "Exit", "Break", "Cancel")) {
    case 0:
      exit(0);
    case 1:
      brun_break();
      runMode = break_state;
    }
  }
}

void MainWindow::help_home(Widget* w, void* eventData)
{
  strcpy(path, "http://smallbasic.sf.net");
  browseFile(path);
}

// display the results of help.bas
void MainWindow::showHelpPage() {
  showHelpTab();
  getHomeDir(path);
  helpWnd->setDocHome(path);
  strcat(path, "help.html");
  helpWnd->loadFile(path);
}

void MainWindow::execHelp() {
  if (strncmp(opt_command, "http://", 7) == 0) {
    // launch in real browser
    browseFile(opt_command);
  }
  else {
    sprintf(path, "%s/%s/help.bas", packageHome, pluginHome);
    basicMain(path, true);
    statusMsg(editWnd->getFilename());
  }
}

// handle click from within help window
void do_help_contents_anchor(void *)
{
  fltk::remove_check(do_help_contents_anchor);
  strcpy(opt_command, wnd->helpWnd->getEventName());
  wnd->execHelp();
  wnd->showHelpPage();
}

void MainWindow::help_contents_anchor(Widget* w, void* eventData) {
  if (runMode == edit_state) {
    fltk::add_check(do_help_contents_anchor);
  }
}

// handle f1 context help
void MainWindow::help_contents(Widget* w, void* eventData)
{
  if (runMode == edit_state) {
    if (event_key() != 0) {
      // scan for help context
      TextEditor *editor = editWnd->editor;
      TextBuffer *tb = editor->buffer();
      int pos = editor->insert_position();
      int start = tb->word_start(pos);
      int end = tb->word_end(pos);
      char *selection = tb->text_range(start, end);

      strcpy(opt_command, selection);
      free((void *)selection);
    }
    else {
      opt_command[0] = 0;
    }

    execHelp();
  }

  showHelpPage();
}

void MainWindow::help_app(Widget* w, void* eventData)
{
  const char *helpFile = dev_getenv("APP_HELP");
  if (helpFile) {
    helpWnd->loadFile(helpFile);
  }
  else {
    helpWnd->loadBuffer("APP_HELP env variable not found");
  }
  showHelpTab();
}

void MainWindow::help_about(Widget* w, void* eventData)
{
  helpWnd->loadBuffer(aboutText);
  showHelpTab();
}

void MainWindow::run_break(Widget* w, void* eventData)
{
  if (runMode == run_state || runMode == modal_state) {
    runMode = break_state;
  }
}

void MainWindow::set_options(Widget* w, void* eventData)
{
  const char* args = fltk::input("Enter program command line", opt_command);
  if (args) {
    strcpy(opt_command, args);
  }
}

void MainWindow::hide_editor(Widget* w, void* eventData)
{
  isHideEditor = (w->flags() & STATE);
}

void MainWindow::next_tab(Widget* w, void* eventData)
{
  Widget *current = tabGroup->selected_child();
  // cycle around the main tabgroups
  if (current == helpGroup) {
    tabGroup->selected_child(outputGroup);
  }
  else if (current == outputGroup) {
    tabGroup->selected_child(editGroup);
    editWnd->take_focus();
  }
  else {
    tabGroup->selected_child(helpGroup);
  }
}

void MainWindow::copy_text(Widget* w, void* eventData)
{
  // copy from the active tab
  if (editGroup == tabGroup->selected_child()) {
    EditorWindow::copy_cb(0, eventData);
  }
  else {
    handle(EVENT_COPY_TEXT);
  }
}

void MainWindow::cut_text(Widget* w, void* eventData)
{
  if (editGroup == tabGroup->selected_child()) {
    EditorWindow::cut_cb(0, eventData);
  }
}

void MainWindow::paste_text(Widget* w, void* eventData)
{
  if (editGroup == tabGroup->selected_child()) {
    EditorWindow::paste_cb(0, eventData);
  }
}

void MainWindow::turbo(Widget* w, void* eventData)
{
  isTurbo = (w->flags() & STATE);
}

void MainWindow::find(Widget* w, void* eventData)
{
  bool found = editWnd->findText(findText->value(), (int)eventData);
  findText->textcolor(found ? BLACK : RED);
  findText->redraw();
  if (2 == (int)eventData) {
    editWnd->take_focus();
  }
}

void MainWindow::goto_line(Widget* w, void* eventData)
{
  Input *gotoLine = (Input *) eventData;
  editWnd->gotoLine(atoi(gotoLine->value()));
  editWnd->take_focus();
}

void MainWindow::font_size_incr(Widget* w, void* eventData)
{
  Widget *current = tabGroup->selected_child();
  if (current == editGroup) {
    int size = editWnd->getFontSize();
    if (size < MAX_FONT_SIZE) {
      editWnd->setFontSize(size + 1);
      out->setFontSize(size + 1);
    }
  }
  else {
    handle(EVENT_INCREASE_FONT);
  }
}

void MainWindow::font_size_decr(Widget* w, void* eventData)
{
  Widget *current = tabGroup->selected_child();
  if (current == editGroup) {
    int size = editWnd->getFontSize();
    if (size > MIN_FONT_SIZE) {
      editWnd->setFontSize(size - 1);
      out->setFontSize(size - 1);
    }
  }
  else {
    handle(EVENT_DECREASE_FONT);
  }
}

void MainWindow::func_list(Widget* w, void* eventData)
{
  const char *label = funcList->item()->label();
  if (label) {
    if (strcmp(label, SCAN_LABEL) == 0) {
      funcList->clear();
      funcList->begin();
      editWnd->createFuncList();
      new Item(SCAN_LABEL);
      funcList->end();
    }
    else {
      editWnd->findFunc(label);
      editWnd->take_focus();
    }
  }
}

void MainWindow::run(Widget* w, void* eventData)
{
  const char *filename = editWnd->getFilename();
  if (runMode == edit_state) {
    // inhibit autosave on run function with environment var
    const char *noSave = dev_getenv("NO_RUN_SAVE");
    if (noSave == 0 || noSave[0] != '1') {
      if (filename == 0 || filename[0] == 0) {
        getHomeDir(path);
        strcat(path, untitledFile);
        filename = path;
        editWnd->doSaveFile(filename, false);
      }
      else {
        editWnd->doSaveFile(filename, true);
      }
    }
    showOutputTab();
    basicMain(filename, false);
  }
  else {
    busyMessage();
  }
}

// callback for editor-plug-in plug-ins. we assume the target
// program will be changing the contents of the editor buffer
void MainWindow::editor_plugin(Widget* w, void* eventData)
{
  char filename[256];
  TextEditor *editor = editWnd->editor;
  strcpy(filename, editWnd->getFilename());

  if (runMode == edit_state) {
    if (editWnd->checkSave(false) && filename[0]) {
      int pos = editor->insert_position();
      int row, col, s1r, s1c, s2r, s2c;
      editWnd->getRowCol(&row, &col);
      editWnd->getSelStartRowCol(&s1r, &s1c);
      editWnd->getSelEndRowCol(&s2r, &s2c);
      sprintf(opt_command, "%s|%d|%d|%d|%d|%d|%d",
              filename, row - 1, col, s1r - 1, s1c, s2r - 1, s2c);
      runMode = run_state;
      runMsg(msg_run);
      sprintf(path, "%s/%s", packageHome, (const char *)eventData);
      int success = sbasic_main(path);
      runMsg(success ? msg_none : msg_err);
      editWnd->loadFile(filename, -1, true);
      editor->insert_position(pos);
      editor->show_insert_position();
      showEditTab();
      runMode = edit_state;
      opt_command[0] = 0;
    }
  }
  else {
    busyMessage();
  }
}

void MainWindow::tool_plugin(Widget* w, void* eventData)
{
  if (runMode == edit_state) {
    sprintf(opt_command, "%s/%s", packageHome, pluginHome);
    statusMsg((const char *)eventData);
    sprintf(path, "%s/%s", packageHome, (const char *)eventData);
    showOutputTab();
    basicMain(path, true);
    statusMsg(editWnd->getFilename());
    opt_command[0] = 0;
  }
  else {
    busyMessage();
  }
}

void MainWindow::change_case(Widget* w, void* eventData)
{
  int start, end;
  TextEditor *editor = editWnd->editor;
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

void MainWindow::expand_word(Widget* w, void* eventData)
{
  int start, end;
  const char *fullWord = 0;
  unsigned fullWordLen = 0;
  TextEditor *editor = editWnd->editor;
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
  editWnd->getKeywords(keywords);

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

void MainWindow::load_file(Widget* w, void* eventData)
{
  if (editWnd->checkSave(true)) {
    TextEditor *editor = editWnd->editor;
    // save current position
    recentPosition[recentIndex] = editor->insert_position();
    recentIndex = ((int)eventData) - 1;
    // load selected file
    const char *path = recentPath[recentIndex].toString();
    if (access(path, 0) == 0) {
      editWnd->loadFile(path, -1, false);
      // restore previous position
      editor->insert_position(recentPosition[recentIndex]);
      statusMsg(path);
      fileChanged(true);
      saveLastEdit(path);
      showEditTab();
    }
    else {
      pathMessage(path);
    }
  }
}

//--Startup functions-----------------------------------------------------------

void MainWindow::scanRecentFiles(Menu * menu)
{
  FILE *fp;
  char buffer[MAX_PATH];
  char label[1024];
  int i = 0;

  getHomeDir(path);
  strcat(path, historyFile);
  fp = fopen(path, "r");
  if (fp) {
    while (feof(fp) == 0 && fgets(buffer, sizeof(buffer), fp)) {
      buffer[strlen(buffer) - 1] = 0; // trim new-line
      if (access(buffer, 0) == 0) {
        char *fileLabel = strrchr(buffer, '/');
        if (fileLabel == 0)
          fileLabel = strrchr(buffer, '\\');
        fileLabel = fileLabel ? fileLabel + 1 : buffer;
        if (fileLabel != 0 && *fileLabel == '_')
          fileLabel++;
        sprintf(label, "&File/Open Recent File/%s", fileLabel);
        recentMenu[i] = menu->add(label, CTRL + '1' + i, (Callback *)
                                  load_file_cb, (void *)(i + 1), RAW_LABEL);
        recentPath[i].append(buffer);
        if (++i == NUM_RECENT_ITEMS) {
          break;
        }
      }
    }
    fclose(fp);
  }
  while (i < NUM_RECENT_ITEMS) {
    sprintf(label, "&File/Open Recent File/%s", untitledFile);
    recentMenu[i] = menu->add(label, CTRL + '1' + i, (Callback *)
                              load_file_cb, (void *)(i + 1));
    recentPath[i].append(untitledFile);
    i++;
  }
}

void MainWindow::scanPlugIns(Menu* menu)
{
  FILE *file;
  char buffer[MAX_PATH];
  char label[1024];
  DIR *dp;
  struct dirent *e;

  snprintf(path, sizeof(path), "%s/%s", packageHome, pluginHome);
  for (dp = opendir(path); (e = readdir(dp)) != NULL;) {
    const char* filename = e->d_name;
    int len = strlen(filename);
    if (strcasecmp(filename + len - 4, ".bas") == 0) {
      sprintf(path, "%s/%s/%s", packageHome, pluginHome, filename);
      file = fopen(path, "r");
      if (!file) {
        continue;
      }

      if (!fgets(buffer, 1024, file)) {
        fclose(file);
        continue;
      }
      bool editorTool = false;
      if (strcmp("'tool-plug-in\n", buffer) == 0) {
        editorTool = true;
      }
      else if (strcmp("'app-plug-in\n", buffer) != 0) {
        fclose(file);
        continue;
      }

      if (fgets(buffer, 1024, file) && strncmp("'menu", buffer, 5) == 0) {
        int offs = 6;
        buffer[strlen(buffer) - 1] = 0; // trim new-line
        while (buffer[offs] && (buffer[offs] == '\t' || buffer[offs] == ' ')) {
          offs++;
        }
        sprintf(label, (editorTool ? "&Edit/Basic/%s" : "&Basic/%s"), buffer + offs);
        // use an absolute path
        sprintf(path, "%s/%s", pluginHome, filename);
        menu->add(label, 0, (Callback *)
                  (editorTool ? MainWindow::editor_plugin_cb : MainWindow::tool_plugin_cb),
                  strdup(path));
      }
      fclose(file);
    }
  }
  // cleanup
  closedir(dp);
}

int arg_cb(int argc, char **argv, int &i)
{
  const char *s = argv[i];
  int len = strlen(s);

  if (strcasecmp(s + len - 4, ".bas") == 0 && access(s, 0) == 0) {
    runfile = strdup(s);
    runMode = run_state;
    i += 1;
    return 1;
  }

  if (argv[i][0] == '-'  && !argv[i][2] && argv[i + 1]) {
    switch (argv[i][1]) {
    case 'e':
      runfile = strdup(argv[i + 1]);
      runMode = edit_state;
      i += 2;
      return 1;

    case 'r':
      runfile = strdup(argv[i + 1]);
      runMode = run_state;
      i += 2;
      return 1;

    case 'm':
      opt_loadmod = 1;
      strcpy(opt_modlist, argv[i + 1]);
      i += 2;
      return 1;
    }
  }


  if (argv[i][0] == '-' && argv[i][1] == '-') {
    // echo foo | sbasic foo.bas --
    int c;
    while ((c = fgetc(stdin)) != EOF) {
      int len = strlen(opt_command);
      opt_command[len] = c;
      opt_command[len + 1] = 0;
    }
    i++;
    return 1;
  }

  if (runMode == run_state) {
    if (opt_command[0]) {
      strcat(opt_command, " ");
    }
    strcat(opt_command, s);
    i++;
    return 1;
  }

  return 0;
}

int main(int argc, char **argv)
{
  int i = 0;
  if (args(argc, argv, i, arg_cb) < argc) {
    fatal("Options are:\n"
          " -e[dit] file.bas\n"
          " -r[run] file.bas\n"
          " -m[odule]-home\n\n%s", help);
  }

  // package home contains installed components
#if defined(WIN32)
  getcwd(path, sizeof(path));
  packageHome = strdup(path);
#else
  packageHome = (char*)PACKAGE_DATA_DIR;
#endif
  sprintf(path, "PKG_HOME=%s", packageHome);
  dev_putenv(path);

  // bas_home contains user editable files along with generated help
  strcpy(path, basHome);
  getHomeDir(path + strlen(basHome));
  dev_putenv(path);

  wnd = new MainWindow(600, 500);

  // setup styles
  Font* defaultFont = font("arial");
  if (defaultFont) {
    Widget::default_style->labelfont(defaultFont);
    Button::default_style->labelfont(defaultFont);
    Widget::default_style->textfont(defaultFont);
    Button::default_style->textfont(defaultFont);
  }

#if defined(WIN32)
  HICON icon = (HICON) wnd->icon();
  if (!icon) {
    icon = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(101),
                             IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_SHARED);
    if (!icon) {
      icon = LoadIcon(NULL, IDI_APPLICATION);
    }
  }
  wnd->icon((char *)icon);
#endif

  check();
  wnd->execInit();

  switch (runMode) {
  case run_state:
    wnd->setHideEditor();
    wnd->editWnd->loadFile(runfile, -1, true);
    wnd->addHistory(runfile);
    wnd->showOutputTab();
    if (!wnd->basicMain(runfile, false)) {
      return 0; // continue if break hit
    }
  case edit_state:
    wnd->editWnd->loadFile(runfile, -1, true);
    break;
  default:
    wnd->restoreEdit();
    runMode = edit_state;
  }

  wnd->show(argc, argv);
  return run();
}

//--MainWindow methods----------------------------------------------------------

MainWindow::MainWindow(int w, int h) : BaseWindow(w, h)
{
  int mnuHeight = 22;
  int tbHeight = 26;
  int statusHeight = mnuHeight;
  int groupHeight = h - mnuHeight - statusHeight - 3;
  int tabBegin = 0;             // =mnuHeight for top position tabs
  int pageHeight = groupHeight - mnuHeight;

  isTurbo = false;
  isHideEditor = false;
  opt_graphics = 1;
  opt_quiet = 1;
  opt_interactive = 0;
  opt_nosave = 1;
  opt_ide = IDE_NONE;           // for sberr.c
  opt_pref_bpp = 0;
  os_graphics = 1;

  updatePath(runfile);
  begin();
  MenuBar *m = new MenuBar(0, 0, w, mnuHeight);
  m->add("&File/&New File", 0, (Callback *) EditorWindow::newFile_cb);
  m->add("&File/&Open File...", CTRL + 'o', (Callback *) EditorWindow::openFile_cb);
  scanRecentFiles(m);
  m->add("&File/_&Insert File...", CTRL + 'i', (Callback *) EditorWindow::insertFile_cb);
  m->add("&File/&Save File", CTRL + 's', (Callback *) EditorWindow::saveFile_cb);
  m->add("&File/_Save File &As...", CTRL + SHIFT + 'S',
         (Callback *) EditorWindow::saveFileAs_cb);
  m->add("&File/E&xit", CTRL + 'q', (Callback *) MainWindow::quit_cb);
  m->add("&Edit/_&Undo", CTRL + 'z', (Callback *) EditorWindow::undo_cb);
  m->add("&Edit/Cu&t", CTRL + 'x', (Callback *) MainWindow::cut_text_cb);
  m->add("&Edit/&Copy", CTRL + 'c', (Callback *) MainWindow::copy_text_cb);
  m->add("&Edit/_&Paste", CTRL + 'v', (Callback *) MainWindow::paste_text_cb);
  m->add("&Edit/&Change Case", ALT + 'c', (Callback *) MainWindow::change_case_cb);
  m->add("&Edit/_&Expand Word", ALT + '/', (Callback *) MainWindow::expand_word_cb);
  m->add("&Edit/&Replace...", F2Key, (Callback *) EditorWindow::replaceAll_cb);
  m->add("&Edit/_Replace &Again", CTRL + 't',
         (Callback *) EditorWindow::replaceNext_cb);
  m->add("&View/_&Next Tab", F6Key, (Callback *) MainWindow::next_tab_cb);
  m->add("&View/Text Size/&Increase", CTRL + ']', (Callback *) MainWindow::font_size_incr_cb);
  m->add("&View/Text Size/&Decrease", CTRL + '[', (Callback *) MainWindow::font_size_decr_cb);
  scanPlugIns(m);
  m->add("&Program/_&Run", F9Key, (Callback *) run_cb);
  m->add("&Program/&Break", CTRL + 'b', (Callback *) MainWindow::run_break_cb);
  m->add("&Program/_&Restart", CTRL + 'r', (Callback *) MainWindow::restart_run_cb);
  m->add("&Program/&Command", F10Key, (Callback *) MainWindow::set_options_cb);
  m->add("&Program/Toggle/&Turbo", 0, (Callback *) turbo_cb)->type(Item::TOGGLE);
  m->add("&Program/Toggle/&Hide Editor", 0,
         (Callback *) hide_editor_cb)->type(Item::TOGGLE);
  m->add("&Help/&Help Contents", F1Key, (Callback *) MainWindow::help_contents_cb);
  m->add("&Help/_&Program Help", F11Key, (Callback *) MainWindow::help_app_cb);
  m->add("&Help/_&Home Page", 0, (Callback *) MainWindow::help_home_cb);
  m->add("&Help/&About SmallBASIC", F12Key, (Callback *) MainWindow::help_about_cb);

  callback(quit_cb);
  shortcut(0);                  // remove default EscapeKey shortcut

  tabGroup = new TabGroup(0, mnuHeight, w, groupHeight);
  tabGroup->begin();

  editGroup = new Group(0, tabBegin, w, pageHeight, "Editor");
  editGroup->begin();
  editGroup->box(THIN_DOWN_BOX);

  // create the editor edit window
  editWnd = new EditorWindow(2, 2, w - 4, pageHeight - tbHeight - 5);
  m->user_data(editWnd);        // the EditorWindow is callback user data
                                // (void*)
  editWnd->box(NO_BOX);
  editWnd->editor->box(NO_BOX);
  editGroup->resizable(editWnd);

  // create the editor toolbar
  Group *toolbar = new Group(2, pageHeight - tbHeight - 2, w - 4, tbHeight);
  toolbar->begin();
  toolbar->box(THIN_UP_BOX);

  // find control
  findText = new Input(38, 2, 120, mnuHeight, "Find:");
  findText->align(ALIGN_LEFT | ALIGN_CLIP);
  Button *prevBn = new Button(160, 4, 18, mnuHeight - 4, "@-98>;");
  Button *nextBn = new Button(180, 4, 18, mnuHeight - 4, "@-92>;");
  prevBn->callback(find_cb, (void *)0);
  nextBn->callback(find_cb, (void *)1);
  findText->callback(find_cb, (void *)2);
  findText->when(WHEN_ENTER_KEY_ALWAYS);
  findText->labelfont(HELVETICA);

  // goto-line control
  gotoLine = new Input(238, 2, 40, mnuHeight, "Goto:");
  gotoLine->align(ALIGN_LEFT | ALIGN_CLIP);
  Button *gotoBn = new Button(280, 4, 18, mnuHeight - 4, "@-92>;");
  gotoBn->callback(MainWindow::goto_line_cb, gotoLine);
  gotoLine->callback(MainWindow::goto_line_cb, gotoLine);
  gotoLine->when(WHEN_ENTER_KEY_ALWAYS);
  gotoLine->labelfont(HELVETICA);

  // sub-func jump droplist
  funcList = new Choice(309, 2, 168, mnuHeight);
  funcList->callback(func_list_cb, 0);
  funcList->labelfont(COURIER);
  funcList->begin();
  new Item();
  new Item(SCAN_LABEL);
  funcList->end();
  toolbar->resizable(funcList);

  // close the tool-bar with a resizeable end-box
  Group *boxEnd = new Group(1000, 4, 0, 0);
  toolbar->resizable(boxEnd);
  toolbar->end();

  editGroup->end();
  tabGroup->resizable(editGroup);

  // create the help tab
  helpGroup = new Group(0, tabBegin, w, pageHeight, "Help");
  helpGroup->box(THIN_DOWN_BOX);
  helpGroup->hide();
  helpGroup->begin();
  helpWnd = new HelpWidget(2, 2, w - 4, pageHeight - 4);
  helpWnd->callback(help_contents_anchor_cb);
  helpWnd->loadBuffer(aboutText);
  helpGroup->resizable(helpWnd);
  helpGroup->end();

  // create the output tab
  outputGroup = new Group(0, tabBegin, w, pageHeight, "Output");
  outputGroup->box(THIN_DOWN_BOX);
  outputGroup->hide();
  outputGroup->begin();
  out = new AnsiWidget(2, 2, w - 4, pageHeight - 4, DEF_FONT_SIZE);
  outputGroup->resizable(out);
  outputGroup->end();

  tabGroup->end();
  resizable(tabGroup);

  Group *statusBar = new Group(0, h - mnuHeight + 1, w, mnuHeight - 2);
  statusBar->begin();
  statusBar->box(NO_BOX);
  fileStatus = new Widget(0, 0, w - 137, mnuHeight - 2);
  modStatus = new Widget(w - 136, 0, 33, mnuHeight - 2);
  runStatus = new Widget(w - 102, 0, 33, mnuHeight - 2);
  rowStatus = new Widget(w - 68, 0, 33, mnuHeight - 2);
  colStatus = new Widget(w - 34, 0, 33, mnuHeight - 2);

  for (int n = 0; n < statusBar->children(); n++) {
    Widget *w = statusBar->child(n);
    w->labelfont(HELVETICA);
    w->box(THIN_DOWN_BOX);
    w->color(color());
  }

  fileStatus->align(ALIGN_INSIDE_LEFT | ALIGN_CLIP);
  statusBar->resizable(fileStatus);
  statusBar->end();
  end();
  editWnd->take_focus();
}

bool MainWindow::isBreakExec(void)
{
  return (runMode == break_state || runMode == quit_state);
}

void MainWindow::setModal(bool modal)
{
  runMode = modal ? modal_state : run_state;
}

void MainWindow::setBreak()
{
  runMode = break_state;
}

bool MainWindow::isModal()
{
  return (runMode == modal_state);
}

bool MainWindow::isEdit()
{
  return (runMode == edit_state);
}

void MainWindow::resetPen()
{
  penDownX = 0;
  penDownY = 0;
  penMode = 0;
  penState = 0;
}

void MainWindow::execLink(const char *file)
{
  if (file == 0 || file[0] == 0) {
    return;
  }

  siteHome.empty();
  bool execFile = false;
  if (file[0] == '!' || file[0] == '|') {
    execFile = true;            // exec flag passed with name
    file++;
  }

  // execute a link from the html window
  if (0 == strncasecmp(file, "http://", 7)) {
    char localFile[PATH_MAX];
    dev_file_t df;

    memset(&df, 0, sizeof(dev_file_t));
    strcpy(df.name, file);
    if (http_open(&df) == 0) {
      sprintf(localFile, "Failed to open URL: %s", file);
      statusMsg(localFile);
      return;
    }

    bool httpOK = cacheLink(&df, localFile);
    char *extn = strrchr(file, '.');

    if (httpOK && extn && 0 == strncasecmp(extn, ".bas", 4)) {
      // run the remote program
      editWnd->loadFile(localFile, -1, false);
      statusMsg(file);
      addHistory(file);
      showOutputTab();
      basicMain(localFile, false);
    }
    else {
      // display as html
      int len = strlen(localFile);
      if (strcasecmp(localFile + len - 4, ".gif") == 0 ||
          strcasecmp(localFile + len - 4, ".jpeg") == 0 ||
          strcasecmp(localFile + len - 4, ".jpg") == 0) {
        sprintf(path, "<img src=%s>", localFile);
      }
      else {
        sprintf(path, "file:%s", localFile);
      }
      siteHome.append(df.name, df.drv_dw[1]);
      statusMsg(siteHome.toString());
      updateForm(path);
      showOutputTab();
    }
    return;
  }

  char *colon = strrchr(file, ':');
  if (colon && colon - 1 != file) {
    file = colon + 1;           // clean 'file:' but not 'c:'
  }

  char *extn = strrchr(file, '.');
  if (extn && (0 == strncasecmp(extn, ".bas ", 5) ||
               0 == strncasecmp(extn, ".sbx ", 5))) {
    strcpy(opt_command, extn + 5);
    extn[4] = 0;                // make args available to bas program
  }

  // if the extension is .sbx and this does not exists or is older
  // than the matching .bas then rename to .bas and set opt_nosave
  // to false - otherwise set execFile flag to true
  if (extn && 0 == strncasecmp(extn, ".sbx", 4)) {
    struct stat st_sbx;
    struct stat st_bas;
    bool sbxExists = (stat(file, &st_sbx) == 0);
    strcpy(extn + 1, "bas");    // remains .bas unless sbx valid
    opt_nosave = 0;             // create/use sbx files
    if (sbxExists) {
      if (stat(file, &st_bas) == 0 && st_sbx.st_mtime > st_bas.st_mtime) {
        strcpy(extn + 1, "sbx");
        // sbx exists and is newer than .bas
        execFile = true;
      }
    }
  }
  if (access(file, 0) == 0) {
    statusMsg(file);
    if (execFile) {
      addHistory(file);
      showOutputTab();
      basicMain(file, false);
      opt_nosave = 1;
    }
    else {
      editWnd->loadFile(file, -1, true);
      showEditTab();
    }
  }
  else {
    pathMessage(file);
  }
}

void MainWindow::focusWidget() {
  switch (event_key()) {
  case 'f':
    findText->take_focus();
    break;
  case 'g':
    gotoLine->take_focus();
    break;
  case 'h':
    funcList->take_focus();
    break;
  }
}

int BaseWindow::handle(int e)
{
  switch (e) {
  case KEY:
    break;                      // process keys below
  case PUSH:
    penState = 1;
    return Window::handle(e);
  case RELEASE:
    penState = -1;
    return Window::handle(e);
  default:
    return Window::handle(e);
  }

  int k;
  switch (runMode) {
  case edit_state:
    if (event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey)) {
      wnd->focusWidget();
    }
    break;
  case run_state:
    k = event_key();
    switch (k) {
    case PageUpKey:
      dev_pushkey(SB_KEY_PGUP);
      break;
    case PageDownKey:
      dev_pushkey(SB_KEY_PGDN);
      break;
    case UpKey:
      dev_pushkey(SB_KEY_UP);
      break;
    case DownKey:
      dev_pushkey(SB_KEY_DN);
      break;
    case LeftKey:
      dev_pushkey(SB_KEY_LEFT);
      break;
    case RightKey:
      dev_pushkey(SB_KEY_RIGHT);
      break;
    case BackSpaceKey:
    case DeleteKey:
      dev_pushkey(SB_KEY_BACKSPACE);
      break;
    case ReturnKey:
      dev_pushkey(13);
      break;
    case 'b':
      if (event_key_state(LeftCtrlKey) ||
          event_key_state(RightCtrlKey)) {
        wnd->run_break();
        break;
      }
      dev_pushkey(k);
      break;
    case 'q':
      if (event_key_state(LeftCtrlKey) ||
          event_key_state(RightCtrlKey)) {
        wnd->quit();
        break;
      }
      dev_pushkey(k);
      break;

    default:
      if (k >= LeftShiftKey && k <= RightAltKey) {
        break;                  // ignore caps+shift+ctrl+alt
      }
      dev_pushkey(k);
      break;
    }
  default:
    break;
  }
  return Window::handle(e);
}

//--Debug support---------------------------------------------------------------

#if defined(WIN32)
#include <windows.h>
#endif
// see http://download.sysinternals.com/Files/DebugView.zip
// for the free DebugView program
// an alternative debug method is to use insight.exe which
// is included with cygwin.

extern "C" void trace(const char *format, ...)
{
  char buf[4096], *p = buf;
  va_list args;

  va_start(args, format);
  p += vsnprintf(p, sizeof buf - 1, format, args);
  va_end(args);

  while (p > buf && isspace(p[-1])) {
    *--p = '\0';
  }

  *p++ = '\r';
  *p++ = '\n';
  *p = '\0';
#if defined(WIN32)
  OutputDebugString(buf);
#else
  fprintf(stderr, buf);
#endif
}

// End of "$Id$".
