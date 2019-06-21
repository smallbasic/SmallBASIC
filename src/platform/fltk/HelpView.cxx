// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <config.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "platform/fltk/HelpView.h"
#include "ui/kwp.h"
#include "ui/strlib.h"

const char *aboutText =
  "<b>About SmallBASIC...</b><br><br>"
  "Version " SB_STR_VER "<br>"
  "Copyright (c) 2002-2019 Chris Warren-Smith.<br><br>"
  "Copyright (c) 2000-2006 Nicholas Christopoulos.<br><br>"
  "<a href=https://smallbasic.github.io>"
  "https://smallbasic.github.io</a><br><br>"
  "SmallBASIC comes with ABSOLUTELY NO WARRANTY. "
  "This program is free software; you can use it "
  "redistribute it and/or modify it under the terms of the "
  "GNU General Public License version 2 as published by "
  "the Free Software Foundation.<br><br>" "<i>Press F1 for help";

const char CMD_LEVEL1_OPEN = '+';
const char CMD_LEVEL2_OPEN = '!';
const char CMD_MORE = '^';
const char LEVEL1_OPEN[]   = "+ ";
const char LEVEL2_OPEN[]   = "  + ";
const char LEVEL1_CLOSE[]  = " - ";
const char LEVEL2_CLOSE[]  = "   - ";

HelpView *helpView;

const char *getBriefHelp(const char *selection) {
  const char *result = NULL;
  int len = selection != NULL ? strlen(selection) : 0;
  if (len > 0) {
    for (int i = 0; i < keyword_help_len && !result; i++) {
      if (strcasecmp(selection, keyword_help[i].keyword) == 0) {
        result = keyword_help[i].signature;
        break;
      }
    }
  }
  return result;
}

static void helpViewClick_event(void *) {
  Fl::remove_check(helpViewClick_event);
  helpView->anchorClick();
  helpView = NULL;
}

// post message and return
static void helpViewClick_cb(Fl_Widget *w, void *v) {
  helpView = (HelpView *)w;
  Fl::add_check(helpViewClick_event);
}

HelpView::HelpView(Fl_Widget *rect) :
  HelpWidget(rect),
  _openKeyword(-1),
  _openPackage(0) {
}

HelpView::~HelpView() {
}

void HelpView::about() {
  loadBuffer(aboutText);
}

//
// anchor link clicked
//
void HelpView::anchorClick() {
  const char *target = (const char *)user_data();

  switch (target[0]) {
  case CMD_LEVEL1_OPEN:
    for (int i = 0; i < keyword_help_len; i++) {
      if (strcasecmp(target + 1, keyword_help[i].package) == 0) {
        _openPackage = i;
        _openKeyword = i;
        break;
      }
    }
    helpIndex();
    break;

  case CMD_LEVEL2_OPEN:
    for (int i = 0; i < keyword_help_len; i++) {
      if (strcasecmp(target + 1, keyword_help[i].keyword) == 0) {
        _openKeyword = i;
        break;
      }
    }
    helpIndex();
    break;

  case CMD_MORE:
    if (_openKeyword != -1) {
      showHelp(keyword_help[_openKeyword].nodeId);
    }
    break;
  }
}

//
// display help index
//
void HelpView::helpIndex() {
  callback(helpViewClick_cb);

  String html;
  const char *package = NULL;

  for (int i = 0; i < keyword_help_len; i++) {
    if (package == NULL || strcasecmp(package, keyword_help[i].package) != 0) {
      package = keyword_help[i].package;
      bool bold = (_openPackage != -1 && strcasecmp(keyword_help[_openPackage].package,
                                                    keyword_help[i].package) == 0);
      if (bold) {
        html.append("<b>");
      }
      html.append("<a href='")
          .append(CMD_LEVEL1_OPEN)
          .append(package)
          .append("'>")
          .append(package)
          .append("</a>");
      if (strcasecmp("System", package) != 0) {
        html.append(" | ");
      }
      if (bold) {
        html.append("</b>");
      }
    }
  }

  if (_openKeyword != -1) {
    // display opened keyword
    html.append("<br><br><b>")
        .append(keyword_help[_openKeyword].signature)
        .append("</b><br>")
        .append(keyword_help[_openKeyword].help)
        .append("<br><u><a href=")
        .append(CMD_MORE)
        .append(">More</a></u>");
  }

  html.append("<br><br>");

  for (int i = 0; i < keyword_help_len; i++) {
    if (_openPackage != -1 && strcasecmp(keyword_help[_openPackage].package,
                                         keyword_help[i].package) == 0) {
      if (_openKeyword == i) {
        html.append("<b>");
      }
      // display opened package
      html.append("<a href='")
          .append(CMD_LEVEL2_OPEN)
          .append(keyword_help[i].keyword)
          .append("'>")
          .append(keyword_help[i].keyword)
          .append("</a> | ");
      if (_openKeyword == i) {
        html.append("</b>");
      }
    }
  }

  loadBuffer(html);
  take_focus();
}

bool HelpView::loadHelp(const char *path) {
  char localFile[PATH_MAX];
  dev_file_t df;
  bool result;

  memset(&df, 0, sizeof(dev_file_t));
  strcpy(df.name, path);
  if (http_open(&df) && cacheLink(&df, localFile, sizeof(localFile))) {
    loadFile(localFile);
    result = true;
  } else {
    result = false;
  }
  return result;
}

void HelpView::showContextHelp(const char *selection) {
  int len = selection != NULL ? strlen(selection) : 0;
  if (len > 0) {
    for (int i = 0; i < keyword_help_len; i++) {
      if (strcasecmp(selection, keyword_help[i].keyword) == 0) {
        _openPackage = _openKeyword = i;
        break;
      }
    }
  }
  helpIndex();
}

void HelpView::showHelp(const char *nodeId) {
  char path[PATH_MAX];
  sprintf(path, "http://smallbasic.github.io/reference/ide/%s.html", nodeId);
  loadHelp(path);
}
