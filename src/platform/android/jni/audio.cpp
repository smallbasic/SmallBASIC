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
constexpr int32_t AUDIO_SAMPLE_RATE = 44100;
constexpr float PI2 = 2.0f * M_PI;
constexpr int SILENCE_BEFORE_STOP = kMillisPerSecond * 5;

int instances = 0;

struct Sound {
  Sound(int frequency, int millis, int volume);
  ~Sound();

  bool ready();
  float sample();
  void sync(Sound *previous);

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

//
// Returns whether the sound is ready or completed
//
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

//
// Returns the next wave sample value
//
float Sound::sample() {
  auto result = sinf(_phase) * _amplitude;
  _sampled++;
  _phase = fmod(_phase + _increment, PI2);
  return result;
}

//
// Continues the same phase for the previous sound
//
void Sound::sync(Sound *previous) {
  _phase = previous->_phase;
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
    ->setSharingMode(SharingMode::Exclusive)
    ->setUsage(oboe::Usage::Game)
    ->openStream(_stream);
  if (result == oboe::Result::OK) {
    // play silence to initialise the player
    play(0, 1, 100, true);
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
      trace("Start audio");
      _stream->requestStart();
    }
    if (!background) {
      if (millis < kMillisPerSecond) {
        usleep(millis * kMillisPerSecond);
      } else {
        maWait(millis);
      }
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
  DataCallbackResult result = DataCallbackResult::Continue;
  Sound *sound = front();
  auto *buffer = (float *)audioData;
  if (sound == nullptr) {
    for (int i = 0; i < numFrames; ++i) {
      buffer[i] = 0;
    }
    // continue filling with silence in case the script requests further sounds
    if (_silenceStart != 0 && maGetMilliSecondCount() - _silenceStart > SILENCE_BEFORE_STOP) {
      trace("Stop audio. xruns: %d", oboeStream->getXRunCount());
      result = DataCallbackResult::Stop;
    }
  } else {
    for (int i = 0; i < numFrames; ++i) {
      auto sample = sound->sample();
      buffer[i] = sample;
    }
  }
  return result;
}

//
// Adds a sound to the queue
//
void Audio::add(int frequency, int millis, int volume) {
  std::lock_guard<std::mutex> lock(_lock);
  _queue.push(new Sound(frequency, millis, volume));
  _silenceStart = 0;
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

    _queue.pop();
    auto *next = _queue.front();
    if (next != nullptr) {
      next->sync(result);
    }
    result = nullptr;
  }

  if (result == nullptr && _silenceStart == 0) {
    _silenceStart = maGetMilliSecondCount();
  }

  return result;
}
