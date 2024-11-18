// This file is part of SmallBASIC
//
// Copyright(C) 2001-2024 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <oboe/Oboe.h>
#include <cstdint>
#include <cmath>
#include <unistd.h>

#include "config.h"
#include "ui/strlib.h"
#include "ui/utils.h"
#include "lib/maapi.h"
#include "audio.h"

const int32_t AUDIO_SAMPLE_RATE = 48000;
const float PI2 = 2.0f * M_PI;

struct Sound {
  Sound(int frequency, int millis, int volume);
  ~Sound() = default;

  int16_t sample();
  bool ready();

private:
  int32_t _duration;
  int32_t _start;
  int32_t _samples;
  int32_t _sampled;
  float _amplitude;
  float _increment;
  float _phase;
};

Sound::Sound(int frequency, int millis, int volume) :
  _duration(millis),
  _start(0),
  _samples(millis * AUDIO_SAMPLE_RATE / 1000),
  _sampled(0),
  _amplitude((float)volume / 100 * INT16_MAX),
  _increment((float)frequency / AUDIO_SAMPLE_RATE * PI2),
  _phase(_increment) {
}

bool Sound::ready() {
  bool result;
  if (_start == 0) {
    _start = maGetMilliSecondCount();
    result = true;
  } else if (_sampled > _samples || maGetMilliSecondCount() - _start > _duration) {
    result = false;
  } else {
    result = true;
  }
  return result;
}

int16_t Sound::sample() {
  auto result = static_cast<int16_t>(sinf(_phase) * _amplitude);
  _sampled++;
  _phase += _increment;
  if (_phase >= PI2) {
    // wrap phase
    _phase -= PI2;
  }
  return result;
}


Audio::Audio() {
  oboe::AudioStreamBuilder builder;
  oboe::AudioStream *stream = nullptr;
  builder.setDirection(oboe::Direction::Output)
    ->setFormat(oboe::AudioFormat::I16)
    ->setChannelCount(oboe::ChannelCount::Mono)
    ->setSampleRate(44100)
    ->openStream(&stream);

}

Audio::~Audio() {
  clearSoundQueue();
}

void Audio::play(int frequency, int millis, int volume, bool background) {
}

//
// Remove any cached sounds from the queue
//
void Audio::clearSoundQueue() {
  logEntered();
  pthread_mutex_lock(&_mutex);

  List_each(Sound *, it, _queue) {
    Sound *next = *it;
    if (next != nullptr) {
      _queue.remove(it);
      it--;
    }
  }
  _queue.removeAll();

  pthread_mutex_unlock(&_mutex);
}

Sound *Audio::front() {
  return nullptr;
}
