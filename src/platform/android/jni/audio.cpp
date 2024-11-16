// This file is part of SmallBASIC
//
// Copyright(C) 2001-2024 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <aaudio/AAudio.h>
#include <pthread.h>
#include <cstdint>
#include <cmath>

#include "config.h"
#include "ui/strlib.h"
#include "ui/utils.h"
#include "lib/maapi.h"
#include "audio.h"

const int32_t AUDIO_SAMPLE_RATE = 8000;
const float PI2 = 2.0f * 3.14159265359f;

int instances = 0;

struct Sound {
  Sound(float frequency, int millis, int volume);
  virtual ~Sound() {
    instances--;
  };

  int16_t sample();
  bool ready();

  private:
  uint32_t _volume;
  uint32_t _duration;
  uint32_t _start;
  float _increment;
  float _phase;
};

Sound::Sound(float frequency, int millis, int volume) :
  _volume(volume),
  _duration(millis),
  _start(0),
  _increment((frequency * PI2) / AUDIO_SAMPLE_RATE),
  _phase(0) {
  instances++;
}

bool Sound::ready() {
  bool result = true;
  if (_start == 0) {
    _start = maGetMilliSecondCount();
  } else if (maGetMilliSecondCount() - _start > _duration) {
    result = false;
  }
  return result;
}

int16_t Sound::sample() {
  float result = sinf(_phase);
  _phase += _increment;
  if (_phase >= PI2) {
    // wrap phase
    _phase -= PI2;
  }
  return static_cast<int16_t>(result * INT16_MAX * _volume / 100);
}

//
// Callback function to generate sine wave tone
//
aaudio_data_callback_result_t callback(AAudioStream* stream, void *userData, void *audioData, int32_t numFrames) {
  aaudio_data_callback_result_t result;
  Sound *sound = ((Audio *)userData)->front();
  if (sound == nullptr) {
    result = AAUDIO_CALLBACK_RESULT_STOP;
  } else {
    auto *buffer = (int16_t *)audioData;
    for (int i = 0; i < numFrames; ++i) {
      buffer[i] = sound->sample();
    }
    result = AAUDIO_CALLBACK_RESULT_CONTINUE;
  }
  return result;
}

AAudioStream* createAudioStream(Audio *audio) {
  AAudioStreamBuilder *builder = nullptr;
  AAudio_createStreamBuilder(&builder);

  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
  AAudioStreamBuilder_setSampleRate(builder, AUDIO_SAMPLE_RATE);
  AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
  AAudioStreamBuilder_setDataCallback(builder, callback, audio);

    //AAudioStreamBuilder_setUsage(builder, AAUDIO_USAGE_MEDIA);
    //AAudioStreamBuilder_setContentType(builder, AAUDIO_CONTENT_TYPE_MUSIC);
    //AAudioStreamBuilder_setChannelMask(builder, AAUDIO_CHANNEL_MONO);


  AAudioStream *result = nullptr;
  AAudioStreamBuilder_openStream(builder, &result);
  AAudioStreamBuilder_delete(builder);
  return result;
}

Audio::Audio() :
  _stream(createAudioStream(this)) {
  pthread_mutex_init(&_mutex, nullptr);
}

Audio::~Audio() {
  clearSoundQueue();
  AAudioStream_close(_stream);
  pthread_mutex_destroy(&_mutex);
  _stream = nullptr;
  trace("Leaked %d sound instances", instances);
}

void Audio::play(int frequency, int millis, int volume, bool background) {
  if (_stream == nullptr) {
    trace("Invalid stream");
  } else {
    pthread_mutex_lock(&_mutex);
    _queue.push(new Sound((float)frequency, millis, volume));
    pthread_mutex_unlock(&_mutex);

    AAudioStream_requestStart(_stream);

    if (!background) {
      maWait(millis);
      AAudioStream_requestStop(_stream);
      pthread_mutex_lock(&_mutex);
      _queue.pop();
      pthread_mutex_unlock(&_mutex);
      trace("Stopped playback.");
    }
  }
}

//
// returns the next active Sound from the queue
//
Sound *Audio::front() {
  Sound *result = nullptr;
  pthread_mutex_lock(&_mutex);

  while (!_queue.empty()) {
    result = _queue.front();
    if (result->ready()) {
      break;
    }
    _queue.pop();
  }

  pthread_mutex_unlock(&_mutex);
  return result;
}

//
// Remove any cached sounds from the queue
//
void Audio::clearSoundQueue() {
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
