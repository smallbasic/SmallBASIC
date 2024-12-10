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
constexpr int MAX_RAMP = 350;
int instances = 0;

struct Sound {
  Sound(int blockSize, int frequency, int millis, int volume);
  ~Sound();

  bool ready() const;
  float sample();
  void sync(Sound *previous);

private:
  uint32_t _duration;
  uint32_t _samples;
  uint32_t _sampled;
  uint32_t _ramp;
  float _amplitude;
  float _increment;
  float _phase;
};

Sound::Sound(int blockSize, int frequency, int millis, int volume) :
  _duration(millis),
  _samples(AUDIO_SAMPLE_RATE * millis / 1000),
  _sampled(0),
  _ramp(_samples / 10),
  _amplitude((float)volume / 100.0f),
  _increment(PI2 * (float)frequency / AUDIO_SAMPLE_RATE),
  _phase(0) {
  instances++;

  if (frequency != 0) {
    _ramp = 0;
  } else if (_ramp > MAX_RAMP) {
    _ramp = MAX_RAMP;
  }

  // align sample size with burst-size
  if (_ramp != 0 && _samples > blockSize) {
    _samples = (_samples / blockSize) * blockSize;
  }
}

Sound::~Sound() {
  instances--;
}

//
// Returns whether the sound is ready or completed
//
bool Sound::ready() const {
  return _sampled < _samples;
}

//
// Returns the next wave sample value
//
float Sound::sample() {
  float result = sinf(_phase) * _amplitude;
  _sampled++;
  _phase = fmod(_phase + _increment, PI2);

  if (_ramp != 0) {
    if (_sampled < _ramp) {
      // fadeOut the previous sound
      result *= (float)(_ramp - _sampled) / (float)_ramp;
    } else if (_samples - _sampled < _ramp) {
      // fadeIn the next sound
      result *= (float)(_ramp - (_samples - _sampled)) / (float)_ramp;
    } else {
      result = 0;
    }
  }
  return result;
}

//
// Continues the same phase for the previous sound
//
void Sound::sync(Sound *previous) {
  _phase = previous->_phase;
  if (_ramp != 0) {
    // for fadeOut/In
    _increment = previous->_increment;
  }
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
  std::lock_guard<std::mutex> lock(_lock);
  _queue.removeAll();
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
    if (_startNoSound != 0 && maGetMilliSecondCount() - _startNoSound > SILENCE_BEFORE_STOP) {
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
  _queue.push(new Sound(_stream->getFramesPerBurst(), frequency, millis, volume));
  _startNoSound = 0;
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

  if (result == nullptr && _startNoSound == 0) {
    _startNoSound = maGetMilliSecondCount();
  }

  return result;
}
