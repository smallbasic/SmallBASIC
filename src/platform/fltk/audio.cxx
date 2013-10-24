// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "common/osd.h"

#if defined(HAVE_SDL)
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <queue>
#include <cmath>

struct SoundObject {
  double freq;
  int samplesLeft;
};

bool initDone = false;
double v;
std::queue<SoundObject> sounds;

#define AMPLITUDE 28000
#define FREQUENCY 44100

void audio_callback(void *_beeper, Uint8 *_stream, int _length) {
  Sint16 *stream = (Sint16 *)_stream;
  int length = _length / 2;
  int i = 0;
  while (i < length) {
    if (sounds.empty()) {
      while (i < length) {
        stream[i] = 0;
        i++;
      }
      return;
    }

    SoundObject &sound = sounds.front();
    int samplesToDo = std::min(i + sound.samplesLeft, length);
    sound.samplesLeft -= samplesToDo - i;

    while (i < samplesToDo) {
      stream[i] = AMPLITUDE * std::sin(v * 2 * M_PI / FREQUENCY);
      i++;
      v += sound.freq;
    }

    if (sound.samplesLeft == 0) {
      sounds.pop();
    }
  }
}

void open_audio() {
  if (!initDone) {
    initDone = true;
    SDL_Init(SDL_INIT_AUDIO);
  }

  SDL_AudioSpec desiredSpec;
  desiredSpec.freq = FREQUENCY;
  desiredSpec.format = AUDIO_S16SYS;
  desiredSpec.channels = 1;
  desiredSpec.samples = 2048;
  desiredSpec.callback = audio_callback;
  
  SDL_AudioSpec obtainedSpec;
  SDL_OpenAudio(&desiredSpec, &obtainedSpec);
}

void close_audio() {
  SDL_CloseAudio();
}

void do_beep(double freq, int duration) {
  SoundObject sound;
  sound.freq = freq;
  sound.samplesLeft = duration * FREQUENCY / 1000;

  SDL_LockAudio();
  sounds.push(sound);
  SDL_UnlockAudio();
}

void flush_queue() {
  int size;
  do {
    SDL_Delay(20);
    SDL_LockAudio();
    size = sounds.size();
    SDL_UnlockAudio();
  } while (size > 0);
}

void osd_beep() {
  SDL_PauseAudio(0);
  v = 0;
  do_beep(1000, 30);
  do_beep(500, 30);
  flush_queue();
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
  SDL_PauseAudio(0);
  v = 0;
  do_beep(frq, ms);
  if (!bgplay) {
    flush_queue();
  }
}

#else

#include <fltk/ask.h>
#if defined(WIN32)
  #include <windows.h>
#endif

// non SDL based implementation
void osd_beep() {
  fltk::beep(fltk::BEEP_MESSAGE);
}

void osd_sound(int frq, int ms, int vol, int bgplay) {
#if defined(WIN32)
  if (!bgplay) {
    ::Beep(frq, ms);
  }
#endif
}

void open_audio() {
}

void close_audio() {
}

#endif

void osd_clear_sound_queue() {
}
