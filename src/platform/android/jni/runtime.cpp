// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <android/native_window.h>
#include <android/keycodes.h>
#include <jni.h>

#include "platform/android/jni/runtime.h"
#include "platform/common/maapi.h"
#include "platform/common/utils.h"
#include "platform/common/form_ui.h"
#include "common/sbapp.h"
#include "common/sys.h"
#include "common/smbas.h"
#include "common/osd.h"
#include "common/device.h"
#include "common/blib_ui.h"
#include "common/fs_socket_client.h"

#define WAIT_INTERVAL 10
#define DEFAULT_FONT_SIZE 30
#define MAIN_BAS "__main_bas__"
#define CONFIG_FILE "/settings.txt"
#define PATH_KEY "path"
#define FONT_SCALE_KEY "fontScale"
#define SERVER_SOCKET_KEY "serverSocket"
#define SERVER_TOKEN_KEY "serverToken"

Runtime *runtime;

MAEvent *getMotionEvent(int type, AInputEvent *event) {
  MAEvent *result = new MAEvent();
  result->type = type;
  result->point.x = AMotionEvent_getX(event, 0);
  result->point.y = AMotionEvent_getY(event, 0);
  return result;
}

int32_t handleInput(android_app *app, AInputEvent *event) {
  int32_t result = 0;
  if (runtime->isActive()) {
    MAEvent *maEvent = NULL;
    switch (AInputEvent_getType(event)) {
    case AINPUT_EVENT_TYPE_MOTION:
      switch (AKeyEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) {
      case AMOTION_EVENT_ACTION_DOWN:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_PRESSED, event);
        break;
      case AMOTION_EVENT_ACTION_MOVE:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_DRAGGED, event);
        break;
      case AMOTION_EVENT_ACTION_UP:
        maEvent = getMotionEvent(EVENT_TYPE_POINTER_RELEASED, event);
        break;
      }
      break;
    case AINPUT_EVENT_TYPE_KEY:
      if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
        maEvent = new MAEvent();
        maEvent->type = EVENT_TYPE_KEY_PRESSED;
        maEvent->nativeKey = AKeyEvent_getKeyCode(event);
        maEvent->key = AKeyEvent_getMetaState(event);
      }
      break;
    }
    if (maEvent != NULL) {
      result = 1;
      runtime->pushEvent(maEvent);
    }
  }
  return result;
}

void handleCommand(android_app *app, int32_t cmd) {
  trace("handleCommand = %d", cmd);
  switch (cmd) {
  case APP_CMD_INIT_WINDOW:
    // thread is ready to start
    runtime->construct();
    break;
  case APP_CMD_TERM_WINDOW:
    if (runtime) {
      // thread is ending
      runtime->setExit(true);
    }
    break;
  case APP_CMD_LOST_FOCUS:
    break;
  }
}

// see http://stackoverflow.com/questions/15913080
static void process_input(android_app *app, android_poll_source *source) {
  AInputEvent* event = NULL;
  while (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY &&
        AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
      // prevent AInputQueue_preDispatchEvent from attempting to close
      // the keypad here to avoid a crash in android 4.2 + 4.3.
      if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN &&
          runtime->isActive()) {
        MAEvent *maEvent = new MAEvent();
        maEvent->nativeKey = AKEYCODE_BACK;
        maEvent->type = EVENT_TYPE_KEY_PRESSED;
        runtime->pushEvent(maEvent);
      }
      AInputQueue_finishEvent(app->inputQueue, event, true);
    } else if (!AInputQueue_preDispatchEvent(app->inputQueue, event)) {
      AInputQueue_finishEvent(app->inputQueue, event, handleInput(app, event));
    }
  }
}

// callback from MainActivity.java
extern "C" JNIEXPORT jboolean JNICALL Java_net_sourceforge_smallbasic_MainActivity_optionSelected
  (JNIEnv *env, jclass jclazz, jint index) {
  MAEvent *maEvent = new MAEvent();
  maEvent->type = EVENT_TYPE_OPTIONS_BOX_BUTTON_CLICKED;
  maEvent->optionsBoxButtonIndex = index;
  runtime->pushEvent(maEvent);
  return true;
}

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_runFile
  (JNIEnv *env, jclass jclazz, jstring path) {
  const char *fileName = env->GetStringUTFChars(path, JNI_FALSE);
  runtime->runPath(fileName);
  env->ReleaseStringUTFChars(path, fileName);
}

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_onResize
  (JNIEnv *env, jclass jclazz, jint width, jint height) {
  runtime->onResize(width, height);
}

void onContentRectChanged(ANativeActivity *activity, const ARect *rect) {
  logEntered();
  runtime->onResize(rect->right, rect->bottom);
}

Runtime::Runtime(android_app *app) :
  System(),
  _keypadActive(false),
  _app(app) {
  _app->userData = NULL;
  _app->onAppCmd = handleCommand;
  _app->onInputEvent = handleInput;
  _app->inputPollSource.process = process_input;
  runtime = this;
  pthread_mutex_init(&_mutex, NULL);
  _looper = ALooper_forThread();
}

Runtime::~Runtime() {
  logEntered();
  delete _output;
  delete _eventQueue;
  delete _graphics;
  runtime = NULL;
  _output = NULL;
  _eventQueue = NULL;
  _graphics = NULL;
  pthread_mutex_destroy(&_mutex);
}

bool Runtime::getUntrusted() {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "getUntrusted", "()Z");
  jboolean result = (jboolean) env->CallBooleanMethod(_app->activity->clazz, methodId);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

String Runtime::getStartupBas() {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "getStartupBas", "()Ljava/lang/String;");
  jstring startupBasObj = (jstring) env->CallObjectMethod(_app->activity->clazz, methodId);
  const char *startupBas = env->GetStringUTFChars(startupBasObj, JNI_FALSE);
  String result = startupBas;
  env->ReleaseStringUTFChars(startupBasObj, startupBas);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

int Runtime::getUnicodeChar(int keyCode, int metaState) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "getUnicodeChar", "(II)I");
  jint result = env->CallIntMethod(_app->activity->clazz, methodId, keyCode, metaState);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

void Runtime::construct() {
  logEntered();
  _state = kClosingState;
  _graphics = new Graphics(_app);
  if (_graphics && _graphics->construct()) {
    int w = ANativeWindow_getWidth(_app->window);
    int h = ANativeWindow_getHeight(_app->window);
    _output = new AnsiWidget(this, w, h);
    if (_output && _output->construct()) {
      _eventQueue = new Stack<MAEvent *>();
      if (_eventQueue) {
        _state = kActiveState;
      }
    }
  }
}

char *Runtime::loadResource(const char *fileName) {
  char *buffer = System::loadResource(fileName);
  if (buffer == NULL && strcmp(fileName, MAIN_BAS) == 0) {
    AAssetManager *assetManager = _app->activity->assetManager;
    AAsset *mainBasFile = AAssetManager_open(assetManager, "main.bas", AASSET_MODE_BUFFER);
    off_t len = AAsset_getLength(mainBasFile);
    buffer = (char *)tmp_alloc(len + 1);
    if (AAsset_read(mainBasFile, buffer, len) < 0) {
      trace("failed to read main.bas");
    }
    buffer[len] = '\0';
    trace("loaded main.bas [%ld] bytes", len);
    AAsset_close(mainBasFile);
  }
  return buffer;
}

void Runtime::pushEvent(MAEvent *event) {
  pthread_mutex_lock(&_mutex);
  _eventQueue->push(event);
  pthread_mutex_unlock(&_mutex);
}

MAEvent *Runtime::popEvent() {
  pthread_mutex_lock(&_mutex);
  MAEvent *result = _eventQueue->pop();
  pthread_mutex_unlock(&_mutex);
  return result;
}

void Runtime::runShell() {
  logEntered();

  opt_ide = IDE_NONE;
  opt_graphics = true;
  opt_pref_bpp = 0;
  opt_nosave = true;
  opt_interactive = true;
  opt_verbose = false;
  opt_quiet = true;
  opt_command[0] = 0;
  opt_usevmt = 0;
  os_graphics = 1;
  opt_file_permitted = 1;

  _app->activity->callbacks->onContentRectChanged = onContentRectChanged;
  loadConfig();

  String startupBas = getStartupBas();
  if (startupBas.length()) {
    if (getUntrusted()) {
      opt_file_permitted = 0;
    }
    runOnce(startupBas.c_str());
  } else {
    runMain(MAIN_BAS);
  }
  saveConfig();

  _state = kDoneState;
  logLeaving();
}

void Runtime::loadConfig() {
  String buffer;
  String path;
  Properties profile;

  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(DEFAULT_FONT_SIZE);
  _initialFontSize = _output->getFontSize();

  path.append(_app->activity->internalDataPath);
  path.append(CONFIG_FILE);
  FILE *fp = fopen(path.c_str(), "r");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    rewind(fp);
    buffer.append(fp, len);
    fclose(fp);
    profile.load(buffer.toString(), buffer.length());
    String *s = profile.get(FONT_SCALE_KEY);
    if (s) {
      _fontScale = s->toInteger();
      trace("_fontScale = %d", _fontScale);
      if (_fontScale != 100) {
        int fontSize = (_initialFontSize * _fontScale / 100);
        _output->setFontSize(fontSize);
      }
    }
    s = profile.get(PATH_KEY);
    if (s) {
      trace("path = %s", s->c_str());
      chdir(s->c_str());
    }
    loadEnvConfig(profile, SERVER_SOCKET_KEY);
    loadEnvConfig(profile, SERVER_TOKEN_KEY);
  }
}

void Runtime::loadEnvConfig(Properties &profile, const char *key) {
  String *s = profile.get(key);
  if (s) {
    trace("%s = %s", key, s->c_str());
    String env = key;
    env += "=";
    env += s->c_str();
    dev_putenv(env.c_str());
  }
}

void Runtime::saveConfig() {
  String path;
  path.append(_app->activity->internalDataPath);
  path.append(CONFIG_FILE);
  FILE *fp = fopen(path.c_str(), "w");
  if (fp) {
    char path[FILENAME_MAX + 1];
    getcwd(path, FILENAME_MAX);
    fprintf(fp, "%s='%s'\n", PATH_KEY, path);
    fprintf(fp, "%s=%d\n", FONT_SCALE_KEY, _fontScale);
    for (int i = 0; environ[i] != NULL; i++) {
      char *env = environ[i];
      if (strstr(env, SERVER_SOCKET_KEY) != NULL) {
        fprintf(fp, "%s\n", env);
      } else if (strstr(env, SERVER_TOKEN_KEY) != NULL) {
        fprintf(fp, "%s\n", env);
      }
    }
    fclose(fp);
  }
}

void Runtime::runPath(const char *path) {
  pthread_mutex_lock(&_mutex);
  buttonClicked(path);
  setExit(false);
  ALooper_wake(_looper);
  pthread_mutex_unlock(&_mutex);
}

void Runtime::handleKeyEvent(MAEvent &event) {
  trace("key = %d %d", event.nativeKey, event.key);
  switch (event.nativeKey) {
  case AKEYCODE_BACK:
    if (_keypadActive) {
      showKeypad(false);
    } else {
      setBack();
    }
    break;
  case AKEYCODE_MENU:
    showMenu();
    break;
  case AKEYCODE_TAB:
    event.key = SB_KEY_TAB;
    break;
  case AKEYCODE_HOME:
    event.key = SB_KEY_KP_HOME;
    break;
    // These are not available in android-9
    //  case AKEYCODE_MOVE_END:
    //    event.key = SB_KEY_END;
    //    break;
    //  case AKEYCODE_INSERT:
    //    event.key = SB_KEY_INSERT;
    //    break;
    //  case AKEYCODE_NUMPAD_MULTIPLY:
    //    event.key = SB_KEY_KP_MUL;
    //    break;
    //  case AKEYCODE_NUMPAD_ADD:
    //    event.key = SB_KEY_KP_PLUS;
    //    break;
    //  case AKEYCODE_NUMPAD_SUBTRACT:
    //    event.key = SB_KEY_KP_MINUS;
    //    break;
  case AKEYCODE_SLASH:
    event.key = SB_KEY_KP_DIV;
    break;
  case AKEYCODE_PAGE_UP:
    event.key = SB_KEY_PGUP;
    break;
  case AKEYCODE_PAGE_DOWN:
    event.key = SB_KEY_PGDN;
    break;
  case AKEYCODE_DPAD_UP:
    event.key = SB_KEY_UP;
    break;
  case AKEYCODE_DPAD_DOWN:
    event.key = SB_KEY_DN;
    break;
  case AKEYCODE_DPAD_LEFT:
    event.key = SB_KEY_LEFT;
    break;
  case AKEYCODE_DPAD_RIGHT:
    event.key = SB_KEY_RIGHT;
    break;
  case AKEYCODE_CLEAR:
  case AKEYCODE_DEL:
    event.key = MAK_CLEAR;
    break;
  case AKEYCODE_ENTER:
    event.key = SB_KEY_ENTER;
    break;
  default:
    event.key = getUnicodeChar(event.nativeKey, event.key);
    break;
  }
  if (isRunning() && event.key) {
    dev_pushkey(event.key);
  }
}

void Runtime::optionsBox(StringList *items) {
  logEntered();

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);

  jstring elem = env->NewStringUTF(items->get(0)->c_str());
  jclass stringClass = env->GetObjectClass(elem);
  jobjectArray array = env->NewObjectArray(items->size(), stringClass, elem);
  for (int i = 1; i < items->size(); i++) {
    elem = env->NewStringUTF(items->get(i)->c_str());
    env->SetObjectArrayElement(array, i, elem);
  }
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "optionsBox", "([Ljava/lang/String;)V");
  env->CallObjectMethod(_app->activity->clazz, methodId, array);

  for (int i = 0; i < items->size(); i++) {
    env->DeleteLocalRef(env->GetObjectArrayElement(array, i));
  }
  env->DeleteLocalRef(clazz);
  env->DeleteLocalRef(array);
  env->DeleteLocalRef(stringClass);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::playTone(int frq, int dur, int vol, bool bgplay) {
  logEntered();

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "playTone", "(IIIZ)V");
  env->CallVoidMethod(_app->activity->clazz, methodId, frq, dur, vol, bgplay);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::pollEvents(bool blocking) {
  int events;
  android_poll_source *source;
  ALooper_pollAll(blocking ? -1 : 0, NULL, &events, (void **)&source);
  if (source != NULL) {
    source->process(_app, source);
  }
  if (_app->destroyRequested != 0) {
    trace("Thread destroy requested");
    setExit(true);
  }
}

MAEvent Runtime::processEvents(int waitFlag) {
  switch (waitFlag) {
  case 1:
    // wait for an event
    _output->flush(true);
    pollEvents(true);
    break;
  case 2:
    // pause
    maWait(WAIT_INTERVAL);
    break;
  default:
    pollEvents(false);
    checkLoadError();
  }

  MAEvent event;
  if (hasEvent()) {
    MAEvent *nextEvent = popEvent();
    event = *nextEvent;
    delete nextEvent;
  } else {
    event.type = 0;
  }

  switch (event.type) {
  case EVENT_TYPE_SCREEN_CHANGED:
    _graphics->resize();
    resize();
    break;
  case EVENT_TYPE_KEY_PRESSED:
    handleKeyEvent(event);
    break;
  default:
    handleEvent(event);
    break;
  }
  return event;
}

void Runtime::setExit(bool quit) {
  if (!isClosing()) {
    _state = quit ? kClosingState : kBackState;
    if (isRunning()) {
      brun_break();
    }
  }
}

void Runtime::showKeypad(bool show) {
  logEntered();
  _keypadActive = show;

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "showKeypad", "(Z)V");
  env->CallObjectMethod(_app->activity->clazz, methodId, show);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::showAlert(const char *title, const char *message) {
  logEntered();

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, NULL);
  jstring titleString = env->NewStringUTF(title);
  jstring messageString = env->NewStringUTF(message);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID method = env->GetMethodID(clazz, "showAlert",
                                      "(Ljava/lang/String;Ljava/lang/String;)V");
  env->CallObjectMethod(_app->activity->clazz, method, titleString, messageString);

  env->DeleteLocalRef(clazz);
  env->DeleteLocalRef(messageString);
  env->DeleteLocalRef(titleString);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::onResize(int width, int height) {
  logEntered();
  int w = ANativeWindow_getWidth(_app->window);
  int h = ANativeWindow_getHeight(_app->window);
  if (w != width || h != height) {
    trace("Resized from %d %d to %d %d", w, h, width, height);
    ANativeWindow_setBuffersGeometry(_app->window, width, height, WINDOW_FORMAT_RGB_565);
    ALooper_acquire(_app->looper);
    MAEvent *maEvent = new MAEvent();
    maEvent->type = EVENT_TYPE_SCREEN_CHANGED;
    runtime->pushEvent(maEvent);
    ALooper_wake(_app->looper);
    ALooper_release(_app->looper);
  }
}

//
// form_ui implementation
//
AnsiWidget *form_ui::getOutput() {
  return runtime->_output;
}

void form_ui::optionsBox(StringList *items) {
  runtime->optionsBox(items);
}

//
// ma event handling
//
int maGetEvent(MAEvent *event) {
  int result;
  if (runtime->hasEvent()) {
    MAEvent *nextEvent = runtime->popEvent();
    event->point = nextEvent->point;
    event->type = nextEvent->type;
    delete nextEvent;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

void maWait(int timeout) {
  if (timeout == -1) {
    runtime->pollEvents(true);
  } else {
    int slept = 0;
    while (1) {
      runtime->pollEvents(false);
      if (runtime->hasEvent()
          || runtime->isBack()
          || runtime->isClosing()) {
        break;
      }
      usleep(WAIT_INTERVAL * 1000);
      slept += WAIT_INTERVAL;
      if (timeout > 0 && slept > timeout) {
        break;
      }
    }
  }
}

int maGetMilliSecondCount(void) {
  timespec t;
  t.tv_sec = t.tv_nsec = 0;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (int) (1000L * t.tv_sec + ((double) t.tv_nsec / 1e6));
}

int maShowVirtualKeyboard(void) {
  osd_beep();
  runtime->showKeypad(true);
  return 0;
}

void maAlert(const char *title, const char *message, const char *button1,
             const char *button2, const char *button3) {
  runtime->showAlert(title, message);
}

//
// sbasic implementation
//
int osd_devinit(void) {
  setsysvar_str(SYSVAR_OSNAME, "Android");
  runtime->setRunning(true);
  return 1;
}

void osd_sound(int frq, int dur, int vol, int bgplay) {
  runtime->playTone(frq, dur, vol, bgplay);
}

void osd_clear_sound_queue() {
}

void osd_beep(void) {
  osd_sound(1000, 30, 100, 1);
  osd_sound(500, 30, 100, 0);
}

void dev_image(int handle, int index,
               int x, int y, int sx, int sy, int w, int h) {
}

int dev_image_width(int handle, int index) {
  return 0;
}

int dev_image_height(int handle, int index) {
  return 0;
}
