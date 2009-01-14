// $Id$
//
// FileWidget
//
// Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "StringLib.h"
#include "HelpWidget.h"
#include "FileWidget.h"
#include "device.h"

FileWidget* fileWidget;
String click;

static void anchorClick_event(void *) 
{
  fltk::remove_check(anchorClick_event);
  fileWidget->anchorClick();
}

static void anchorClick_cb(Widget* w, void *v) 
{
  if (fileWidget) {
    click.empty();
    click.append((char*) v);
    fltk::add_check(anchorClick_event); // post message
  }
}

FileWidget::FileWidget(int x, int y, int w, int h) : HelpWidget(x, y, w, h) 
{
  callback(anchorClick_cb);
  fileWidget = this;
  saveEditorAs = 0;

  if (getcwd(path, sizeof(path))) {
    forwardSlash(path);
    displayPath();
  }
}

FileWidget::~FileWidget() 
{
  fileWidget = 0;
}

int FileWidget::handle(int e) {
  static char buffer[PATH_MAX];
  static int dnd_active = 0;

  switch (e) {
  case SHOW:
    if (saveEditorAs) {
      saveEditorAs = 0;
      displayPath();
    }
    break;

  case DND_LEAVE:
    dnd_active = 0;
    return 1;

  case DND_DRAG:
  case DND_RELEASE:
  case DND_ENTER:
    dnd_active = 1;
    return 1;

  case MOVE:
    if (dnd_active) {
      return 1; // return 1 to become drop-target
    }
    break;

  case PASTE:
    strncpy(buffer, fltk::event_text(), fltk::event_length());
    buffer[fltk::event_length()] = 0;
    forwardSlash(buffer);
    wnd->editFile(buffer);
    dnd_active = 0;
    return 1;
  }

  return HelpWidget::handle(e);
}

char* FileWidget::forwardSlash(char *filename)
{
  char* result = 0;
  int len = filename ? strlen(filename) : 0;
  for (int i = 0; i < len; i++) {
    if (filename[i] == '\\') {
      filename[i] = '/';
      result = &filename[i];
    }
  }
  return result;
}

void FileWidget::fileOpen(EditorWidget* saveEditorAs)
{
  this->saveEditorAs = saveEditorAs;
  displayPath();
}

void FileWidget::displayPath() 
{ 
  if (chdir(path) != 0) {
    return;
  }
  DIR* dp = opendir(path);
  if (dp == 0) {
    return;
  }

  dirent* entry;
  struct stat stbuf;
  String html;
  
  if (saveEditorAs) {
    const char* path = saveEditorAs->getFilename();
    char* slash = strrchr(path, '/');
    html.append("<p><b>Save ").append(slash ? slash + 1 : path).append(" as:<br>");
    html.append("<input size=220 type=text value='").append(slash ? slash + 1 : path);
    html.append("' name=saveas>&nbsp;<input type=button onclick='~' value='Save As'><br>");
  }

  html.append("<br><b>Files in: <a href=@>").append(path).append("</a></b><br>");

  while ((entry = readdir(dp)) != 0) {
    char* name = entry->d_name;
    int len = strlen(name);
    if (strcmp(name, ".") == 0) {
      continue;
    } 
    else if (strcmp(name, "..") == 0 && path[0] == '/' && path[1] == 0) {
      continue;
    }

    if (stat(name, &stbuf) != -1 && stbuf.st_mode & S_IFDIR) {
      if (!strcmp(name, "..")) {
        html.append("<input type=button onclick='!..' value='@<;'>");
      }
      else {
        html.append("<p><a href='!");
        html.append(name);
        html.append("'>[");
        html.append(name);
        html.append("]</a>");
      }
    } 
    else if (strncasecmp(name+len-4, ".htm", 4) == 0 ||
             strncasecmp(name+len-5, ".html", 5) == 0||
             strncasecmp(name+len-4, ".bas", 4) == 0 ||
             strncasecmp(name+len-4, ".txt", 4) == 0) {
      html.append("<p><a href=").append(name).append(">");
      html.append(name).append("</a>");
    }
  }
  closedir(dp);
  loadBuffer(html);
}

// anchor link clicked
void FileWidget::anchorClick() 
{
  const char* target = click.toString();
  if (target[0] == '!') {
    // file browser window
    if (strcmp(target+1, "..") == 0) {
      // go up a level c:/src/foo or /src/foo
      char* p = strrchr(path, '/');
      if (strchr(path, '/') != p) {
        *p = 0; // last item not first
      } 
      else {
        *(p+1) = 0; // found root
      }
    } 
    else {
      // go down a level
      if (path[strlen(path)-1] != '/') {
        strcat(path, "/");
      }
      strcat(path, target+1);
    }
    displayPath();
    return;
  }
  else if (target[0] == '~') {
    if (saveEditorAs) {
      const char* enteredPath = getInputValue(getInput("saveas"));
      if (enteredPath && enteredPath[0]) {
        // a path has been entered
        char savepath[PATH_MAX+1];
        if (enteredPath[0] == '~') {
          // substitute ~ for $HOME contents
          const char *home = dev_getenv("HOME");
          if (home) {
            strcpy(savepath, home);
          }
          else {
            savepath[0] = 0;
          }
          strcat(savepath, enteredPath + 1);
        }
        else if (enteredPath[0] == '/' || enteredPath[1] == ':') {
          // absolute path given
          strcpy(savepath, enteredPath);
        }
        else {
          strcpy(savepath, path);
          strcat(savepath, "/");
          strcat(savepath, enteredPath);
        }
        const char* msg = "%s\n\nFile already exists.\nDo you want to replace it?";
        if (access(savepath, 0) != 0 || ask(msg, savepath)) {
          saveEditorAs->doSaveFile(savepath);
        }
      }
    }
    return;
  }
  else if (target[0] == '@') {
    const char* newPath = fltk::input("Enter path:", path);
    if (newPath != 0) {
      if (chdir(newPath) == 0) {
        strcpy(path, newPath);
        displayPath();
      }
      else {
        message("Invalid path");
      }
    }
    return;
  }

  String docHome;
  if (target[0] == '/') {
    const char* base = getDocHome();
    if (base && base[0]) {
      // remove any overlapping string segments between the docHome
      // of the index page, eg c:/home/cache/smh/handheld/ and the 
      // anchored sub-page, eg, "/handheld/articles/2006/... "
      // ie, remove the URL component from the file name
      int len = strlen(base);
      const char* p = strchr(base+1, '/');
      while (p && *p && *(p+1) ) {
        if (strncmp(p, target, len-(p-base)) == 0) {
          len = p-base;
          break;
        }
        p = strchr(p+1, '/');
      }
      docHome.append(base, len);
    }  
  }
  else {
    docHome.append(path);
  }

  if (saveEditorAs) {
    Input* input = (Input*) getInput("saveas");
    input->value(target);
  } 
  else {
    setDocHome(docHome);
    String fullPath;
    fullPath.append(docHome.toString());
    fullPath.append("/");
    fullPath.append(target[0] == '/' ? target+1 : target);
    wnd->editFile(fullPath.toString());
  }
}

// --- End of file -------------------------------------------------------------
