// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#define MINIAUDIO_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION

#include "config.h"
#include <stdio.h>
#include "include/osd.h"
#include "ui/strlib.h"
#include "ui/audio.h"
#include "lib/miniaudio/extras/dr_wav.h"
#include "lib/miniaudio/miniaudio.h"

extern "C" {
  uint32_t dev_get_millisecond_count();
  void err_throw(const char *fmt, ...);
}

#define DEFAULT_FORMAT ma_format_f32
#define DEFAULT_SAMPLE_RATE  44100
#define DEFAULT_CHANNELS 1
#define MILLIS_TO_MICROS(n) (n * 1000)

struct Sound;
static ma_device device;
static ma_device_config config;
static strlib::Queue<Sound *> queue;
static strlib::Properties<Sound *> cache;

struct Sound {
  Sound(int frequency, int millis, int volume);
  Sound();
  virtual ~Sound();

  ma_result construct(const char *path);

  ma_sine_wave *_tone;
  ma_decoder *_decoder;
  uint32_t _start;
  uint32_t _duration;
};

Sound::Sound() :
  _tone(nullptr),
  _decoder(nullptr),
  _start(0),
  _duration(0) {
}

Sound::Sound(int frequency, int millis, int volume) :
  _tone(nullptr),
  _decoder(nullptr),
  _start(0),
  _duration(millis) {
  _tone = (ma_sine_wave *)malloc(sizeof(ma_sine_wave));
  ma_sine_wave_init(volume / 100.0, frequency, DEFAULT_SAMPLE_RATE, _tone);
}

Sound::~Sound() {
  if (_decoder) {
    ma_decoder_uninit(_decoder);
    free(_decoder);
    _decoder = nullptr;
  }
  free(_tone);
  _tone = nullptr;
}

ma_result Sound::construct(const char *path) {
  _decoder = (ma_decoder *)malloc(sizeof(ma_decoder));
  return _decoder == nullptr ? MA_OUT_OF_MEMORY : ma_decoder_init_file(path, nullptr, _decoder);
}

static void data_callback(ma_device *device, void *output, const void *input, ma_uint32 frameCount) {
  if (queue.empty()) {
    usleep(MILLIS_TO_MICROS(10));
  } else {
    Sound *sound = queue.front();
    if (sound->_decoder != nullptr) {
      // play audio track
      ma_decoder_read_pcm_frames(sound->_decoder, output, frameCount);
    } else if (sound->_start == 0) {
      // start new sound
      sound->_start = dev_get_millisecond_count();
      ma_sine_wave_read_f32(sound->_tone, frameCount, (float*)output);
    } else if (dev_get_millisecond_count() - sound->_start > sound->_duration) {
      // sound has timed out
      queue.pop(sound->_decoder == nullptr);
    } else {
      // continue sound
      ma_sine_wave_read_f32(sound->_tone, frameCount, (float*)output);
    }
  }
}

static void setup_config(ma_format format, ma_uint32 channels, ma_uint32 sampleRate) {
  config = ma_device_config_init(ma_device_type_playback);
  config.playback.format = format;
  config.playback.channels = channels;
  config.sampleRate = sampleRate;
  config.dataCallback = data_callback;
  config.pUserData = nullptr;
}

static void setup_format(ma_format format, ma_uint32 channels, ma_uint32 sampleRate) {
  if (config.playback.format != format ||
      config.playback.channels != channels ||
      config.sampleRate != sampleRate) {
    audio_close();
    setup_config(format, channels, sampleRate);
    ma_result result = ma_device_init(nullptr, &config, &device);
    if (result != MA_SUCCESS) {
      err_throw("Failed to prepare sound device [%d]", result);
    }
  }
}

bool audio_open() {
  setup_config(DEFAULT_FORMAT, DEFAULT_CHANNELS, DEFAULT_SAMPLE_RATE);
  return (ma_device_init(nullptr, &config, &device) == MA_SUCCESS);
}

void audio_close() {
  osd_clear_sound_queue();
  ma_device_uninit(&device);
}

void osd_audio(const char *path) {
  Sound *sound = cache.get(path);
  if (sound) {
    ma_mutex_lock(&device.lock);
    queue.push(sound);
    ma_mutex_unlock(&device.lock);
  } else {
    sound = new Sound();
    ma_result result = sound->construct(path);
    if (result != MA_SUCCESS) {
      delete sound;
      err_throw("Failed to open sound file [%d]", result);
    } else {
      setup_format(sound->_decoder->outputFormat,
                   sound->_decoder->outputChannels,
                   sound->_decoder->outputSampleRate);
      ma_mutex_lock(&device.lock);
      queue.push(sound);
      cache.put(path, sound);
      ma_mutex_unlock(&device.lock);
      if (queue.size() == 1) {
        result = ma_device_start(&device);
        if (result != MA_SUCCESS) {
          err_throw("Failed to start audio [%d]", result);
        }
      }
    }
  }
}

void osd_beep() {
  osd_sound(1000, 30, 100, 0);
  osd_sound(500, 30, 100, 0);
}

void osd_clear_sound_queue() {
  ma_device_stop(&device);

  // remove any cached sounds from the queue
  List_each(Sound *, it, queue) {
    Sound *next = *it;
    if (next != NULL && next->_decoder != nullptr) {
      queue.remove(it);
      it--;
    }
  }

  queue.removeAll();
  cache.removeAll();
}

void osd_sound(int frequency, int millis, int volume, int background) {
  setup_format(DEFAULT_FORMAT, DEFAULT_CHANNELS, DEFAULT_SAMPLE_RATE);
  ma_mutex_lock(&device.lock);
  queue.push(new Sound(frequency, millis, volume));
  ma_mutex_unlock(&device.lock);

  if (!background) {
    ma_device_start(&device);
    usleep(MILLIS_TO_MICROS(millis));
    ma_device_stop(&device);
    ma_mutex_lock(&device.lock);
    queue.pop();
    ma_mutex_unlock(&device.lock);
  } else  if (queue.size() == 1) {
    ma_device_start(&device);
  }
}
