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
int instances = 0;

struct Sound {
  Sound(int frequency, int millis, int volume);
  ~Sound();

  float sample();
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
  _amplitude((float)volume / 100.0f),
  _increment((float)frequency * PI2 / AUDIO_SAMPLE_RATE),
  _phase(0) {
  instances++;
}

Sound::~Sound() {
  instances--;
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

float Sound::sample() {
  auto result = sinf(_phase) * _amplitude;
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
  Result result = builder.setDirection(Direction::Output)
    ->setChannelCount(ChannelCount::Mono)
    ->setDataCallback(this)
    ->setFormat(AudioFormat::Float)
    ->setPerformanceMode(PerformanceMode::LowLatency)
    ->setSampleRate(AUDIO_SAMPLE_RATE)
    ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
    ->setSharingMode(SharingMode::Shared)
    ->openStream(_stream);
  if (result == oboe::Result::OK) {
    // play silence to initialise the player
    play(0, 2000, 100, true);
  } else {
    _stream = nullptr;
  }
}

Audio::~Audio() {
  clearSoundQueue();
  std::lock_guard<std::mutex> lock(_lock);
  if (_stream != nullptr) {
    _stream->stop();
    _stream->close();
    _stream.reset();
  }
  trace("Leaked %d sound instances", instances);
}

//
// Play a sound with the given specification
//
void Audio::play(int frequency, int millis, int volume, bool background) {
  if (_stream != nullptr && millis > 0) {
    add(frequency, millis, volume);
    if (_stream->getState() != StreamState::Started) {
      _stream->requestStart();
    }
    if (!background) {
      usleep(millis * 1000);
    }
  }
}

//
// Remove any cached sounds from the queue
//
void Audio::clearSoundQueue() {
  logEntered();
  std::lock_guard<std::mutex> lock(_lock);
  _queue.removeAll();
}

//
// Callback to play the next block of audio
//
DataCallbackResult Audio::onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) {
  DataCallbackResult result;
  Sound *sound = front();
  auto *buffer = (float *)audioData;
  if (sound == nullptr) {
    for (int i = 0; i < numFrames; ++i) {
      buffer[i] = 0;
    }
    result = DataCallbackResult::Stop;
  } else {
    for (int i = 0; i < numFrames; ++i) {
      auto sample = sound->sample();
      buffer[i] = sample;
    }
    result = DataCallbackResult::Continue;
  }
  return result;
}

//
// Adds a sound to the queue
//
void Audio::add(int frequency, int millis, int volume) {
  std::lock_guard<std::mutex> lock(_lock);
  _queue.push(new Sound(frequency, millis, volume));
}

//
// Returns the next sound at the head of the queue
//
Sound *Audio::front() {
  std::lock_guard<std::mutex> lock(_lock);
  Sound *result = nullptr;

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
