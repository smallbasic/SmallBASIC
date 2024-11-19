// This file is part of SmallBASIC
//
// Copyright(C) 2001-2024 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <cstdint>
#include <cmath>
#include <unistd.h>

#include "config.h"
#include "ui/strlib.h"
#include "ui/utils.h"
#include "lib/maapi.h"
#include "audio.h"

//see: https://github.com/google/oboe/blob/main/docs/GettingStarted.md
constexpr int32_t AUDIO_SAMPLE_RATE = 48000;
constexpr float PI2 = 2.0f * M_PI;

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
  _amplitude((float)volume * INT16_MAX / 100),
  _increment((float)frequency * PI2 / AUDIO_SAMPLE_RATE),
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
  AudioStreamBuilder builder;
  AudioStream *stream = nullptr;
  builder.setDirection(Direction::Output)
    ->setSharingMode(SharingMode::Exclusive)
    ->setPerformanceMode(PerformanceMode::LowLatency)
    ->setChannelCount(1)
    ->setFormat(AudioFormat::I16)
    ->setChannelCount(ChannelCount::Mono)
    ->setSampleRate(AUDIO_SAMPLE_RATE)
    ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
    ->setDataCallback(this)
    ->openStream(&stream);
}

Audio::~Audio() {
  clearSoundQueue();
  std::lock_guard<std::mutex> lock(_lock);
  if (_stream) {
    _stream->stop();
    _stream->close();
    _stream.reset();
  }
}

DataCallbackResult Audio::onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) {
  DataCallbackResult result;
  Sound *sound = front();
  if (sound == nullptr) {
    result = DataCallbackResult::Stop;
  } else {
    auto *buffer = (int16_t *)audioData;
    for (int i = 0; i < numFrames; ++i) {
      int16_t sample = sound->sample();
      buffer[i] = sample;
    }
    result = DataCallbackResult::Continue;
  }
  return result;
}

void Audio::add(int frequency, int millis, int volume) {
  std::lock_guard<std::mutex> lock(_lock);
  _queue.push(new Sound(frequency, millis, volume));
}

void Audio::play(int frequency, int millis, int volume, bool background) {
  if (millis > 0) {
    add(frequency, millis, volume);
    _stream->requestStart();

    if (!background) {
      usleep(millis * 1000);
      _stream->requestStop();
      std::lock_guard<std::mutex> lock(_lock);
      _queue.pop();
    }
  }
}

//
// Remove any cached sounds from the queue
//
void Audio::clearSoundQueue() {
  logEntered();
  std::lock_guard<std::mutex> lock(_lock);
  while (!_queue.empty()) {
    Sound* sound = _queue.front();
    _queue.pop();
    delete sound;
  }
}

Sound *Audio::front() {
  Sound *result = nullptr;
  std::lock_guard<std::mutex> lock(_lock);

  while (!_queue.empty()) {
    result = _queue.front();
    if (result->ready()) {
      break;
    }
    result = nullptr;
    _queue.pop();
  }

  return result;

}
