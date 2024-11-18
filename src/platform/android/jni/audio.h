// This file is part of SmallBASIC
//
// Copyright(C) 2001-2024 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include <pthread.h>
#include "ui/strlib.h"

struct Sound;

struct Audio {
  Audio();
  virtual ~Audio();

  Sound *front();
  void play(int frequency, int millis, int volume, bool background);
  void clearSoundQueue();

  private:
  void construct();
  void destroy();

  pthread_mutex_t _mutex{};
  strlib::Queue<Sound *> _queue;
};
