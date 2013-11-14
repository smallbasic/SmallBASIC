/// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <android_native_app_glue.h>

#include <errno.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define  LOG_TAG    "smallbasic"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

// Return current time in milliseconds
static double now_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000. + tv.tv_usec/1000.;
}

struct engine {
  struct android_app* app;
  int animating;
};


/*
ANativeWindow_Buffer surface_buffer;
if (ANativeWindow_lock(_window, &surface_buffer, NULL) == 0) {
    memcpy(surface_buffer.bits, _buffer,  _bufferSize);
    ANativeWindow_unlockAndPost(_window);
}
*/

static void engine_draw_frame(struct engine* engine) {
  if (engine->app->window != NULL) {
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window, &buffer, NULL) < 0) {
      LOGW("Unable to lock window buffer");
    } else {
      struct timespec t;
      t.tv_sec = t.tv_nsec = 0;
      clock_gettime(CLOCK_MONOTONIC, &t);
      int64_t time_ms = (((int64_t)t.tv_sec)*1000000000LL + t.tv_nsec)/1000000;
      
      /* Now fill the values with a nice little plasma */
      //fill_plasma(&buffer, time_ms);
      ANativeWindow_unlockAndPost(engine->app->window);
    }
  }
}

static void engine_term_display(struct engine *engine) {
  engine->animating = 0;
}

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
  struct engine *engine = (struct engine *)app->userData;
  int32_t result;
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
    engine->animating = 1;
    result = 1;
  } else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
    LOGI("Key event: action=%d keyCode=%d metaState=0x%x",
         AKeyEvent_getAction(event),
         AKeyEvent_getKeyCode(event),
         AKeyEvent_getMetaState(event));
    result = 0;
  }
  return result;
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
  struct engine *engine = (struct engine *)app->userData;
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    if (engine->app->window != NULL) {
      engine_draw_frame(engine);
    }
    break;
  case APP_CMD_TERM_WINDOW:
    engine_term_display(engine);
    break;
  case APP_CMD_LOST_FOCUS:
    engine->animating = 0;
    engine_draw_frame(engine);
    break;
  }
}

void android_main(struct android_app *state) {
  static int init;
  struct engine engine;

  // Make sure glue isn't stripped.
  app_dummy();

  memset(&engine, 0, sizeof(engine));
  state->userData = &engine;
  state->onAppCmd = engine_handle_cmd;
  state->onInputEvent = engine_handle_input;
  engine.app = state;

  // loop waiting for stuff to do.
  while (1) {
    // Read all pending events.
    int ident;
    int events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                  (void**)&source)) >= 0) {
      // Process this event.
      if (source != NULL) {
        source->process(state, source);
      }

      // Check if we are exiting.
      if (state->destroyRequested != 0) {
        LOGI("Engine thread destroy requested!");
        engine_term_display(&engine);
        return;
      }
    }
    if (engine.animating) {
      engine_draw_frame(&engine);
    }
  }
}
