// This file is part of SmallBASIC
//
// Copyright(C) 2001-2024 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#pragma once

#include <oboe/Oboe.h>
#include <queue>

struct Sound;

using namespace oboe;

struct Audio : public AudioStreamDataCallback {
  explicit Audio();
  ~Audio() override;

  void play(int frequency, int millis, int volume, bool background);
  void clearSoundQueue();

  DataCallbackResult onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) override;

  private:
  Sound *front();
  void add(int frequency, int millis, int volume);
  void construct();
  void destroy();

  std::mutex _lock;
  std::shared_ptr<oboe::AudioStream> _stream;
  std::queue<Sound *> _queue;
};
