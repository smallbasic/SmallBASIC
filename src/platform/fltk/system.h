// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef SYSTEM_H
#define SYSTEM_H

struct System {
  bool isBreak();
  bool isRunning();
  void buttonClicked(const char *url);
  MAEvent processEvents(bool wait);
};

#endif
