// $Id$
// This file is part of SmallBASIC
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "sys.h"

#include <fltk/Item.h>
#include <fltk/MenuBar.h>
#include <fltk/TabGroup.h>
#include <fltk/ask.h>
#include <fltk/error.h>
#include <fltk/events.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "EditorWidget.h"
#include "HelpWidget.h"
#include "FileWidget.h"
#include "sbapp.h"
#include "StringLib.h"

#if defined(WIN32)
#include <fltk/win32.h>
#endif

extern "C" {
#include "fs_socket_client.h"
}
using namespace fltk;

char* packageHome;
char* runfile = 0;
int recentIndex = 0;
int restart = 0;
Widget* recentMenu[NUM_RECENT_ITEMS];
String recentPath[NUM_RECENT_ITEMS];
int recentPosition[NUM_RECENT_ITEMS];
MainWindow* wnd;
ExecState runMode = init_state;

const char* fileTabName = "File";
const char* helpTabName = "Help";
const char* basHome = "BAS_HOME=";
const char* pluginHome = "plugins";
const char historyFile[] = "history.txt";
const char keywordsFile[] = "keywords.txt";
const char aboutText[] =
  "<b>About SmallBASIC...</b><br><br>"
  "Copyright (c) 2000-2006 Nicholas Christopoulos.<br><br>"
  "FLTK Version " SB_STR_VER "<br>"
  "Copyright (c) 2002-2009 Chris Warren-Smith.<br><br>"
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

// scan for fixed pitch fonts in the background
struct ScanFont {
  ScanFont(Menu* argMenu) : menu(argMenu), index(0) {
    numfonts = fltk::list_fonts(fonts);
    fltk::add_idle(ScanFont::scan_font_cb, this);
  }
  static void scan_font_cb(void* eventData) {
    ((ScanFont*) eventData)->scanNext();
  }
  void scanNext() {
    if (index < numfonts) {
      char label[256];
      sprintf(label, "&View/Font/%s", fonts[index]->system_name());
      setfont(font(fonts[index]->name()), 12);
      if (getwidth("QW#@") == getwidth("il:(")) {
        menu->add(label, 0, (Callback *)EditorWidget::font_name_cb);
      }
      index++;
    }
    else {
      fltk::remove_idle(scan_font_cb, this);
      delete this;
    }
  }
  Menu* menu;
  Font** fonts;
  int numfonts;
  int index;
};

//--EditWindow functions--------------------------------------------------------

void MainWindow::statusMsg(RunMessage runMessage, const char *statusMessage) {
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    editWidget->statusMsg(statusMessage ? statusMessage : editWidget->getFilename());
    editWidget->runMsg(runMessage);
  }
}

void MainWindow::busyMessage()
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    editWidget->statusMsg("Selection unavailable while program is running.");
  }
}

void MainWindow::pathMessage(const char *file)
{
  char message[MAX_PATH];
  sprintf(message, "File not found: %s", file);
  statusMsg(msg_err, message);
}

void MainWindow::showEditTab(EditorWidget* editWidget)
{
  if (editWidget) {
    tabGroup->selected_child(editWidget->parent());
    editWidget->take_focus();
  }
}

void MainWindow::showOutputTab()
{
  tabGroup->selected_child(outputGroup);
}

void MainWindow::saveLastEdit(const char *filename)
{
  // remember the last edited file
  FILE *fp = openConfig(LASTEDIT_FILE);
  if (fp) {
    int err;
    err = fwrite(filename, strlen(filename), 1, fp);
    err = fwrite("\n", 1, 1, fp);
    fclose(fp);
  }
}

void MainWindow::setHideIde() {
  opt_ide = IDE_NONE;

  // update the menu
  ((Menu*) ((Menu*) child(0))->find("Program/Toggle/Hide IDE"))->set();
}

void MainWindow::addHistory(const char *filename)
{
  FILE *fp;
  char buffer[MAX_PATH];
  char updatedfile[MAX_PATH];
  char path[MAX_PATH];

  int len = strlen(filename);
  if (strcasecmp(filename + len - 4, ".sbx") == 0) {
    // don't remember bas exe files
    return;
  }

  // save paths with unix path separators
  strcpy(updatedfile, filename);
  FileWidget::forwardSlash(updatedfile);
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
    int err;
    err = fwrite(filename, strlen(filename), 1, fp);
    err = fwrite("\n", 1, 1, fp);
    fclose(fp);
  }
}

/**
 * run the give file. returns whether break was hit
 */
bool MainWindow::basicMain(EditorWidget* editWidget, const char *filename, bool toolExec)
{
  int len = strlen(filename);
  char path[MAX_PATH];
  if (strcasecmp(filename + len - 4, ".htm") == 0 ||
      strcasecmp(filename + len - 5, ".html") == 0) {
    // render html edit buffer
    sprintf(path, "file:%s", filename);
    updateForm(path);
    if (editWidget) {
      editWidget->take_focus();
    }
    return false;
  }
  if (access(filename, 0) != 0) {
    pathMessage(filename);
    runMode = edit_state;
    return false;
  }

  if (editWidget) {
    editWidget->readonly(true);
    editWidget->runMsg(msg_run);
  }

  opt_pref_width = 0;
  opt_pref_height = 0;

  Window* fullScreen = NULL;
  Group* oldOutputGroup = outputGroup;
  int old_w = out->w();
  int old_h = out->h();
  int interactive = opt_interactive;

  if (!toolExec) {
    if (opt_ide == IDE_NONE) {
      // run in a separate window with the ide hidden
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
      copy_label("SmallBASIC +");
    }
  }
  else {
    opt_interactive = false;
  }

  int success;
  do {
    restart = false;
    runMode = run_state;
    success = sbasic_main(filename);
  }
  while (restart);

  opt_interactive = interactive;
  bool was_break = (runMode == break_state);
  if (editWidget) {
    // may have closed during run
    editWidget = getEditor(filename);
  }

  if (fullScreen != NULL) {
    closeForm(); // cleanup any global formView
    fullScreen->remove(out);
    delete fullScreen;

    outputGroup = oldOutputGroup;
    outputGroup->add(out);
    out->resize(old_w, old_h);
    show();
  }
  else {
    copy_label("SmallBASIC");
  }

  if (runMode == quit_state) {
    exit(0);
  }

  if (!success || was_break) {
    if (!toolExec && editWidget && (!was_break || breakToLine)) {
      editWidget->gotoLine(gsb_last_line);
    }
    if (was_break) {
      // override any possible stack error
      sprintf(gsb_last_errmsg, "BREAK AT LINE %d", gsb_last_line);
    }
    else {
      int len = strlen(gsb_last_errmsg);
      if (gsb_last_errmsg[len - 1] == '\n') {
        gsb_last_errmsg[len - 1] = 0;
      }
    }
    closeForm();  // unhide the error
    if (editWidget) {
      showEditTab(editWidget);
      editWidget->runMsg(was_break ? msg_none : msg_err);
      editWidget->statusMsg(gsb_last_errmsg);
    }
  }
  else if (editWidget) {
    editWidget->runMsg(msg_none);
  }

  if (editWidget) {
    editWidget->readonly(false);
  }
  runMode = edit_state;
  return was_break;
}

//--Menu callbacks--------------------------------------------------------------

void MainWindow::close_tab(Widget* w, void* eventData) {
  if (tabGroup->children() > 1) {
    Group* group = getSelectedTab();
    if (group && group != outputGroup) {
      if (gw_editor == ((GroupWidget) (int)group->user_data())) {
        EditorWidget* editWidget = (EditorWidget*)group->child(0);
        if (!editWidget->checkSave(true)) {
          return;
        }
      }
      tabGroup->remove(group);
      delete group;
    }
  }
}

void MainWindow::restart_run(Widget* w, void* eventData) {
  if (runMode == run_state) {
    brun_break();
    restart = true;
    runMode = break_state;
  }
}

void MainWindow::quit(Widget* w, void* eventData)
{
  if (runMode == edit_state || runMode == quit_state) {
    // auto-save scratchpad
    int n = tabGroup->children();
    for (int c = 0; c<n; c++) {
      Group* group = (Group*)tabGroup->child(c);
      char path[MAX_PATH];
      if (gw_editor == ((GroupWidget) (int)group->user_data())) {
        EditorWidget* editWidget = (EditorWidget*)group->child(0);
        const char *filename = editWidget->getFilename();
        int offs = strlen(filename) - strlen(UNTITLED_FILE);
        if (filename[0] == 0 ||
            (offs > 0 && strcasecmp(filename + offs, UNTITLED_FILE) == 0)) {
          getHomeDir(path);
          strcat(path, UNTITLED_FILE);
          editWidget->doSaveFile(path);
        }
        else if (!editWidget->checkSave(true)) {
          return;
        }
      }
    }
    exit(0);
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
  char path[MAX_PATH];
  strcpy(path, "http://smallbasic.sf.net");
  browseFile(path);
}

/**
 * display the results of help.bas
 */
void MainWindow::showHelpPage() {
  HelpWidget* help = getHelp();
  char path[MAX_PATH];
  getHomeDir(path);
  help->setDocHome(path);
  strcat(path, "help.html");
  help->loadFile(path);
}

void MainWindow::execHelp() {
  char path[MAX_PATH];
  if (strncmp(opt_command, "http://", 7) == 0) {
    // launch in real browser
    browseFile(opt_command);
  }
  else {
    sprintf(path, "%s/%s/help.bas", packageHome, pluginHome);
    basicMain(0, path, true);
  }
}

/**
 * handle click from within help window
 */
void do_help_contents_anchor(void *)
{
  fltk::remove_check(do_help_contents_anchor);
  strcpy(opt_command, wnd->getHelp()->getEventName());
  wnd->execHelp();
  wnd->showHelpPage();
}

void MainWindow::help_contents_anchor(Widget* w, void* eventData) {
  if (runMode == edit_state) {
    fltk::add_check(do_help_contents_anchor);
  }
}

/**
 * handle f1 context help
 */
void MainWindow::help_contents(Widget* w, void* eventData)
{
  if (runMode == edit_state) {
    EditorWidget* editWidget = getEditor();
    if (editWidget && event_key() != 0) {
      // scan for help context
      TextEditor *editor = editWidget->editor;
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
    if (access(helpFile, R_OK) == 0) {
      getHelp()->loadFile(helpFile);
    }
    else {
      getHelp()->loadBuffer(helpFile);
    }
  }
  else {
    getHelp()->loadBuffer("APP_HELP env variable not found");
  }
}

void MainWindow::help_about(Widget* w, void* eventData)
{
  getHelp()->loadBuffer(aboutText);
}

void MainWindow::set_flag(Widget* w, void* eventData)
{
  bool* flag = (bool*) eventData;
  *flag = (w->flags() & STATE);
}

void MainWindow::set_options(Widget* w, void* eventData)
{
  const char* args = fltk::input("Enter program command line", opt_command);
  if (args) {
    strcpy(opt_command, args);
  }
}

void MainWindow::hide_ide(Widget* w, void* eventData)
{
  opt_ide = (w->flags() & STATE) ? IDE_NONE : IDE_LINKED;
}

void MainWindow::next_tab(Widget* w, void* eventData)
{
  Group* group = getNextTab(getSelectedTab());
  tabGroup->selected_child(group);
  EditorWidget* editWidget = getEditor(group);
  if (editWidget) {
    editWidget->take_focus();
  }
}

void MainWindow::prev_tab(Widget* w, void* eventData)
{
  Group* group = getPrevTab(getSelectedTab());
  tabGroup->selected_child(group);
  EditorWidget* editWidget = getEditor(group);
  if (editWidget) {
    editWidget->take_focus();
  }
}

void MainWindow::copy_text(Widget* w, void* eventData)
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    EditorWidget::copy_cb(0, editWidget);
  }
  else {
    handle(EVENT_COPY_TEXT);
  }
}

void MainWindow::font_size_incr(Widget* w, void* eventData)
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    int size = editWidget->getFontSize();
    if (size < MAX_FONT_SIZE) {
      editWidget->setFontSize(size + 1);
      updateConfig(editWidget);
      out->setFontSize(size + 1);
    }
  }
  else {
    handle(EVENT_INCREASE_FONT);
  }
}

void MainWindow::font_size_decr(Widget* w, void* eventData)
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    int size = editWidget->getFontSize();
    if (size > MIN_FONT_SIZE) {
      editWidget->setFontSize(size - 1);
      updateConfig(editWidget);
      out->setFontSize(size - 1);
    }
  }
  else {
    handle(EVENT_DECREASE_FONT);
  }
}

void MainWindow::run(Widget* w, void* eventData)
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    const char *filename = editWidget->getFilename();
    if (runMode == edit_state) {
      // inhibit autosave on run function with environment var
      const char *noSave = dev_getenv("NO_RUN_SAVE");
      char path[MAX_PATH];
      if (noSave == 0 || noSave[0] != '1') {
        if (filename == 0 || filename[0] == 0) {
          getHomeDir(path);
          strcat(path, UNTITLED_FILE);
          filename = path;
          editWidget->doSaveFile(filename);
        }
        else if (access(filename, W_OK) == 0) {
          editWidget->doSaveFile(filename);
        }
      }
      showOutputTab();
      basicMain(editWidget, filename, false);
    }
    else {
      busyMessage();
    }
  }
}

void MainWindow::run_break(Widget* w, void* eventData)
{
  if (runMode == run_state || runMode == modal_state) {
    runMode = break_state;
  }
}

/**
 * run the selected text as the main program
 */
void MainWindow::run_selection(Widget* w, void* eventData)
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    char path[MAX_PATH];
    getHomeDir(path);
    strcat(path, "selection.bas");
    editWidget->saveSelection(path);
    basicMain(0, path, false);
  }
}

/**
 * callback for editor-plug-in plug-ins. we assume the target
 * program will be changing the contents of the editor buffer
 */
void MainWindow::editor_plugin(Widget* w, void* eventData)
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    TextEditor *editor = editWidget->editor;
    char filename[MAX_PATH];
    char path[MAX_PATH];
    strcpy(filename, editWidget->getFilename());

    if (runMode == edit_state) {
      if (editWidget->checkSave(false) && filename[0]) {
        int pos = editor->insert_position();
        int row, col, s1r, s1c, s2r, s2c;
        editWidget->getRowCol(&row, &col);
        editWidget->getSelStartRowCol(&s1r, &s1c);
        editWidget->getSelEndRowCol(&s2r, &s2c);
        sprintf(opt_command, "%s|%d|%d|%d|%d|%d|%d",
                filename, row - 1, col, s1r - 1, s1c, s2r - 1, s2c);
        runMode = run_state;
        editWidget->runMsg(msg_run);
        sprintf(path, "%s/%s", packageHome, (const char *)eventData);
        int success = sbasic_main(path);
        editWidget->runMsg(success ? msg_none : msg_err);
        editWidget->loadFile(filename);
        editor->insert_position(pos);
        editor->show_insert_position();
        showEditTab(editWidget);
        runMode = edit_state;
        opt_command[0] = 0;
      }
    }
    else {
      busyMessage();
    }
  }
}

void MainWindow::tool_plugin(Widget* w, void* eventData)
{
  if (runMode == edit_state) {
    char path[MAX_PATH];
    sprintf(opt_command, "%s/%s", packageHome, pluginHome);
    statusMsg(msg_none, (const char *)eventData);
    sprintf(path, "%s/%s", packageHome, (const char *)eventData);
    showOutputTab();
    basicMain(0, path, true);
    statusMsg(msg_none, 0);
    opt_command[0] = 0;
  }
  else {
    busyMessage();
  }
}

void MainWindow::load_file(Widget* w, void* eventData)
{
  int pathIndex = ((int)eventData) - 1;
  const char *path = recentPath[pathIndex].toString();
  EditorWidget* editWidget = getEditor(path);
  if (!editWidget) {
    editWidget = getEditor(createEditor(path));
  }

  if (editWidget->checkSave(true)) {
    TextEditor *editor = editWidget->editor;
    // save current position
    recentPosition[recentIndex] = editor->insert_position();
    recentIndex = pathIndex;
    // load selected file
    if (access(path, 0) == 0) {
      editWidget->loadFile(path);
      // restore previous position
      editor->insert_position(recentPosition[recentIndex]);
      editWidget->statusMsg(path);
      editWidget->fileChanged(true);
      saveLastEdit(path);
      const char* slash = strrchr(path, '/');
      editWidget->parent()->copy_label(slash ? slash + 1 : path);
      showEditTab(editWidget);
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
  char path[MAX_PATH];
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
        if (fileLabel == 0) {
          fileLabel = strrchr(buffer, '\\');
        }
        fileLabel = fileLabel ? fileLabel + 1 : buffer;
        if (fileLabel != 0 && *fileLabel == '_') {
          fileLabel++;
        }
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
    sprintf(label, "&File/Open Recent File/%s", UNTITLED_FILE);
    recentMenu[i] = menu->add(label, CTRL + '1' + i, (Callback *)
                              load_file_cb, (void *)(i + 1));
    recentPath[i].append(UNTITLED_FILE);
    i++;
  }
}

void MainWindow::scanPlugIns(Menu* menu)
{
  FILE *file;
  char buffer[MAX_PATH];
  char path[MAX_PATH];
  char label[1024];
  DIR *dp;
  struct dirent *e;

  snprintf(path, sizeof(path), "%s/%s", packageHome, pluginHome);
  dp = opendir(path);
  while (dp != NULL) {
    e = readdir(dp);
    if (e == NULL) {
      break;
    }
    const char* filename = e->d_name;
    int len = strlen(filename);
    if (strcasecmp(filename + len - 4, ".bas") == 0) {
      sprintf(path, "%s/%s/%s", packageHome, pluginHome, filename);
      file = fopen(path, "r");
      if (!file) {
        continue;
      }
      
      if (!fgets(buffer, MAX_PATH, file)) {
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

      if (fgets(buffer, MAX_PATH, file) && strncmp("'menu", buffer, 5) == 0) {
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

  if (strcmp(argv[i], "-n") == 0) {
    i += 1;
    opt_interactive = 0;
    return 1;
  }

  if (argv[i][0] == '-'  && !argv[i][2] && argv[i + 1]) {
    switch (argv[i][1]) {
    case 'e':
      runfile = strdup(argv[i + 1]);
      runMode = edit_state;
      i += 2;
      return 1;

    case 'v':
      i += 1;
      opt_verbose = 1;
      opt_quiet = 0;
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

bool initialise(int argc, char **argv)
{
  opt_graphics = 1;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_nosave = 1;
  opt_ide = IDE_LINKED;
  opt_pref_bpp = 0;
  os_graphics = 1;
  opt_interactive = 1;

  int i = 0;
  if (args(argc, argv, i, arg_cb) < argc) {
    fatal("Options are:\n"
          " -e[dit] file.bas\n"
          " -r[un] file.bas\n"
          " -v[erbose]\n"
          " -n[on]-interactive\n"
          " -m[odule]-home\n\n%s", help);
  }

  // package home contains installed components
#if defined(WIN32)
  packageHome = strdup(argv[0]);
  char* slash = FileWidget::forwardSlash(packageHome);
  if (slash) {
    *slash = 0;
  }
#else
  packageHome = (char*)PACKAGE_DATA_DIR;
#endif

  char path[MAX_PATH];
  sprintf(path, "PKG_HOME=%s", packageHome);
  dev_putenv(path);

  // bas_home contains user editable files along with generated help
  strcpy(path, basHome);
  getHomeDir(path + strlen(basHome));
  dev_putenv(path);

  wnd = new MainWindow(800, 650);

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

  switch (runMode) {
  case run_state:
    wnd->setHideIde();
    wnd->getEditor(true)->loadFile(runfile);
    wnd->addHistory(runfile);
    wnd->showOutputTab();
    if (!wnd->basicMain(0, runfile, false)) {
      return false; // continue if break hit
    }
  case edit_state:
    wnd->getEditor(true)->loadFile(runfile);
    break;
  default:
    wnd->getEditor(true)->restoreEdit();
    runMode = edit_state;
  }

  wnd->updateEditTabName(wnd->getEditor());
  wnd->show(argc, argv);
  return true;
}

int main(int argc, char **argv)
{
  if (initialise(argc, argv)) {
    run();
  }
  return 0;
}

//--MainWindow methods----------------------------------------------------------

MainWindow::MainWindow(int w, int h) : BaseWindow(w, h)
{
  isTurbo = false;
  breakToLine = false;

  FileWidget::forwardSlash(runfile);
  begin();
  MenuBar *m = new MenuBar(0, 0, w, MNU_HEIGHT);
  m->add("&File/&New File", CTRL + 'n', (Callback *) MainWindow::new_file_cb);
  m->add("&File/&Open File", CTRL + 'o', (Callback *) MainWindow::open_file_cb);
  scanRecentFiles(m);
  m->add("&File/_&Close", CTRL + F4Key, (Callback *) MainWindow::close_tab_cb);
  m->add("&File/&Save File", CTRL + 's', (Callback *) EditorWidget::save_file_cb);
  m->add("&File/_Save File &As", CTRL + SHIFT + 'S', (Callback *) MainWindow::save_file_as_cb);
  m->add("&File/E&xit", CTRL + 'q', (Callback *) MainWindow::quit_cb);
  m->add("&Edit/_&Undo", CTRL + 'z', (Callback *) EditorWidget::undo_cb);
  m->add("&Edit/Cu&t", CTRL + 'x', (Callback *) EditorWidget::cut_text_cb);
  m->add("&Edit/&Copy", CTRL + 'c', (Callback *) MainWindow::copy_text_cb);
  m->add("&Edit/_&Paste", CTRL + 'v', (Callback *) EditorWidget::paste_text_cb);
  m->add("&Edit/&Change Case", ALT + 'c', (Callback *) EditorWidget::change_case_cb);
  m->add("&Edit/&Expand Word", ALT + '/', (Callback *) EditorWidget::expand_word_cb);
  m->add("&Edit/_&Rename Word", CTRL + SHIFT + 'r', (Callback *) EditorWidget::rename_word_cb);
  m->add("&Edit/&Find", CTRL + 'f', (Callback *) EditorWidget::find_cb);
  m->add("&Edit/&Replace", CTRL + 'r', (Callback *) EditorWidget::show_replace_cb);
  m->add("&Edit/_&Goto Line", CTRL + 'g', (Callback *) EditorWidget::goto_line_cb);
  m->add("&View/&Next Tab", F6Key, (Callback *) MainWindow::next_tab_cb);
  m->add("&View/_&Prev Tab", CTRL + F6Key, (Callback *) MainWindow::prev_tab_cb);
  m->add("&View/Text Size/&Increase", CTRL + ']', (Callback *) MainWindow::font_size_incr_cb);
  m->add("&View/Text Size/&Decrease", CTRL + '[', (Callback *) MainWindow::font_size_decr_cb);
  m->add("&View/Text Color/Background", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_background);
  m->add("&View/Text Color/_Text", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_text);
  m->add("&View/Text Color/Comments", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_comments);
  m->add("&View/Text Color/_Strings", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_strings);
  m->add("&View/Text Color/Keywords", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_keywords);
  m->add("&View/Text Color/Funcs", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_funcs);
  m->add("&View/Text Color/_Subs", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_subs);
  m->add("&View/Text Color/_Find Matches", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_findMatches);
  m->add("&View/Text Color/Numbers", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_numbers);
  m->add("&View/Text Color/Operators", 0, (Callback *) EditorWidget::set_color_cb, (void*) st_operators);

  new ScanFont(m);
  scanPlugIns(m);

  m->add("&Program/&Run", F9Key, (Callback *) run_cb);
  m->add("&Program/_&Run Selection", F8Key, (Callback *) run_selection_cb);
  m->add("&Program/&Break", CTRL + 'b', (Callback *) MainWindow::run_break_cb);
  m->add("&Program/_&Restart", CTRL + 'r', (Callback *) MainWindow::restart_run_cb);
  m->add("&Program/&Command", F10Key, (Callback *) MainWindow::set_options_cb);
  m->add("&Program/Toggle/&Hide IDE", 0, (Callback *) hide_ide_cb)->type(Item::TOGGLE);
  m->add("&Program/Toggle/&Break To Line", 0, (Callback *) set_flag_cb, &breakToLine)->type(Item::TOGGLE);
  m->add("&Program/Toggle/&Turbo", 0, (Callback *) set_flag_cb, &isTurbo)->type(Item::TOGGLE);
  m->add("&Help/&Help Contents", F1Key, (Callback *) MainWindow::help_contents_cb);
  m->add("&Help/_&Program Help", F11Key, (Callback *) MainWindow::help_app_cb);
  m->add("&Help/_&Home Page", 0, (Callback *) MainWindow::help_home_cb);
  m->add("&Help/&About SmallBASIC", F12Key, (Callback *) MainWindow::help_about_cb);

  callback(quit_cb);
  shortcut(0);                  // remove default EscapeKey shortcut

  // outer decoration group
  h -= MNU_HEIGHT;
  Group* outer = new Group(0, MNU_HEIGHT, w, h);
  outer->begin();
  outer->box(ENGRAVED_BOX);

  // group for all tabs
  w -= 8;
  h -= MNU_HEIGHT;
  tabGroup = new TabGroup(4, 4, w, h);

  h -= 8; // TabGroup border
  tabGroup->begin();

  // create the output tab
  outputGroup = new Group(0, MNU_HEIGHT, w, h, "Output");
  outputGroup->box(THIN_DOWN_BOX);
  outputGroup->hide();
  outputGroup->user_data((void*) gw_output);
  outputGroup->begin();
  out = new AnsiWidget(2, 2, w - 4, h - 4, DEF_FONT_SIZE);
  outputGroup->resizable(out);
  outputGroup->end();

  createEditor(UNTITLED_FILE);
  tabGroup->resizable(outputGroup);
  tabGroup->end();
  outer->end();
  end();

  outer->resizable(tabGroup);
  resizable(outer);
}

/**
 * create a new help widget and add it to the tab group
 */
Group* MainWindow::createEditor(const char* title) {
  int w = tabGroup->w();
  int h = tabGroup->h() - 8;

  Group* editGroup = new Group(0, MNU_HEIGHT, w, h);
  const char* slash = strrchr(title, '/');
  editGroup->copy_label(slash ? slash + 1 : title);
  editGroup->begin();
  editGroup->box(THIN_DOWN_BOX);
  editGroup->resizable(new EditorWidget(2, 2, w - 4, h));
  editGroup->user_data((void*) gw_editor);
  editGroup->end();

  tabGroup->add(editGroup);
  tabGroup->selected_child(editGroup);
  return editGroup;
}

void MainWindow::new_file(Widget* w, void* eventData) 
{
  EditorWidget* editWidget = 0;
  Group* untitledEditor = findTab(UNTITLED_FILE);
  char path[MAX_PATH];

  if (untitledEditor) {
    tabGroup->selected_child(untitledEditor);
    editWidget = getEditor(untitledEditor);
  }
  if (!editWidget) {
    editWidget = getEditor(createEditor(UNTITLED_FILE));

    // preserve the contents of any existing untitled.bas
    getHomeDir(path);
    strcat(path, UNTITLED_FILE);
    if (access(path, 0) == 0) {
      editWidget->loadFile(path);
    }
  }

  TextBuffer *textbuf = editWidget->editor->buffer();
  textbuf->select(0, textbuf->length());
}

void MainWindow::open_file(Widget* w, void* eventData) 
{
  Group* openFileGroup = findTab(gw_file);
  if (!openFileGroup ) {
    int w = tabGroup->w();
    int h = tabGroup->h() - 8;
    tabGroup->begin();
    openFileGroup = new Group(0, MNU_HEIGHT, w, h, fileTabName);
    openFileGroup->begin();
    openFileGroup->box(THIN_DOWN_BOX);
    openFileGroup->user_data((void*) gw_file);
    openFileGroup->resizable(new FileWidget(2, 2, w - 4, h - 4));
    openFileGroup->end();
    tabGroup->end();
  }

  tabGroup->selected_child(openFileGroup);
}

void MainWindow::save_file_as(Widget* w, void* eventData) 
{
  EditorWidget* editWidget = getEditor();
  if (editWidget) {
    open_file(w, eventData);
    FileWidget* fileWidget = (FileWidget*) getSelectedTab()->resizable();
    fileWidget->fileOpen(editWidget);
  }
}

HelpWidget* MainWindow::getHelp()
{
  HelpWidget* help = 0;
  Group* helpGroup = findTab(gw_help);
  if (!helpGroup) {
    int w = tabGroup->w();
    int h = tabGroup->h() - 8;
    tabGroup->begin();
    helpGroup = new Group(0, MNU_HEIGHT, w, h, helpTabName);
    helpGroup->box(THIN_DOWN_BOX);
    helpGroup->hide();
    helpGroup->user_data((void*) gw_help);
    helpGroup->begin();
    help = new HelpWidget(2, 2, w - 4, h - 4);
    help->callback(help_contents_anchor_cb);
    helpGroup->resizable(help);
    tabGroup->end();
  }
  else {
    help = (HelpWidget*) helpGroup->resizable();
  }
  tabGroup->selected_child(helpGroup);
  return help;
}

EditorWidget* MainWindow::getEditor(bool select) 
{
  EditorWidget* result=0;
  if (select) {
    int n = tabGroup->children();
    for (int c = 0; c<n; c++) {
      Group* group = (Group*)tabGroup->child(c);
      if (gw_editor == ((GroupWidget) (int)group->user_data())) {
        result = (EditorWidget*)group->child(0);
        tabGroup->selected_child(group);
        break;
      }
    }
  }
  else {
    result = getEditor(getSelectedTab());
  }
  return result;
}

EditorWidget* MainWindow::getEditor(Group* group) 
{
  EditorWidget* editWidget = 0;
  if (group != 0 && gw_editor == ((GroupWidget) (int)group->user_data())) {
    editWidget = (EditorWidget*)group->resizable();
  }
  return editWidget;
}

EditorWidget* MainWindow::getEditor(const char* fullPath) 
{
  if (fullPath != 0 && fullPath[0] != 0) {
    int n = tabGroup->children();
    for (int c = 0; c < n; c++) {
      Group* group = (Group*)tabGroup->child(c);
      if (gw_editor == ((GroupWidget) (int)group->user_data())) {
        EditorWidget* editWidget = (EditorWidget*)group->child(0);
        const char* fileName = editWidget->getFilename();
        if (fileName && strcmp(fullPath, fileName) == 0) {
          return editWidget;
        }
      }
    }
  }
  return NULL;
}

/**
 * called by FileWidget to open the selected file
 */
void MainWindow::editFile(const char* filePath) 
{
  EditorWidget* editWidget = getEditor(filePath);
  if (!editWidget) {
    editWidget = getEditor(createEditor(filePath));
    editWidget->loadFile(filePath);
    addHistory(filePath);
  }
  showEditTab(editWidget);
}

Group* MainWindow::getSelectedTab()
{
  return (Group*)tabGroup->selected_child();
}

/**
 * returns the tab with the given name
 */
Group* MainWindow::findTab(const char* label) 
{
  int n = tabGroup->children();
  for (int c = 0; c<n; c++) {
    Group* child = (Group*)tabGroup->child(c);
    if (strcmp(child->label(), label) == 0) {
      return child;
    }
  }
  return 0;
}

Group* MainWindow::findTab(GroupWidget groupWidget) 
{
  int n = tabGroup->children();
  for (int c = 0; c<n; c++) {
    Group* child = (Group*)tabGroup->child(c);
    if (groupWidget == ((GroupWidget) (int) child->user_data())) {
      return child;
    }
  }
  return 0;
}

/**
 * find and select the tab with the given tab label
 */
Group* MainWindow::selectTab(const char* label) 
{
  Group* tab = findTab(label);
  if (tab) {
    tabGroup->selected_child(tab);
  }
  return tab;
}

/**
 * copies the configuration from the current editor to any remaining editors
 */
void MainWindow::updateConfig(EditorWidget* current) 
{
  current->saveConfig();
  int n = tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group* group = (Group*)tabGroup->child(c);
    if (gw_editor == ((GroupWidget) (int)group->user_data())) {
      EditorWidget* editWidget = (EditorWidget*)group->child(0);
      if (editWidget != current) {
        editWidget->updateConfig(current);
      }
    }
  }
}

/**
 * updates the names of the editor tabs based on the enclosed editing file
 */
void MainWindow::updateEditTabName(EditorWidget* editWidget) 
{
  int n = tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group* group = (Group*)tabGroup->child(c);
    if (gw_editor == ((GroupWidget) (int)group->user_data()) &&
        editWidget == (EditorWidget*)group->child(0)) {
      const char* editFileName = editWidget->getFilename();
      if (editFileName && editFileName[0]) {
        const char* slash = strrchr(editFileName, '/');
        group->copy_label(slash ? slash + 1 : editFileName);
      }
    }      
  }
}

/**
 * returns the tab following the given tab
 */
Group* MainWindow::getNextTab(Group* current) 
{
  int n = tabGroup->children();
  for (int c = 0; c < n - 1; c++) {
    Group* child = (Group*) tabGroup->child(c);
    if (child == current) {
      return (Group*) tabGroup->child(c+1);
    }
  }
  return (Group*) tabGroup->child(0);
}

/**
 * returns the tab prior the given tab or null if not found
 */
Group* MainWindow::getPrevTab(Group* current) 
{
  int n = tabGroup->children();
  for (int c = n - 1; c > 0; c--) {
    Group* child = (Group*)tabGroup->child(c);
    if (child == current) {
      return (Group*) tabGroup->child(c - 1);
    }
  }
  return (Group*) tabGroup->child(n - 1);
}

/**
 * Opens the config file ready for writing
 */
FILE* MainWindow::openConfig(const char* fileName, const char* flags) {
  char path[MAX_PATH];
  getHomeDir(path);
  strcat(path, fileName);
  return fopen(path, flags);
}

bool MainWindow::isBreakExec(void)
{
  return (runMode == break_state || runMode == quit_state);
}

bool MainWindow::isRunning(void)
{
  return (runMode == run_state || runMode == modal_state);
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

bool MainWindow::isInteractive() {
  return opt_interactive;
}

bool MainWindow::isIdeHidden() {
  return (opt_ide == IDE_NONE);
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
  if (!file || !file[0]) {
    return;
  }

  EditorWidget* editWidget = getEditor(true);
  siteHome.empty();
  bool execFile = false;
  if (file[0] == '!' || file[0] == '|') {
    execFile = true;            // exec flag passed with name
    file++;
  }

  // execute a link from the html window
  if (0 == strncasecmp(file, "http://", 7)) {
    char localFile[PATH_MAX];
    char path[MAX_PATH];
    dev_file_t df;

    memset(&df, 0, sizeof(dev_file_t));
    strcpy(df.name, file);
    if (http_open(&df) == 0) {
      sprintf(localFile, "Failed to open URL: %s", file);
      statusMsg(msg_none, localFile);
      return;
    }

    bool httpOK = cacheLink(&df, localFile);
    char *extn = strrchr(file, '.');
    if (!editWidget) {
      editWidget = getEditor(createEditor(file));
    }

    if (httpOK && extn && 0 == strncasecmp(extn, ".bas", 4)) {
      // run the remote program
      editWidget->loadFile(localFile);
      addHistory(file);
      showOutputTab();
      basicMain(editWidget, localFile, false);
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
      statusMsg(msg_none, siteHome.toString());
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
    statusMsg(msg_none, file);
    if (execFile) {
      addHistory(file);
      showOutputTab();
      basicMain(0, file, false);
      opt_nosave = 1;
    }
    else {
      if (!editWidget) {
        editWidget = getEditor(createEditor(file));
      }
      editWidget->loadFile(file);
      showEditTab(editWidget);
    }
  }
  else {
    pathMessage(file);
  }
}

int BaseWindow::handle(int e)
{
  switch (e) {
  case SHORTCUT:
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
      EditorWidget* editWidget = wnd->getEditor();
      if (editWidget) {
        editWidget->focusWidget();
      }
    }
    break;
  case run_state:
  case modal_state:
    k = event_key();
    switch (k) {
    case TabKey:
      dev_pushkey(SB_KEY_TAB);
      break;
    case HomeKey:
      dev_pushkey(SB_KEY_KP_HOME);
      break;
    case EndKey:
      dev_pushkey(SB_KEY_END);
      break;
    case InsertKey:
      dev_pushkey(SB_KEY_INSERT);
      break;
    case MenuKey:
      dev_pushkey(SB_KEY_MENU);
      break;
    case MultiplyKey:
      dev_pushkey(SB_KEY_KP_MUL);
      break;
    case AddKey:
      dev_pushkey(SB_KEY_KP_PLUS);
      break;
    case SubtractKey:
      dev_pushkey(SB_KEY_KP_MINUS);
      break;
    case DivideKey:
      dev_pushkey(SB_KEY_KP_DIV);
      break;
    case F0Key:
      dev_pushkey(SB_KEY_F(0));
      break;
    case F1Key:       
      dev_pushkey(SB_KEY_F(1));
      break;
    case F2Key:
      dev_pushkey(SB_KEY_F(2));
      break;
    case F3Key:
      dev_pushkey(SB_KEY_F(3));
      break;
    case F4Key:
      dev_pushkey(SB_KEY_F(4));
      break;
    case F5Key:
      dev_pushkey(SB_KEY_F(5));
      break;
    case F6Key:
      dev_pushkey(SB_KEY_F(6));
      break;
    case F7Key:
      dev_pushkey(SB_KEY_F(7));
      break;
    case F8Key:
      dev_pushkey(SB_KEY_F(8));
      break;
    case F9Key:
      dev_pushkey(SB_KEY_F(9));
      break;
    case F10Key:
      dev_pushkey(SB_KEY_F(10));
      break;
    case F11Key:
      dev_pushkey(SB_KEY_F(11));
      break;
    case F12Key:
      dev_pushkey(SB_KEY_F(12));
      break;
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

LineInput::LineInput(int x, int y, int w, int h) : fltk::Input(x, y, w, h) {
  this->orig_x = x;
  this->orig_y = y;
  this->orig_w = w;
  this->orig_h = h;
  when(WHEN_ENTER_KEY_ALWAYS);
  box(BORDER_BOX);
  color(fltk::color(220, 220, 220));
  take_focus();
} 

/**
 * grow the input box width as text is entered
 */
bool LineInput::replace(int b, int e, const char *text, int ilen) {
  bool result = Input::replace(b, e, text, ilen);
  if (ilen) {
    int strw = (int) (getwidth(text) + getwidth(value())) + 4;
    if (strw > w()) {
      w(strw);
      orig_w = strw;
      redraw();
    }
  }
  return result;
}

/**
 * veto the layout changes
 */ 
void LineInput::layout() {
  fltk::Input::layout();
  x(orig_x);
  y(orig_y);
  w(orig_w);
  h(orig_h);
}

int LineInput::handle(int event) {
  if (event == fltk::KEY) {
    if ((event_key_state(LeftCtrlKey) ||
         event_key_state(RightCtrlKey)) && event_key() == 'b') {
      if (!wnd->isEdit()) {
        wnd->setBreak();
      }
    }
    if (event_key_state(EscapeKey)) {
      do_callback();
    }
  }
  return fltk::Input::handle(event);
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
  fprintf(stderr, buf, 0);
#endif
}

// End of "$Id$".
