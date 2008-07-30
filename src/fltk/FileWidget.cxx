// $Id: EditorWidget.cxx 623 2008-07-15 12:53:20Z zeeb90au $
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
#include <sys/stat.h>
#include <fltk/run.h>

#include "MainWindow.h"
#include "StringLib.h"
#include "HelpWidget.h"
#include "FileWidget.h"

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
  setSelectMode();

  getcwd(path, sizeof(path));
  int len = path ? strlen(path) : 0;
  for (int i=0; i<len; i++) {
    if (path[i] == '\\') {
      path[i] = '/';
    }
  }

  displayPath();
}

FileWidget::~FileWidget() 
{
  fileWidget = 0;
}

void FileWidget::fileOpen() 
{
  if (saveAsPath[0]) {
    saveAsPath[0] = 0;
    displayPath();
  }
}

void FileWidget::fileSaveAs(const char* filename) 
{
  strcpy(saveAsPath, filename);
  displayPath();
}

void FileWidget::displayPath() 
{
  chdir(path);
  DIR* dp = opendir(path);
  if (dp == 0) {
    return;
  }

  String html;
  dirent* entry;
  struct stat stbuf;

  if (saveAsPath[0]) {
    html.append("<p><b>Save ").append(saveAsPath).append(" as:<br>");
    html.append("<input size=120 type=text value='").append(saveAsPath).append("' name=saveas>");
    html.append("&nbsp;<input type=button onclick='~' value='Save As'><br>");
  }

  html.append("<br><b>Files in: ").append(path).append("</b><br>");

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
        html.append("<input type=button onclick='!..' value='@<-;'>");
      }
      else {
         html.append("<p><a href=!");
         html.append(name);
         html.append(">[");
         html.append(name);
         html.append("]</a>");
      }
    } 
    else if (strncasecmp(name+len-4, ".htm", 4) == 0 ||
             strncasecmp(name+len-5, ".html", 5) == 0||
             strncasecmp(name+len-4, ".bas", 4) == 0 ||
             strncasecmp(name+len-4, ".txt", 4) == 0) {
      html.append("<p><a href=");
      html.append(name);
      html.append(">");
      html.append(name);
      html.append("</a>");
    }
  }
  closedir(dp);

  // open into an exiting file open tab if one exists
  html.append("<br>");
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
    // TODO call wnd to rename file, then redisplay as normal file mode
    //ask(getInputValue(getInput("saveas")));
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

  setDocHome(docHome);
  String fullPath;
  fullPath.append(docHome.toString());
  fullPath.append("/");
  fullPath.append(target[0] == '/' ? target+1 : target);
  wnd->editFile(fullPath.toString());
}

// --- End of file -------------------------------------------------------------
