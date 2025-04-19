// This file is part of SmallBASIC
//
// Copyright(C) 2001-2024 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <unistd.h>

#include "config.h"
#include "ui/strlib.h"
#include "ui/utils.h"
#include "lib/maapi.h"
#include "audio.h"

//see: https://github.com/google/oboe/blob/main/docs/GettingStarted.md
constexpr int32_t AUDIO_SAMPLE_RATE = 48000;
constexpr float PI2 = 2.0f * M_PI;
constexpr int MAX_QUEUE_SIZE = 250;
constexpr int MAX_RAMP = 512;
constexpr int RAMP_SCALE = 20;
constexpr int BUFFER_DURATION = 40;
constexpr int FRAMES_PER_CALLBACK = AUDIO_SAMPLE_RATE * BUFFER_DURATION / 1000;
constexpr int SILENCE_BEFORE_STOP = AUDIO_SAMPLE_RATE * 60 / FRAMES_PER_CALLBACK;

int instances = 0;
float phase = 0;

struct Sound {
  Sound(int frequency, int millis, int volume);
  ~Sound();

  bool ready() const;
  float sample();
  void sync(Sound *previous);

private:
  uint32_t _duration;
  uint32_t _samples;
  uint32_t _sampled;
  uint32_t _fadedIn;
  uint32_t _fadeOut;
  float _amplitude;
  float _increment;
};

Sound::Sound(int frequency, int millis, int volume) :
  _duration(millis),
  _samples(AUDIO_SAMPLE_RATE * millis / 1000),
  _sampled(0),
  _fadedIn(0),
  _fadeOut(0),
  _amplitude((float)volume / 100.0f),
  _increment(PI2 * (float)frequency / AUDIO_SAMPLE_RATE) {
  _fadedIn = _fadeOut = std::min((int)_samples / RAMP_SCALE, MAX_RAMP);
  instances++;
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
  float result = sinf(phase) * _amplitude;
  _sampled++;
  phase = fmod(phase + _increment, PI2);

  if (_fadedIn != 0 && _sampled < _fadedIn) {
    // fadeIn from silence
    result *= (float)(_sampled) / (float)_fadedIn;
  } else if (_fadeOut != 0 && (_samples - _sampled) < _fadeOut) {
    // fadeOut to silence
    result *= (float)(_samples - _sampled) / (float)_fadeOut;
  }
  return result;
}

//
// Skips fadeIn when continuing the same sound
//
void Sound::sync(Sound *previous) {
  if (previous->_increment == _increment) {
    _fadedIn = 0;
    previous->_fadeOut = 0;
  }
}

Audio::Audio(): _silentTicks(0) {
  AudioStreamBuilder builder;
  Result result = builder.setDirection(Direction::Output)
    ->setChannelCount(ChannelCount::Mono)
    ->setDataCallback(this)
    ->setFormat(AudioFormat::Float)
    ->setPerformanceMode(PerformanceMode::LowLatency)
    ->setSampleRate(AUDIO_SAMPLE_RATE)
    ->setSampleRateConversionQuality(SampleRateConversionQuality::None)
    ->setSharingMode(SharingMode::Exclusive)
    ->setUsage(oboe::Usage::Game)
    ->setFramesPerCallback(FRAMES_PER_CALLBACK)
    ->openStream(_stream);
  if (result != oboe::Result::OK) {
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
    if (_stream->getState() != StreamState::Started) {
      trace("Start audio");
      // play silence to initialise the player
      add(0, 250, 1);
      _stream->requestStart();
    }
    add(frequency, millis, volume);
    if (!background || _queue.size() >= MAX_QUEUE_SIZE) {
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
  Sound *sound = front();
  auto *buffer = (float *)audioData;
  for (int i = 0; i < numFrames; ++i) {
    if (sound == nullptr) {
      buffer[i] = 0;
    } else {
      buffer[i] = sound->sample();
      if (!sound->ready()) {
        sound = front();
      }
    }
  }

  DataCallbackResult result;
  if (sound == nullptr && ++_silentTicks > SILENCE_BEFORE_STOP) {
    phase = 0;
    result = DataCallbackResult::Stop;
  } else {
    result = DataCallbackResult::Continue;
  }
  return result;
}

//
// Adds a sound to the queue
//
void Audio::add(int frequency, int millis, int volume) {
  std::lock_guard<std::mutex> lock(_lock);
  auto previous = _queue.back();
  auto sound = new Sound(frequency, millis, volume);
  _queue.push(sound);
  _silentTicks = 0;
  if (previous != nullptr) {
    sound->sync(previous);
  }
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
    result = nullptr;
  }

  return result;
}
