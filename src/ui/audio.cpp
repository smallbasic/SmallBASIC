// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#define MINIAUDIO_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
//#define DR_MP3_IMPLEMENTATION

#include "config.h"
#include <stdio.h>
#include "common/osd.h"
#include "common/device.h"
#include "ui/strlib.h"
#include "lib/miniaudio/miniaudio.h"
#include "lib/miniaudio/extras/dr_wav.h"
//#include "lib/miniaudio/extras/dr_mp3.h"

#define DEVICE_SAMPLE_RATE  48000
#define DEVICE_CHANNELS 1
#define MILLIS_TO_MICROS(n) (n * 1000)

struct Sound {
  Sound(int frequency, int millis, int volume);
  ~Sound();
  
  ma_sine_wave *_tone;
  uint32_t _start;
  uint32_t _duration;
};

Sound::Sound(int frequency, int millis, int volume) :
  _tone(nullptr),
  _start(0),
  _duration(millis) {
  _tone = (ma_sine_wave *)malloc(sizeof(ma_sine_wave));
  ma_sine_wave_init(volume / 100.0, frequency, DEVICE_SAMPLE_RATE, _tone);
}

Sound::~Sound() {
  free(_tone);
  _tone = nullptr;
}

strlib::Queue<Sound *> queue;
ma_decoder decoder;
ma_device device;
ma_device_config config;

void data_callback(ma_device *device, void *output, const void *input, ma_uint32 frameCount) {
  if (device->playback.channels == DEVICE_CHANNELS) {
    if (queue.empty()) {
      usleep(MILLIS_TO_MICROS(10));
    } else {
      Sound *sound = queue.front();
      if (sound->_start == 0) {
        // start new sound
        sound->_start = dev_get_millisecond_count();
        ma_sine_wave_read_f32(sound->_tone, frameCount, (float*)output);
      } else if (dev_get_millisecond_count() - sound->_start > sound->_duration) {
        // sound has timed out
        queue.pop();
      } else {
        // continue sound
        ma_sine_wave_read_f32(sound->_tone, frameCount, (float*)output);
      }
    }
  }
}

bool open_audio() {
  config = ma_device_config_init(ma_device_type_playback);
  config.playback.format   = ma_format_f32;
  config.playback.channels = DEVICE_CHANNELS;
  config.sampleRate        = DEVICE_SAMPLE_RATE;
  config.dataCallback      = data_callback;
  config.pUserData         = nullptr;
  return (ma_device_init(nullptr, &config, &device) == MA_SUCCESS);
}

void close_audio() {
  ma_device_uninit(&device);
}

void osd_audio(const char *path) {
  //if (ma_decoder_init_file_wav(path, nullptr, &decoder) == MA_SUCCESS) {
  //    ma_device_start(&device);
  //  } else {
  //    fprintf(stderr, "failed\n");
  //  }
}

void osd_clear_sound_queue() {
  ma_device_stop(&device);
  queue.removeAll();
}

void osd_beep() {
  osd_sound(1000, 30, 100, 0);
  osd_sound(500, 30, 100, 0);
}

void osd_sound(int frequency, int millis, int volume, int background) {
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
