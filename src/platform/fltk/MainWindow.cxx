// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dirent.h>

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
#include "common/sbapp.h"
#include "platform/common/StringLib.h"

#include "common/sys.h"
#include "common/fs_socket_client.h"

using namespace fltk;

char *packageHome;
char *runfile = 0;
int recentIndex = 0;
int restart = 0;
fltk::Widget *recentMenu[NUM_RECENT_ITEMS];
strlib::String recentPath[NUM_RECENT_ITEMS];
int recentPosition[NUM_RECENT_ITEMS];
MainWindow *wnd;
ExecState runMode = init_state;

const char *untitledFile = "untitled.bas";
const char *fileTabName = "File";
const char *helpTabName = "Help";
const char *basHome = "BAS_HOME=";
const char *pluginHome = "plugins";
const char *historyFile = "history.txt";
const char *keywordsFile = "keywords.txt";
const char *aboutText =
  "<b>About SmallBASIC...</b><br><br>"
  "Version " SB_STR_VER "<br>"
  "Copyright (c) 2002-2013 Chris Warren-Smith.<br><br>"
  "Copyright (c) 2000-2006 Nicholas Christopoulos.<br><br>"
  "<a href=http://smallbasic.sourceforge.net>"
  "http://smallbasic.sourceforge.net</a><br><br>"
  "SmallBASIC comes with ABSOLUTELY NO WARRANTY. "
  "This program is free software; you can use it "
  "redistribute it and/or modify it under the terms of the "
  "GNU General Public License version 2 as published by "
  "the Free Software Foundation.<br><br>" "<i>Press F1 for help";

// in dev_fltk.cpp
void getHomeDir(char *filename, bool appendSlash = true);
bool cacheLink(dev_file_t *df, char *localFile);
void updateForm(const char *s);
void closeForm();
bool isFormActive();

// scan for fixed pitch fonts in the background
struct ScanFont {
  ScanFont(Menu *argMenu) : menu(argMenu), index(0) {
    numfonts = fltk::list_fonts(fonts);
    fltk::add_idle(ScanFont::scan_font_cb, this);
  } 
  static void scan_font_cb(void *eventData) {
    ((ScanFont *)eventData)->scanNext();
  }
  void scanNext() {
    if (index < numfonts) {
      char label[256];
      sprintf(label, "&View/Font/%s", fonts[index]->system_name());
      setfont(font(fonts[index]->name()), 12);
      if (getdescent() < MAX_DESCENT && (getwidth("QW#@") == getwidth("il:("))) {
        fltk::Widget *w = menu->add(label, 0, (Callback *)EditorWidget::font_name_cb);
        w->textfont(getfont());
      }
      index++;
    } else {
      fltk::remove_idle(scan_font_cb, this);
      delete this;
    }
  }
  Menu *menu;
  Font **fonts;
  int numfonts;
  int index;
};

//--EditWindow functions--------------------------------------------------------

void MainWindow::statusMsg(RunMessage runMessage, const char *statusMessage) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    editWidget->statusMsg(statusMessage);
    editWidget->runState(runMessage);
  }
}

void MainWindow::busyMessage() {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    editWidget->statusMsg("Selection unavailable while program is running.");
  }
}

void MainWindow::pathMessage(const char *file) {
  char message[MAX_PATH];
  sprintf(message, "File not found: %s", file);
  statusMsg(rs_err, message);
}

void MainWindow::showEditTab(EditorWidget *editWidget) {
  if (editWidget) {
    _tabGroup->selected_child(editWidget->parent());
    editWidget->take_focus();
  }
}

/**
 * run the give file. returns whether break was hit
 */
bool MainWindow::basicMain(EditorWidget *editWidget, 
                           const char *fullpath, bool toolExec) {
  int len = strlen(fullpath);
  char path[MAX_PATH];
  bool breakToLine = false;     // whether to restore the editor cursor 

  if (strcasecmp(fullpath + len - 4, ".htm") == 0 || 
      strcasecmp(fullpath + len - 5, ".html") == 0) {
    // render html edit buffer
    sprintf(path, "file:%s", fullpath);
    updateForm(path);
    if (editWidget) {
      editWidget->take_focus();
    }
    return false;
  }
  if (access(fullpath, 0) != 0) {
    pathMessage(fullpath);
    runMode = edit_state;
    return false;
  }
  // start in the directory of the bas program
  const char *filename = FileWidget::splitPath(fullpath, path);

  if (editWidget) {
    _runEditWidget = editWidget;
    if (!toolExec) {
      editWidget->readonly(true);
      editWidget->runState(rs_run);
      breakToLine = editWidget->isBreakToLine();
      opt_ide = editWidget->isHideIDE()? IDE_NONE : IDE_LINKED;
    }
  }

  opt_pref_width = 0;
  opt_pref_height = 0;

  Window *fullScreen = NULL;
  Group *oldOutputGroup = _outputGroup;
  int old_w = _out->w();
  int old_h = _out->h();
  int interactive = opt_interactive;

  if (!toolExec) {
    if (opt_ide == IDE_NONE) {
      // run in a separate window with the ide hidden
      fullScreen = new BaseWindow(w(), h());
      _profile->restoreAppPosition(fullScreen);

      fullScreen->callback(quit_cb);
      fullScreen->shortcut(0);
      fullScreen->add(_out);
      fullScreen->resizable(fullScreen);
      setTitle(fullScreen, filename);
      _outputGroup = fullScreen;
      _out->resize(0, 0, w(), h());
      hide();
    } else {
      setTitle(this, filename);
    }
  } else {
    opt_interactive = false;
  }

  int success;
  do {
    restart = false;
    runMode = run_state;
    chdir(path);
    success = sbasic_main(filename);
  }
  while (restart);

  opt_interactive = interactive;
  bool was_break = (runMode == break_state);

  if (fullScreen != NULL) {
    _profile->_appPosition = *fullScreen;

    closeForm();                // cleanup any global formView
    fullScreen->remove(_out);
    delete fullScreen;

    _outputGroup = oldOutputGroup;
    _outputGroup->add(_out);
    _out->resize(2, 2, old_w, old_h);
    show();
  } else {
    copy_label("SmallBASIC");
  }

  if (runMode == quit_state) {
    exit(0);
  }

  if (!success || was_break) {
    if (!toolExec && editWidget && (!was_break || breakToLine)) {
      editWidget->gotoLine(gsb_last_line);
    }
    closeForm();                // unhide the error
    if (editWidget) {
      showEditTab(editWidget);
      editWidget->runState(was_break ? rs_ready : rs_err);
    }
  } else if (!toolExec && editWidget) {
    // normal termination
    editWidget->runState(rs_ready);
  }

  if (!toolExec && editWidget) {
    editWidget->readonly(false);
  }

  runMode = edit_state;
  _runEditWidget = 0;
  return was_break;
}

//--Menu callbacks--------------------------------------------------------------

void MainWindow::close_tab(fltk::Widget *w, void *eventData) {
  if (_tabGroup->children() > 1) {
    Group *group = getSelectedTab();
    if (group && group != _outputGroup) {
      if (gw_editor == getGroupWidget(group)) {
        EditorWidget *editWidget = (EditorWidget *)group->child(0);
        if (!editWidget->checkSave(true)) {
          return;
        }
        // check whether the editor is a running program
        if (editWidget == _runEditWidget) {
          setBreak();
          return;
        }
      }
      _tabGroup->remove(group);
      delete group;
    }
  }
}

void MainWindow::restart_run(fltk::Widget *w, void *eventData) {
  if (runMode == run_state) {
    setBreak();
    restart = true;
  }
}

void MainWindow::quit(fltk::Widget *w, void *eventData) {
  if (runMode == edit_state || runMode == quit_state) {
    // auto-save scratchpad
    int n = _tabGroup->children();
    for (int c = 0; c < n; c++) {
      Group *group = (Group *)_tabGroup->child(c);
      char path[MAX_PATH];
      if (gw_editor == getGroupWidget(group)) {
        EditorWidget *editWidget = (EditorWidget *)group->child(0);
        const char *filename = editWidget->getFilename();
        int offs = strlen(filename) - strlen(untitledFile);
        if (filename[0] == 0 || (offs > 0 && strcasecmp(filename + offs, untitledFile) == 0)) {
          getHomeDir(path);
          strcat(path, untitledFile);
          editWidget->doSaveFile(path);
        } else if (!editWidget->checkSave(true)) {
          return;
        }
      }
    }
    exit(0);
  } else {
    switch (choice("Terminate running program?", "*Exit", "Break", "Cancel")) {
    case 2:
      exit(0);
    case 1:
      setBreak();
    }
  }
}

/**
 * opens the smallbasic home page in a browser window
 */
void MainWindow::help_home(fltk::Widget *w, void *eventData) {
  browseFile("http://smallbasic.sf.net");
}

/**
 * display the results of help.bas
 */
void MainWindow::showHelpPage() {
  HelpWidget *help = getHelp();
  char path[MAX_PATH];
  getHomeDir(path);
  help->setDocHome(path);
  strcat(path, "help.html");
  help->loadFile(path);
}

/**
 * when opt_command is a URL, opens the URL in a browser window
 * otherwise runs help.bas with opt_command as the program argument
 */
bool MainWindow::execHelp() {
  bool result = true;
  char path[MAX_PATH];
  if (strncmp(opt_command, "http://", 7) == 0) {
    // launch in real browser
    browseFile(opt_command);
    result = false;
  } else {
    sprintf(path, "%s/%s/help.bas", packageHome, pluginHome);
    basicMain(getEditor(), path, true);
  }
  return result;
}

/**
 * handle click from within help window
 */
void do_help_contents_anchor(void *){
  fltk::remove_check(do_help_contents_anchor);
  strlib::String eventName = wnd->getHelp()->getEventName();
  if (access(eventName, R_OK) == 0) {
    wnd->editFile(eventName);
  } else {
    strcpy(opt_command, eventName);
    if (wnd->execHelp()) {
      wnd->showHelpPage();
    }
  }
}

void MainWindow::help_contents_anchor(fltk::Widget *w, void *eventData) {
  if (runMode == edit_state) {
    fltk::add_check(do_help_contents_anchor);
  }
}

/**
 * handle f1 context help
 */
void MainWindow::help_contents(fltk::Widget *w, void *eventData) {
  bool showHelp = true;
  if (runMode == edit_state) {
    EditorWidget *editWidget = getEditor();
    if (editWidget && event_key() != 0) {
      // scan for help context
      int start, end;
      char *selection = editWidget->getSelection(&start, &end);
      strcpy(opt_command, selection);
      free((void *)selection);
    } else {
      opt_command[0] = 0;
    }

    showHelp = execHelp();
  }

  if (!eventData && showHelp) {
    showHelpPage();
  }
}

/**
 * displays the program help page in a browser window
 */
void MainWindow::help_app(fltk::Widget *w, void *eventData) {
  browseFile("http://smallbasic.sourceforge.net/?q=node/955");
}

void MainWindow::help_about(fltk::Widget *w, void *eventData) {
  getHelp()->loadBuffer(aboutText);
}

void MainWindow::set_flag(fltk::Widget *w, void *eventData) {
  bool *flag = (bool *)eventData;
  *flag = (w->flags() & STATE);
}

void MainWindow::export_file(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    if (runMode == edit_state) {
      int handle = 1;
      char buffer[PATH_MAX];
      if (_exportFile.length()) {
        strcpy(buffer, _exportFile.c_str());
      } else {
        buffer[0] = 0;
      }
      editWidget->getInput(buffer, PATH_MAX);
      if (buffer[0]) {
        _exportFile = buffer;
        if (dev_fopen(handle, _exportFile, DEV_FILE_OUTPUT)) {
          TextBuffer *textbuf = editWidget->editor->buffer();
          const char *data = textbuf->text();
          if (!dev_fwrite(handle, (byte *)data, textbuf->length())) {
            sprintf(buffer, "Failed to write: %s", _exportFile.c_str());
            statusMsg(rs_err, buffer);
          } else {
            sprintf(buffer, "Exported %s to %s", editWidget->getFilename(), 
                    _exportFile.c_str());
            statusMsg(rs_ready, buffer);
          }
        } else {
          sprintf(buffer, "Failed to open: %s", _exportFile.c_str());
          statusMsg(rs_err, buffer);
        }
        dev_fclose(handle);
      }
      // cancel setModal() from editWidget->getInput()
      runMode = edit_state; 
    } else {
      busyMessage();
    }
  }
}

void MainWindow::set_options(fltk::Widget *w, void *eventData) {
  const char *args = fltk::input("Enter program command line", opt_command);
  if (args) {
    strcpy(opt_command, args);
  }
}

void MainWindow::next_tab(fltk::Widget *w, void *eventData) {
  Group *group = getNextTab(getSelectedTab());
  _tabGroup->selected_child(group);
  EditorWidget *editWidget = getEditor(group);
  if (editWidget) {
    editWidget->take_focus();
  }
}

void MainWindow::prev_tab(fltk::Widget *w, void *eventData) {
  Group *group = getPrevTab(getSelectedTab());
  _tabGroup->selected_child(group);
  EditorWidget *editWidget = getEditor(group);
  if (editWidget) {
    editWidget->take_focus();
  }
}

void MainWindow::copy_text(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    editWidget->copyText();
  } else {
    handle(EVENT_COPY_TEXT);
  }
}

void MainWindow::font_size_incr(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    int size = editWidget->getFontSize();
    if (size < MAX_FONT_SIZE) {
      editWidget->setFontSize(size + 1);
      updateConfig(editWidget);
      _out->setFontSize(size + 1);
    }
  } else {
    handle(EVENT_INCREASE_FONT);
  }
}

void MainWindow::font_size_decr(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    int size = editWidget->getFontSize();
    if (size > MIN_FONT_SIZE) {
      editWidget->setFontSize(size - 1);
      updateConfig(editWidget);
      _out->setFontSize(size - 1);
    }
  } else {
    handle(EVENT_DECREASE_FONT);
  }
}

void MainWindow::run(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    const char *filename = editWidget->getFilename();
    if (runMode == edit_state) {
      // inhibit autosave on run function with environment var
      const char *noSave = dev_getenv("NO_RUN_SAVE");
      char path[MAX_PATH];
      if (noSave == 0 || noSave[0] != '1') {
        if (filename == 0 || filename[0] == 0) {
          getHomeDir(path);
          strcat(path, untitledFile);
          filename = path;
          editWidget->doSaveFile(filename);
        } else if (access(filename, W_OK) == 0) {
          editWidget->doSaveFile(filename);
        }
      }
      basicMain(editWidget, filename, false);
    } else {
      busyMessage();
    }
  }
}

void MainWindow::run_break(fltk::Widget *w, void *eventData) {
  if (runMode == modal_state && !count_tasks()) {
    // break from modal edit mode loop
    runMode = edit_state;
  } else if (runMode == run_state || runMode == modal_state) {
    setBreak();
  }
}

/**
 * run the selected text as the main program
 */
void MainWindow::run_selection(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    char path[MAX_PATH];
    getHomeDir(path);
    strcat(path, "selection.bas");
    editWidget->saveSelection(path);
    basicMain(editWidget, path, false);
  }
}

/**
 * callback for editor-plug-in plug-ins. we assume the target
 * program will be changing the contents of the editor buffer
 */
void MainWindow::editor_plugin(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
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
        editWidget->runState(rs_run);
        sprintf(path, "%s/%s", packageHome, (const char *)eventData);
        int interactive = opt_interactive;
        opt_interactive = false;
        int success = sbasic_main(path);
        opt_interactive = interactive;
        editWidget->runState(success ? rs_ready : rs_err);
        editWidget->loadFile(filename);
        editor->insert_position(pos);
        editor->show_insert_position();
        editWidget->setRowCol(row, col + 1);
        showEditTab(editWidget);
        runMode = edit_state;
        opt_command[0] = 0;
      }
    } else {
      busyMessage();
    }
  }
}

/**
 * callback for tool-plug-in plug-ins.
 */
void MainWindow::tool_plugin(fltk::Widget *w, void *eventData) {
  if (runMode == edit_state) {
    char path[MAX_PATH];
    sprintf(opt_command, "%s/%s", packageHome, pluginHome);
    statusMsg(rs_ready, (const char *)eventData);
    sprintf(path, "%s/%s", packageHome, (const char *)eventData);
    _tabGroup->selected_child(_outputGroup);
    basicMain(0, path, true);
    statusMsg(rs_ready, 0);
    opt_command[0] = 0;
  } else {
    busyMessage();
  }
}

void MainWindow::load_file(fltk::Widget * w, void *eventData) {
  int pathIndex = ((intptr_t) eventData) - 1;
  const char *path = recentPath[pathIndex].toString();
  EditorWidget *editWidget = getEditor(path);
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
      editWidget->fileChanged(true);
      const char *slash = strrchr(path, '/');
      editWidget->parent()->copy_label(slash ? slash + 1 : path);
      showEditTab(editWidget);
    } else {
      pathMessage(path);
    }
  }
}

//--Startup functions-----------------------------------------------------------

/**
 *Adds a plug-in to the menu
 */
void MainWindow::addPlugin(Menu *menu, const char *label, const char *filename) {
  char path[MAX_PATH];
  sprintf(path, "%s/%s", pluginHome, filename);
  if (access(path, R_OK) == 0) {
    menu->add(label, 0, editor_plugin_cb, strdup(path));
  }
}

/**
 * scan for recent files
 */
void MainWindow::scanRecentFiles(Menu *menu) {
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
      buffer[strlen(buffer) - 1] = 0;   // trim new-line
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
    sprintf(label, "&File/Open Recent File/%s", untitledFile);
    recentMenu[i] = menu->add(label, CTRL + '1' + i, (Callback *)
                              load_file_cb, (void *)(i + 1));
    recentPath[i].append(untitledFile);
    i++;
  }
}

/**
 * scan for optional plugins
 */
void MainWindow::scanPlugIns(Menu *menu) {
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
    const char *filename = e->d_name;
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
      FileWidget::trimEOL(buffer);
      if (strcmp("'tool-plug-in", buffer) == 0) {
        editorTool = true;
      } else if (strcmp("'app-plug-in", buffer) != 0) {
        fclose(file);
        continue;
      }

      if (fgets(buffer, MAX_PATH, file) && strncmp("'menu", buffer, 5) == 0) {
        FileWidget::trimEOL(buffer);
        int offs = 6;
        while (buffer[offs] && (buffer[offs] == '\t' || buffer[offs] == ' ')) {
          offs++;
        }
        sprintf(label, (editorTool ? "&Edit/Basic/%s" : "&Basic/%s"), buffer + offs);
        // use an absolute path
        sprintf(path, "%s/%s", pluginHome, filename);
        menu->add(label, 0, (Callback *)
                  (editorTool ? editor_plugin_cb : tool_plugin_cb), strdup(path));
      }
      fclose(file);
    }
  }
  // cleanup
  closedir(dp);
}

/**
 * process the program command line arguments
 */
int arg_cb(int argc, char **argv, int &i) {
  const char *s = argv[i];
  int len = strlen(s);
  int c;

  if (strcasecmp(s + len - 4, ".bas") == 0 && access(s, 0) == 0) {
    runfile = strdup(s);
    runMode = run_state;
    i += 1;
    return 1;
  }

  if (argv[i][0] == '-') {
    if (!argv[i][2] && argv[i + 1]) {
      // commands that take an additional file name argument
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

    switch (argv[i][1]) {
    case 'n':
      i += 1;
      opt_interactive = 0;
      return 1;

    case 'v':
      i += 1;
      opt_verbose = 1;
      opt_quiet = 0;
      return 1;

    case '-':
      // echo foo | sbasic foo.bas --
      while ((c = fgetc(stdin)) != EOF) {
        int len = strlen(opt_command);
        opt_command[len] = c;
        opt_command[len + 1] = 0;
      }
      i++;
      return 1;
    }
  }

  if (runMode == run_state) {
    // remaining text is .bas program argument
    if (opt_command[0]) {
      strcat(opt_command, " ");
    }
    strcat(opt_command, s);
    i++;
    return 1;
  }

  return 0;
}

/**
 * start the application in run mode
 */
void run_mode_startup(void *data) {
  if (data) {
    Window *w = (Window *)data;
    w->destroy();
  }

  EditorWidget *editWidget = wnd->getEditor(true);
  if (editWidget) {
    editWidget->setHideIde(true);
    editWidget->loadFile(runfile);

    opt_ide = IDE_NONE;
    if (!wnd->basicMain(0, runfile, false)) {
      exit(0);
    }
    editWidget->editor->take_focus();
    opt_ide = IDE_LINKED;
  }
}

/**
 * prepare to start the application
 */
bool initialise(int argc, char **argv) {
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
  char *slash = (char *)FileWidget::forwardSlash(packageHome);
  if (slash) {
    *slash = 0;
  }
#else
  packageHome = (char *)PACKAGE_DATA_DIR;
#endif

  char path[MAX_PATH];
  sprintf(path, "PKG_HOME=%s", packageHome);
  dev_putenv(path);

  // bas_home contains user editable files along with generated help
  strcpy(path, basHome);
  getHomeDir(path + strlen(basHome), false);
  dev_putenv(path);

  wnd = new MainWindow(800, 650);

  // load startup editors
  wnd->new_file(0, 0);
  wnd->_profile->restore(wnd);

  // setup styles
  Font *defaultFont = HELVETICA;
  if (defaultFont) {
    fltk::Widget::default_style->labelfont(defaultFont);
    fltk::Button::default_style->labelfont(defaultFont);
    fltk::Widget::default_style->textfont(defaultFont);
    fltk::Button::default_style->textfont(defaultFont);
  }

  Window::xclass("smallbasic");

  wnd->loadIcon(PACKAGE_PREFIX, 101);
  check();

  Window *run_wnd;              // required for x11 plumbing

  switch (runMode) {
  case run_state:
    run_wnd = new Window(0, 0);
    run_wnd->show();
    add_timeout(0.5f, run_mode_startup, run_wnd);
    break;

  case edit_state:
    wnd->getEditor(true)->loadFile(runfile);
    break;

  default:
    runMode = edit_state;
  }

  if (runMode != run_state) {
    wnd->show(argc, argv);
  }

  return true;
}

/**
 * save application state at program exit
 */
void save_profile(void) {
  wnd->_profile->save(wnd);
}

/**
 * application entry point
 */
int main(int argc, char **argv) {
  initialise(argc, argv);
  atexit(save_profile);
  run();
  return 0;
}

//--MainWindow methods----------------------------------------------------------

MainWindow::MainWindow(int w, int h) : 
  BaseWindow(w, h) {
  _runEditWidget = 0;
  _profile = new Profile();
  size_range(250, 250);

  FileWidget::forwardSlash(runfile);
  begin();
  MenuBar *m = new MenuBar(0, 0, w, MNU_HEIGHT);
  m->add("&File/&New File", CTRL + 'n', new_file_cb);
  m->add("&File/&Open File", CTRL + 'o', open_file_cb);
  scanRecentFiles(m);
  m->add("&File/_&Close", CTRL + F4Key, close_tab_cb);
  m->add("&File/&Save File", CTRL + 's', EditorWidget::save_file_cb);
  m->add("&File/_Save File &As", CTRL + SHIFT + 'S', save_file_as_cb);
  addPlugin(m, "&File/Publish Online", "publish.bas");
  m->add("&File/_Export", CTRL + F9Key, export_file_cb);
  m->add("&File/E&xit", CTRL + 'q', quit_cb);
  m->add("&Edit/_&Undo", CTRL + 'z', EditorWidget::undo_cb);
  m->add("&Edit/Cu&t", CTRL + 'x', EditorWidget::cut_text_cb);
  m->add("&Edit/&Copy", CTRL + 'c', copy_text_cb);
  m->add("&Edit/_&Paste", CTRL + 'v', EditorWidget::paste_text_cb);
  m->add("&Edit/_&Select All", CTRL + 'a', EditorWidget::select_all_cb);
  m->add("&Edit/&Change Case", ALT + 'c', EditorWidget::change_case_cb);
  m->add("&Edit/&Expand Word", ALT + '/', EditorWidget::expand_word_cb);
  m->add("&Edit/_&Rename Word", CTRL + SHIFT + 'r', EditorWidget::rename_word_cb);
  m->add("&Edit/&Find", CTRL + 'f', EditorWidget::find_cb);
  m->add("&Edit/&Replace", CTRL + 'r', EditorWidget::show_replace_cb);
  m->add("&Edit/_&Goto Line", CTRL + 'g', EditorWidget::goto_line_cb);
  m->add("&View/&Next Tab", F6Key, next_tab_cb);
  m->add("&View/_&Prev Tab", CTRL + F6Key, prev_tab_cb);
  m->add("&View/Text Size/&Increase", CTRL + ']', font_size_incr_cb);
  m->add("&View/Text Size/&Decrease", CTRL + '[', font_size_decr_cb);
  m->add("&View/Text Color/Background Def", 0, EditorWidget::set_color_cb, (void *)st_background_def);
  m->add("&View/Text Color/_Background", 0, EditorWidget::set_color_cb, (void *)st_background);
  m->add("&View/Text Color/Text", 0, EditorWidget::set_color_cb, (void *)st_text);
  m->add("&View/Text Color/Comments", 0, EditorWidget::set_color_cb, (void *)st_comments);
  m->add("&View/Text Color/_Strings", 0, EditorWidget::set_color_cb, (void *)st_strings);
  m->add("&View/Text Color/Keywords", 0, EditorWidget::set_color_cb, (void *)st_keywords);
  m->add("&View/Text Color/Funcs", 0, EditorWidget::set_color_cb, (void *)st_funcs);
  m->add("&View/Text Color/_Subs", 0, EditorWidget::set_color_cb, (void *)st_subs);
  m->add("&View/Text Color/Numbers", 0, EditorWidget::set_color_cb, (void *)st_numbers);
  m->add("&View/Text Color/Operators", 0, EditorWidget::set_color_cb, (void *)st_operators);
  m->add("&View/Text Color/Find Matches", 0, EditorWidget::set_color_cb, (void *)st_findMatches);

  new ScanFont(m);
  scanPlugIns(m);

  m->add("&Program/&Run", F9Key, run_cb);
  m->add("&Program/_&Run Selection", F8Key, run_selection_cb);
  m->add("&Program/&Break", CTRL + 'b', run_break_cb);
  m->add("&Program/_&Restart", CTRL + 'r', restart_run_cb);
  m->add("&Program/_&Command", F10Key, set_options_cb);
  m->add("&Help/_&Help Contents", F1Key, help_contents_cb);
  m->add("&Help/&Program Help", F11Key, help_app_cb);
  m->add("&Help/_&Home Page", 0, help_home_cb);
  m->add("&Help/&About SmallBASIC", F12Key, help_about_cb);

  callback(quit_cb);
  shortcut(0);                  // remove default EscapeKey shortcut

  // outer decoration group
  h -= MNU_HEIGHT;
  Group *outer = new Group(0, MNU_HEIGHT, w, h);
  outer->begin();
  outer->box(ENGRAVED_BOX);

  // group for all tabs
  w -= 8;

  _tabGroup = new TabGroup(4, 4, w, h - 6);
  _tabGroup->box(NO_BOX);

  // create the output tab
  h -= (MNU_HEIGHT + 8);
  _tabGroup->begin();
  _outputGroup = new Group(0, MNU_HEIGHT, w, h, "Output");
  _outputGroup->box(THIN_DOWN_BOX);
  _outputGroup->hide();
  _outputGroup->user_data((void *)gw_output);
  _outputGroup->begin();
  _out = new MosyncWidget(2, 2, w - 4, h - 4, DEF_FONT_SIZE);
  _outputGroup->resizable(_out);
  _outputGroup->end();

  _tabGroup->resizable(_outputGroup);
  _tabGroup->end();
  outer->end();
  end();

  outer->resizable(_tabGroup);
  resizable(outer);
}

/**
 * create a new help widget and add it to the tab group
 */
Group *MainWindow::createEditor(const char *title) {
  int w = _tabGroup->w();
  int h = _tabGroup->h() - MNU_HEIGHT;

  _tabGroup->begin();
  Group *editGroup = new Group(0, MNU_HEIGHT, w, h - 2);
  const char *slash = strrchr(title, '/');
  editGroup->copy_label(slash ? slash + 1 : title);
  editGroup->begin();
  editGroup->box(THIN_DOWN_BOX);
  editGroup->resizable(new EditorWidget(2, 2, w - 4, h - 2));

  editGroup->user_data((void *)gw_editor);
  editGroup->end();

  _tabGroup->add(editGroup);
  _tabGroup->selected_child(editGroup);
  _tabGroup->end();
  return editGroup;
}

void MainWindow::new_file(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = 0;
  Group *untitledEditor = findTab(untitledFile);
  char path[MAX_PATH];

  if (untitledEditor) {
    _tabGroup->selected_child(untitledEditor);
    editWidget = getEditor(untitledEditor);
  }
  if (!editWidget) {
    editWidget = getEditor(createEditor(untitledFile));

    // preserve the contents of any existing untitled.bas
    getHomeDir(path);
    strcat(path, untitledFile);
    if (access(path, 0) == 0) {
      editWidget->loadFile(path);
    }
  }

  TextBuffer *textbuf = editWidget->editor->buffer();
  textbuf->select(0, textbuf->length());
}

void MainWindow::open_file(fltk::Widget *w, void *eventData) {
  FileWidget *fileWidget = null;
  Group *openFileGroup = findTab(gw_file);
  if (!openFileGroup) {
    int w = _tabGroup->w();
    int h = _tabGroup->h() - MNU_HEIGHT;
    _tabGroup->begin();
    openFileGroup = new Group(0, MNU_HEIGHT, w, h, fileTabName);
    openFileGroup->begin();
    fileWidget = new FileWidget(2, 2, w - 4, h - 4);
    openFileGroup->box(THIN_DOWN_BOX);
    openFileGroup->user_data((void *)gw_file);
    openFileGroup->resizable(fileWidget);
    openFileGroup->end();
    _tabGroup->end();
  } else {
    fileWidget = (FileWidget *)openFileGroup->resizable();
  }

  // change to the directory of the current editor widget
  EditorWidget *editWidget = getEditor(false);
  char path[MAX_PATH];

  if (editWidget) {
    FileWidget::splitPath(editWidget->getFilename(), path);
  } else {
    Group *group = (Group *)_tabGroup->selected_child();
    GroupWidget gw = getGroupWidget(group);
    switch (gw) {
    case gw_output:
      strcpy(path, packageHome);
      break;
    case gw_help:
      getHomeDir(path);
      break;
    default:
      path[0] = 0;
    }
  }

  fileWidget->openPath(path);

  _tabGroup->selected_child(openFileGroup);
}

void MainWindow::save_file_as(fltk::Widget *w, void *eventData) {
  EditorWidget *editWidget = getEditor();
  if (editWidget) {
    open_file(w, eventData);
    FileWidget *fileWidget = (FileWidget *)getSelectedTab()->resizable();
    fileWidget->fileOpen(editWidget);
  }
}

HelpWidget *MainWindow::getHelp() {
  HelpWidget *help = 0;
  Group *helpGroup = findTab(gw_help);
  if (!helpGroup) {
    int w = _tabGroup->w();
    int h = _tabGroup->h() - MNU_HEIGHT;
    _tabGroup->begin();
    helpGroup = new Group(0, MNU_HEIGHT, w, h, helpTabName);
    helpGroup->box(THIN_DOWN_BOX);
    helpGroup->hide();
    helpGroup->user_data((void *)gw_help);
    helpGroup->begin();
    help = new HelpWidget(2, 2, w - 4, h - 4);
    help->callback(help_contents_anchor_cb);
    helpGroup->resizable(help);
    _tabGroup->end();
  } else {
    help = (HelpWidget *)helpGroup->resizable();
  }
  _tabGroup->selected_child(helpGroup);
  return help;
}

EditorWidget *MainWindow::getEditor(bool select) {
  EditorWidget *result = 0;
  if (select) {
    int n = _tabGroup->children();
    for (int c = 0; c < n; c++) {
      Group *group = (Group *)_tabGroup->child(c);
      if (gw_editor == getGroupWidget(group)) {
        result = (EditorWidget *)group->child(0);
        _tabGroup->selected_child(group);
        break;
      }
    }
  } else {
    result = getEditor(getSelectedTab());
  }
  return result;
}

EditorWidget *MainWindow::getEditor(Group *group) {
  EditorWidget *editWidget = 0;
  if (group != 0 && gw_editor == getGroupWidget(group)) {
    editWidget = (EditorWidget *)group->resizable();
  }
  return editWidget;
}

EditorWidget *MainWindow::getEditor(const char *fullpath) {
  if (fullpath != 0 && fullpath[0] != 0) {
    int n = _tabGroup->children();
    for (int c = 0; c < n; c++) {
      Group *group = (Group *)_tabGroup->child(c);
      if (gw_editor == getGroupWidget(group)) {
        EditorWidget *editWidget = (EditorWidget *)group->child(0);
        const char *fileName = editWidget->getFilename();
        if (fileName && strcmp(fullpath, fileName) == 0) {
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
void MainWindow::editFile(const char *filePath) {
  EditorWidget *editWidget = getEditor(filePath);
  if (!editWidget) {
    editWidget = getEditor(createEditor(filePath));
    editWidget->loadFile(filePath);
  }
  showEditTab(editWidget);
}

Group *MainWindow::getSelectedTab() {
  return (Group *)_tabGroup->selected_child();
}

/**
 * returns the tab with the given name
 */
Group *MainWindow::findTab(const char *label) {
  int n = _tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group *child = (Group *)_tabGroup->child(c);
    if (strcmp(child->label(), label) == 0) {
      return child;
    }
  }
  return 0;
}

Group *MainWindow::findTab(GroupWidget groupWidget) {
  int n = _tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group *child = (Group *)_tabGroup->child(c);
    if (groupWidget == getGroupWidget(child)) {
      return child;
    }
  }
  return 0;
}

/**
 * find and select the tab with the given tab label
 */
Group *MainWindow::selectTab(const char *label) {
  Group *tab = findTab(label);
  if (tab) {
    _tabGroup->selected_child(tab);
  }
  return tab;
}

/**
 * copies the configuration from the current editor to any remaining editors
 */
void MainWindow::updateConfig(EditorWidget *current) {
  int n = _tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group *group = (Group *)_tabGroup->child(c);
    if (gw_editor == getGroupWidget(group)) {
      EditorWidget *editWidget = (EditorWidget *)group->child(0);
      if (editWidget != current) {
        editWidget->updateConfig(current);
      }
    }
  }
}

/**
 * updates the names of the editor tabs based on the enclosed editing file
 */
void MainWindow::updateEditTabName(EditorWidget *editWidget) {
  int n = _tabGroup->children();
  for (int c = 0; c < n; c++) {
    Group *group = (Group *)_tabGroup->child(c);
    if (gw_editor == getGroupWidget(group) && editWidget == (EditorWidget *)group->child(0)) {
      const char *editFileName = editWidget->getFilename();
      if (editFileName && editFileName[0]) {
        const char *slash = strrchr(editFileName, '/');
        group->copy_label(slash ? slash + 1 : editFileName);
      }
    }
  }
}

/**
 * returns the tab following the given tab
 */
Group *MainWindow::getNextTab(Group *current) {
  int n = _tabGroup->children();
  for (int c = 0; c < n - 1; c++) {
    Group *child = (Group *)_tabGroup->child(c);
    if (child == current) {
      return (Group *)_tabGroup->child(c + 1);
    }
  }
  return (Group *)_tabGroup->child(0);
}

/**
 * returns the tab prior the given tab or null if not found
 */
Group *MainWindow::getPrevTab(Group *current) {
  int n = _tabGroup->children();
  for (int c = n - 1; c > 0; c--) {
    Group *child = (Group *)_tabGroup->child(c);
    if (child == current) {
      return (Group *)_tabGroup->child(c - 1);
    }
  }
  return (Group *)_tabGroup->child(n - 1);
}

/**
 * Opens the config file ready for writing
 */
FILE *MainWindow::openConfig(const char *fileName, const char *flags) {
  char path[MAX_PATH];
  getHomeDir(path);
  strcat(path, fileName);
  return fopen(path, flags);
}

bool MainWindow::isBreakExec(void) {
  return (runMode == break_state || runMode == quit_state);
}

bool MainWindow::isRunning(void) {
  return (runMode == run_state || runMode == modal_state);
}

void MainWindow::setModal(bool modal) {
  runMode = modal ? modal_state : run_state;
}

/**
 * sets the window title based on the filename
 */
void MainWindow::setTitle(Window *widget, const char *filename) {
  char title[MAX_PATH];
  const char *dot = strrchr(filename, '.');
  int len = (dot ? dot - filename : strlen(filename));

  strncpy(title, filename, len);
  title[len] = 0;
  title[0] = toupper(title[0]);
  strcat(title, " - SmallBASIC");
  widget->copy_label(title);
}

void MainWindow::setBreak() {
  brun_break();
  runMode = break_state;
}

bool MainWindow::isModal() {
  return (runMode == modal_state);
}

bool MainWindow::isEdit() {
  return (runMode == edit_state);
}

bool MainWindow::isInteractive() {
  return opt_interactive;
}

bool MainWindow::isIdeHidden() {
  return (opt_ide == IDE_NONE);
}

void MainWindow::resetPen() {
  _penDownX = 0;
  _penDownY = 0;
  _penMode = 0;
}

/**
 * returns any active tty widget
 */
TtyWidget *MainWindow::tty() {
  TtyWidget *result = 0;
  EditorWidget *editor = _runEditWidget;
  if (!editor) {
    editor = getEditor(false);
  }
  if (editor) {
    result = editor->tty;
  }
  return result;
}

/**
 * returns whether printing to the tty widget is active
 */
bool MainWindow::logPrint() {
  return (_runEditWidget && _runEditWidget->tty && _runEditWidget->isLogPrint());
}

void MainWindow::execLink(strlib::String &link) {
  if (!link.length()) {
    return;
  }

  char *file = (char *)link.toString();
  EditorWidget *editWidget = getEditor(true);
  _siteHome.empty();
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
      statusMsg(rs_ready, localFile);
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
      basicMain(editWidget, localFile, false);
    } else {
      // display as html
      int len = strlen(localFile);
      if (strcasecmp(localFile + len - 4, ".gif") == 0 ||
          strcasecmp(localFile + len - 4, ".jpeg") == 0 || 
          strcasecmp(localFile + len - 4, ".jpg") == 0) {
        sprintf(path, "<img src=%s>", localFile);
      } else {
        sprintf(path, "file:%s", localFile);
      }
      _siteHome.append(df.name, df.drv_dw[1]);
      statusMsg(rs_ready, _siteHome.toString());
      updateForm(path);
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
    statusMsg(rs_ready, file);
    if (execFile) {
      basicMain(0, file, false);
      opt_nosave = 1;
    } else {
      if (!editWidget) {
        editWidget = getEditor(createEditor(file));
      }
      editWidget->loadFile(file);
      showEditTab(editWidget);
    }
  } else {
    pathMessage(file);
  }
}

/**
 * loads the desktop icon
 */
void MainWindow::loadIcon(const char *prefix, int resourceId) {
  if (!icon()) {
#if defined(WIN32)
    HICON ico = (HICON) wnd->icon();
    if (!ico) {
      ico = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceId),
                              IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_SHARED);
      if (!ico) {
        ico = LoadIcon(NULL, IDI_APPLICATION);
      }
    }
    wnd->icon((char *)ico);
#else
    char buffer[MAX_PATH];
    char path[MAX_PATH];
    const char *key = "Icon=";

    // read the application desktop file, then scan for the Icon file
    sprintf(path, "%s/share/applications/%s.desktop", prefix, Window::xclass());
    FILE *fp = fopen(path, "r");
    if (fp) {
      while (feof(fp) == 0 && fgets(buffer, sizeof(buffer), fp)) {
        buffer[strlen(buffer) - 1] = 0; // trim new-line
        if (strncasecmp(buffer, key, strlen(key)) == 0) {
          // found icon spec
          const char *filename = buffer + strlen(key);
          Image *ico = loadImage(filename, 0);  // in HelpWidget.cxx
          if (ico) {
            if (sizeof(unsigned) == ico->buffer_depth()) {
              // prefix the buffer with unsigned width and height values
              unsigned size = ico->buffer_width() * ico->buffer_height() * ico->buffer_depth();
              unsigned *image = (unsigned *)malloc(size + sizeof(unsigned) * 2);
              image[0] = ico->buffer_width();
              image[1] = ico->buffer_height();
              memcpy(image + 2, ico->buffer(), size);
              icon(image);
            }
            SharedImage::remove(filename);
          }
          break;
        }
      }
      fclose(fp);
    }
#endif
  }
}

int BaseWindow::handle(int e) {
  switch (runMode) {
  case run_state:
  case modal_state:
    switch (e) {
    case FOCUS:
      // accept key events into handleKeyEvent
      return 1;
    case PUSH:
      if (keymap_invoke(SB_KEY_MK_PUSH)) {
        return 1;
      }
      break;
    case DRAG:
      if (keymap_invoke(SB_KEY_MK_DRAG)) {
        return 1;
      }
      break;
    case MOVE:
      if (keymap_invoke(SB_KEY_MK_MOVE)) {
        return 1;
      }
      break;
    case RELEASE:
      if (keymap_invoke(SB_KEY_MK_RELEASE)) {
        return 1;
      }
      break;
    case MOUSEWHEEL:
      if (keymap_invoke(SB_KEY_MK_WHEEL)) {
        return 1;
      }
      break;
    case SHORTCUT:
    case KEY:
      if (handleKeyEvent()) {
        // no default processing by Window
        return 1;
      }
      break;
    }
    break;

  case edit_state:
    switch (e) {
    case SHORTCUT:
    case KEY:
      if (event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey)) {
        EditorWidget *editWidget = wnd->getEditor();
        if (editWidget) {
          if (event_key() == F1Key) {
            // CTRL + F1 key for brief log mode help
            wnd->help_contents(0, (void *)true);
            return 1;
          }
          if (editWidget->focusWidget()) {
            return 1;
          }
        }
      }
    }
    break;

  default:
    break;
  }

  return Window::handle(e);
}

bool BaseWindow::handleKeyEvent() {
  int k = event_key();
  bool key_pushed = true;

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
    if (event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey)) {
      wnd->run_break();
      key_pushed = false;
      break;
    }
    dev_pushkey(event_text()[0]);
    break;
  case 'q':
    if (event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey)) {
      wnd->quit();
      key_pushed = false;
      break;
    }
    dev_pushkey(event_text()[0]);
    break;

  default:
    if (k >= LeftShiftKey && k <= RightAltKey) {
      key_pushed = false;
      break;                    // ignore caps+shift+ctrl+alt
    }
    dev_pushkey(event_text()[0]);
    break;
  }
  return key_pushed;
}

LineInput::LineInput(int x, int y, int w, int h) : 
  fltk::Input(x, y, w, h) {
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
    int strw = (int)(getwidth(text) + getwidth(value())) + 4;
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
    if ((event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey)) && event_key() == 'b') {
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
