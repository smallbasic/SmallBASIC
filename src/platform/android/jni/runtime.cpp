// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <jni.h>

#include "platform/android/jni/runtime.h"
#include "include/osd.h"
#include "common/sbapp.h"

#define WAIT_INTERVAL 10
#define MAIN_BAS "__main_bas__"
#define CONFIG_FILE "/settings.txt"
#define PATH_KEY "path"
#define FONT_SCALE_KEY "fontScale2"
#define FONT_ID_KEY "fontId"
#define SERVER_SOCKET_KEY "serverSocket"
#define SERVER_TOKEN_KEY "serverToken"
#define MUTE_AUDIO_KEY "muteAudio"
#define THEME_KEY "theme"
#define LOAD_MODULES_KEY "loadModules"
#define OPT_IDE_KEY "optIde"
#define GBOARD_KEY_QUESTION 274
#define EVENT_TYPE_EXIT 100

Runtime *runtime = nullptr;

// Pipe file descriptors: g_backPipe[0] is read-end, g_backPipe[1] is write-end
static int g_backPipe[2] = {-1, -1};

// whether native back key handling is active
static bool g_predictiveBack = false;

// the sensorTypes corresponding to _sensors[] positions
constexpr int SENSOR_TYPES[MAX_SENSORS] = {
  ASENSOR_TYPE_ACCELEROMETER,
  ASENSOR_TYPE_MAGNETIC_FIELD,
  ASENSOR_TYPE_GYROSCOPE,
  ASENSOR_TYPE_LIGHT,
  ASENSOR_TYPE_PROXIMITY,
  ASENSOR_TYPE_PRESSURE,
  ASENSOR_TYPE_RELATIVE_HUMIDITY,
  ASENSOR_TYPE_AMBIENT_TEMPERATURE,
};

MAEvent *getMotionEvent(int type, AInputEvent *event) {
  auto *result = new MAEvent();
  result->type = type;
  result->point.x = (int)AMotionEvent_getX(event, 0);
  result->point.y = (int)AMotionEvent_getY(event, 0);
  return result;
}

int32_t handleInput(android_app *app, AInputEvent *event) {
  int32_t result = 0;
  if (runtime->isActive()) {
    MAEvent *maEvent = nullptr;
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
    if (maEvent != nullptr) {
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
    // thread is ready to start or resume
    if (runtime->isInitial()) {
      runtime->construct();
    }
    break;
  case APP_CMD_GAINED_FOCUS:
    trace("gainedFocus");
    runtime->setFocus(true);
    runtime->redraw();
    break;
  case APP_CMD_LOST_FOCUS:
    trace("lostFocus");
    runtime->setFocus(false);
    break;
  }
}

static void pushBackEvent() {
  auto *maEvent = new MAEvent();
  maEvent->nativeKey = AKEYCODE_BACK;
  maEvent->type = EVENT_TYPE_KEY_PRESSED;
  runtime->pushEvent(maEvent);
}

//
// Callback registered with ALooper that is triggered when the pipe receives data.
// This is what wakes the blocked ALooper_pollOnce() and lets us run pushBackEvent().
//
static int pipeCallback(int fd, int events, void *data) {
  // clear the byte that woke the pipe, then return 1 to stay registered
  logEntered();
  char buf[1];
  read(fd, buf, 1);
  pushBackEvent();
  return 1;
}

//
// Set up the pipe and register its read-end (g_backPipe[0]) with the ALooper.
// This allows us to wake the looper from Java code by writing to the pipe.
//
static void setupBackWakePipe(ALooper *looper) {
  if (pipe(g_backPipe) == 0) {
    // Make read-end non-blocking to avoid stalling the loop
    fcntl(g_backPipe[0], F_SETFL, O_NONBLOCK);

    // Register the pipe with the looper so it wakes up when there's input
    ALooper_addFd(looper,
                  g_backPipe[0],       // fd to watch
                  0,                   // arbitrary/unused identifier
                  ALOOPER_EVENT_INPUT, // watch for input readiness
                  pipeCallback,        // callback to run on wake
                  nullptr);            // no additional data
    trace("Back pipe registered with looper");
  } else {
    trace("Failed to create back pipe");
  }
}

// see http://stackoverflow.com/questions/15913080
static void process_input(android_app *app, android_poll_source *source) {
  AInputEvent* event = nullptr;
  while (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY &&
        AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
      // prevent AInputQueue_preDispatchEvent from attempting to close
      // the keypad here to avoid a crash in android 4.2 + 4.3.
      if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN && runtime->isActive() && !g_predictiveBack) {
        pushBackEvent();
      }
      AInputQueue_finishEvent(app->inputQueue, event, true);
    } else if (!AInputQueue_preDispatchEvent(app->inputQueue, event)) {
      AInputQueue_finishEvent(app->inputQueue, event, handleInput(app, event));
    }
  }
}

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_onBack
  (JNIEnv *env, jclass clazz) {
  if (runtime != nullptr) {
    logEntered();
    if (g_backPipe[1] >= 0) {
      // write a placeholder byte to trigger the read and wake ALooper_pollOnce
      char buf = 'x';
      write(g_backPipe[1], &buf, 1);
    }
  }
}

// callbacks from MainActivity.java
extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_onActivityPaused
  (JNIEnv *env, jclass jclazz, jboolean paused) {
  if (runtime != nullptr && !runtime->isClosing() && runtime->isActive() && os_graphics) {
    trace("paused=%d", paused);
    runtime->onPaused(paused);
  }
}

extern "C" JNIEXPORT jboolean JNICALL Java_net_sourceforge_smallbasic_MainActivity_optionSelected
  (JNIEnv *env, jclass jclazz, jint index) {
  auto *maEvent = new MAEvent();
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

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_setenv
  (JNIEnv *env, jclass jclazz, jstring nameString, jstring valueString) {
  const char *name = env->GetStringUTFChars(nameString, JNI_FALSE);
  const char *value = env->GetStringUTFChars(valueString, JNI_FALSE);
  setenv(name, value, 1);
  env->ReleaseStringUTFChars(nameString, name);
  env->ReleaseStringUTFChars(valueString, value);
}

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_onResize
  (JNIEnv *env, jclass jclazz, jint width, jint height, jint imeState) {
  if (runtime != nullptr && !runtime->isClosing() && runtime->isActive() && os_graphics) {
    runtime->onResize(width, height, imeState);
  }
}

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_onUnicodeChar
  (JNIEnv *env, jclass jclazz, jint ch) {
  if (runtime != nullptr && !runtime->isClosing() && runtime->isActive() && os_graphics) {
    runtime->onUnicodeChar(ch);
  }
}

extern "C" JNIEXPORT jboolean JNICALL Java_net_sourceforge_smallbasic_MainActivity_libraryMode
  (JNIEnv *env, jclass jclazz) {
#if defined(_ANDROID_LIBRARY)
  return true;
#else
  return false;
#endif
}

extern "C" JNIEXPORT jlong JNICALL Java_net_sourceforge_smallbasic_MainActivity_getActivity
  (JNIEnv *env, jobject instance) {
  return runtime->getActivity();
}

extern "C" JNIEXPORT void JNICALL Java_net_sourceforge_smallbasic_MainActivity_consoleLog
  (JNIEnv *env, jclass clazz, jstring jstr) {
  if (jstr != nullptr) {
    const char *str = env->GetStringUTFChars(jstr, nullptr);
    runtime->systemLog(str);
    runtime->systemLog("\n");
    env->ReleaseStringUTFChars(jstr, str);
  }
}

void onContentRectChanged(ANativeActivity *activity, const ARect *rect) {
  logEntered();
  runtime->onResize(rect->right, rect->bottom, 0);
}

jbyteArray newByteArray(JNIEnv *env, const char *str) {
  int size = (int)strlen(str);
  jbyteArray result = env->NewByteArray(size);
  env->SetByteArrayRegion(result, 0, size, (const jbyte *)str);
  return result;
}

Runtime::Runtime(android_app *app) :
  System(),
  _keypadActive(false),
  _hasFocus(false),
  _graphics(nullptr),
  _app(app),
  _eventQueue(nullptr),
  _sensorEventQueue(nullptr),
  _keypad(nullptr) {
  _app->userData = nullptr;
  _app->onAppCmd = handleCommand;
  _app->onInputEvent = handleInput;
  _app->inputPollSource.process = process_input;
  if (runtime != nullptr) {
    trace("another instance is still active");
    _state = kClosingState;
  }
  runtime = this;
  pthread_mutex_init(&_mutex, nullptr);
  _looper = ALooper_forThread();
  _sensorManager = ASensorManager_getInstance();
  memset(&_sensors, 0, sizeof(_sensors));
  g_predictiveBack = getBoolean("isPredictiveBack");
  if (g_predictiveBack) {
    setupBackWakePipe(_looper);
  }
}

Runtime::~Runtime() {
  logEntered();
  delete _output;
  delete _eventQueue;
  delete _graphics;
  delete _keypad;
  runtime = nullptr;
  _output = nullptr;
  _eventQueue = nullptr;
  _graphics = nullptr;
  _keypad = nullptr;
  pthread_mutex_destroy(&_mutex);
  disableSensor();
}

void Runtime::alert(const char *title, const char *message) {
  logEntered();

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jbyteArray titleByteArray = newByteArray(env, title);
  jbyteArray messageByteArray = newByteArray(env, message);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "showAlert", "([B[B)V");
  env->CallVoidMethod(_app->activity->clazz, methodId, titleByteArray, messageByteArray);
  env->DeleteLocalRef(clazz);
  env->DeleteLocalRef(messageByteArray);
  env->DeleteLocalRef(titleByteArray);
  _app->activity->vm->DetachCurrentThread();
}

int Runtime::ask(const char *title, const char *prompt, bool cancel) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jbyteArray titleByteArray = newByteArray(env, title);
  jbyteArray promptByteArray = newByteArray(env, prompt);
  jmethodID methodId = env->GetMethodID(clazz, "ask", "([B[BZ)I");
  jint result = (jint) env->CallIntMethod(_app->activity->clazz, methodId,
                                          titleByteArray, promptByteArray, cancel);
  env->DeleteLocalRef(clazz);
  env->DeleteLocalRef(titleByteArray);
  env->DeleteLocalRef(promptByteArray);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

void Runtime::clearSoundQueue() {
  _audio.clearSoundQueue();
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "clearSoundQueue", "()V");
  env->CallVoidMethod(_app->activity->clazz, methodId);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::construct() {
  logEntered();
  _state = kClosingState;
  _graphics = new Graphics(_app);
  if (_graphics && _graphics->construct(getFontId())) {
    int w = ANativeWindow_getWidth(_app->window);
    int h = ANativeWindow_getHeight(_app->window);
    _output = new AnsiWidget(w, h);
    if (_output && _output->construct()) {
      _eventQueue = new Stack<MAEvent *>();
      if (_eventQueue) {
        _state = kActiveState;
      }
    }
  }
}

void Runtime::disableSensor() {
  logEntered();
  if (_sensorEventQueue) {
    for (auto &i : _sensors) {
      if (i) {
        ASensorEventQueue_disableSensor(_sensorEventQueue, i);
        i = nullptr;
      }
    }
    ASensorManager_destroyEventQueue(_sensorManager, _sensorEventQueue);
  }
  _sensorEventQueue = nullptr;
}

bool Runtime::enableSensor(int sensorId) {
  bool result;
  if (sensorId < 0 || sensorId >= MAX_SENSORS) {
    result = false;
  } else if (_sensors[sensorId] == nullptr) {
    if (!_sensorEventQueue) {
      _sensorEventQueue = ASensorManager_createEventQueue(_sensorManager, _looper, ALOOPER_POLL_CALLBACK, nullptr, nullptr);
    }
    _sensors[sensorId] = ASensorManager_getDefaultSensor(_sensorManager, SENSOR_TYPES[sensorId]);
    result = _sensors[sensorId] != nullptr;
    if (result) {
      ASensorEventQueue_enableSensor(_sensorEventQueue, _sensors[sensorId]);
    }
  } else {
    result = true;
  }
  return result;
}

bool Runtime::getBoolean(const char *methodName) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "()Z");
  auto result = (jboolean) env->CallBooleanMethod(_app->activity->clazz, methodId);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

String Runtime::getString(const char *methodName) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "()Ljava/lang/String;");
  auto resultObj = (jstring)env->CallObjectMethod(_app->activity->clazz, methodId);
  const char *resultStr = env->GetStringUTFChars(resultObj, JNI_FALSE);
  String result = resultStr;
  env->ReleaseStringUTFChars(resultObj, resultStr);
  env->DeleteLocalRef(resultObj);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

String Runtime::getStringBytes(const char *methodName) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "()[B");
  auto valueByteArray = (jbyteArray)env->CallObjectMethod(_app->activity->clazz, methodId);
  jsize len = env->GetArrayLength(valueByteArray);
  String result;
  if (len) {
    auto *buffer = new jbyte[len + 1];
    env->GetByteArrayRegion(valueByteArray, 0, len, buffer);
    buffer[len] = '\0';
    result = (const char *)buffer;
    delete [] buffer;
  }
  env->DeleteLocalRef(valueByteArray);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

int Runtime::getInteger(const char *methodName) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "()I");
  jint result = env->CallIntMethod(_app->activity->clazz, methodId);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

int Runtime::getIntegerFromString(const char *methodName, const char *value) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jbyteArray valueByteArray = newByteArray(env, value);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "([B)I");
  jint result = env->CallIntMethod(_app->activity->clazz, methodId, valueByteArray);
  env->DeleteLocalRef(valueByteArray);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

int Runtime::getUnicodeChar(int keyCode, int metaState) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "getUnicodeChar", "(II)I");
  jint result = env->CallIntMethod(_app->activity->clazz, methodId, keyCode, metaState);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
  return result;
}

char *Runtime::loadResource(const char *fileName) {
  char *buffer = System::loadResource(fileName);
  if (buffer == nullptr && strcmp(fileName, MAIN_BAS) == 0) {
    AAssetManager *assetManager = _app->activity->assetManager;
    AAsset *mainBasFile = AAssetManager_open(assetManager, "main.bas", AASSET_MODE_BUFFER);
    off_t len = AAsset_getLength(mainBasFile);
    buffer = (char *)malloc(len + 1);
    if (AAsset_read(mainBasFile, buffer, len) < 0) {
      trace("failed to read main.bas");
    }
    buffer[len] = '\0';
    trace("loaded main.bas [%ld] bytes", len);
    AAsset_close(mainBasFile);
  }
  return buffer;
}

MAEvent *Runtime::popEvent() {
  pthread_mutex_lock(&_mutex);
  MAEvent *result = _eventQueue->pop();
  pthread_mutex_unlock(&_mutex);
  return result;
}

void Runtime::pushEvent(MAEvent *event) {
  pthread_mutex_lock(&_mutex);
  _eventQueue->push(event);
  pthread_mutex_unlock(&_mutex);
}

void Runtime::setFloat(const char *methodName, float value) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "(F)V");
  env->CallVoidMethod(_app->activity->clazz, methodId, value);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::setSensorData(var_t *retval) {
  v_init(retval);
  map_init(retval);

  ASensorEvent sensorEvent;
  if (ASensorEventQueue_getEvents(_sensorEventQueue, &sensorEvent, 1) == 1) {
    // find the sensor for the event type
    for (int i = 0; i < MAX_SENSORS; i++) {
      if (sensorEvent.type == SENSOR_TYPES[i]) {
        v_setint(map_add_var(retval, "type", 0), i);
        v_setstr(map_add_var(retval, "name", 0), ASensor_getName(_sensors[i]));
        break;
      }
    }
    switch (sensorEvent.type) {
    case ASENSOR_TYPE_ACCELEROMETER:
    case ASENSOR_TYPE_MAGNETIC_FIELD:
    case ASENSOR_TYPE_GYROSCOPE:
      v_setreal(map_add_var(retval, "x", 0), sensorEvent.vector.x);
      v_setreal(map_add_var(retval, "y", 0), sensorEvent.vector.y);
      v_setreal(map_add_var(retval, "z", 0), sensorEvent.vector.z);
      break;
    case ASENSOR_TYPE_LIGHT:
      v_setreal(map_add_var(retval, "light", 0), sensorEvent.light);
      break;
    case ASENSOR_TYPE_PROXIMITY:
      v_setreal(map_add_var(retval, "distance", 0), sensorEvent.distance);
      break;
    case ASENSOR_TYPE_PRESSURE:
      v_setreal(map_add_var(retval, "pressure", 0), sensorEvent.pressure);
      break;
    case ASENSOR_TYPE_RELATIVE_HUMIDITY:
      v_setreal(map_add_var(retval, "relative_humidity", 0), sensorEvent.relative_humidity);
      break;
    case ASENSOR_TYPE_AMBIENT_TEMPERATURE:
      v_setreal(map_add_var(retval, "temperature", 0), sensorEvent.temperature);
      break;
    default:
      break;
    }
  }
}

void Runtime::runShell() {
  logEntered();

  opt_ide = IDE_NONE;
  opt_graphics = 1;
  opt_nosave = 1;
  opt_verbose = 0;
  opt_quiet = 1;
  opt_command[0] = 0;
  opt_file_permitted = 1;
  os_graphics = 1;
  opt_mute_audio = 0;

  _app->activity->callbacks->onContentRectChanged = onContentRectChanged;
  loadConfig();

  strlcpy(opt_modpath, getString("getModulePath"), sizeof(opt_modpath));

#if defined(_ANDROID_LIBRARY)
  runOnce(MAIN_BAS, true);
#else
  String ipAddress = getString("getIpAddress");
  if (!ipAddress.empty()) {
    setenv("IP_ADDR", ipAddress.c_str(), 1);
  }

  String startupBas = getString("getStartupBas");
  if (!startupBas.empty()) {
    if (getBoolean("getUntrusted")) {
      opt_file_permitted = 0;
    }
    runOnce(startupBas.c_str(), true);
  } else {
    runMain(MAIN_BAS);
  }
  saveConfig();
#endif

  _state = kDoneState;
  logLeaving();
}

void Runtime::loadConfig() {
  Properties<String *> settings;
  int fontSize = getInteger("getStartupFontSize");
  trace("fontSize = %d", fontSize);

  int height = getInteger("getWindowHeight");
  if (height !=  _graphics->getHeight()) {
    // height adjustment for bottom virtual navigation bar
    onResize(_graphics->getWidth(), height, 0);
  }

  _output->setTextColor(DEFAULT_FOREGROUND, DEFAULT_BACKGROUND);
  _output->setFontSize(fontSize);
  _initialFontSize = _output->getFontSize();

  onRunCompleted();
  if (loadSettings(settings)) {
    String *s = settings.get(FONT_SCALE_KEY);
    if (s) {
      _fontScale = s->toInteger();
      trace("_fontScale = %d", _fontScale);
      if (_fontScale != 100) {
        fontSize = (_initialFontSize * _fontScale / 100);
        _output->setFontSize(fontSize);
      }
    }
    s = settings.get(MUTE_AUDIO_KEY);
    if (s && s->toInteger() == 1) {
      opt_mute_audio = 1;
    }
    s = settings.get(OPT_IDE_KEY);
    if (s) {
      opt_ide = s->toInteger();
    }
    s = settings.get(THEME_KEY);
    if (s) {
      int id = s->toInteger();
      if (id > -1 && id < NUM_THEMES) {
        g_themeId = id;
      }
    }
    s = settings.get(LOAD_MODULES_KEY);
    if (s && s->toInteger() == 1) {
      if (getBoolean(LOAD_MODULES_KEY)) {
        systemLog("Extension modules loaded\n");
        settings.put(LOAD_MODULES_KEY, "2");
      } else {
        systemLog("Loading extension modules failed\n");
        settings.put(LOAD_MODULES_KEY, "0");
      }
    }
    loadEnvConfig(settings, SERVER_SOCKET_KEY);
    loadEnvConfig(settings, SERVER_TOKEN_KEY);
    loadEnvConfig(settings, LOAD_MODULES_KEY);
    loadEnvConfig(settings, FONT_ID_KEY);
  }
}

bool Runtime::loadSettings(Properties<String *> &settings) {
  bool result;
  String path;

  path.append(_app->activity->internalDataPath);
  path.append(CONFIG_FILE);
  FILE *fp = fopen(path.c_str(), "r");
  if (fp) {
    String buffer;
    struct stat st{};
    if (stat(path.c_str(), &st) == 0) {
      long len = st.st_size;
      buffer.append(fp, len);
      settings.load(buffer.c_str(), buffer.length());
      result = true;
    } else {
      result = false;
    }
    fclose(fp);
  } else {
    result = false;
  }
  return result;
}

void Runtime::loadEnvConfig(Properties<String *> &settings, const char *key) {
  String *s = settings.get(key);
  if (s) {
    trace("%s = %s", key, s->c_str());
    setenv(key, s->c_str(), 1);
  }
}

void Runtime::saveConfig() {
  String path;
  path.append(_app->activity->internalDataPath);
  path.append(CONFIG_FILE);
  FILE *fp = fopen(path.c_str(), "w");
  if (fp) {
    char buffer[FILENAME_MAX + 1];
    getcwd(buffer, FILENAME_MAX);
    fprintf(fp, "%s='%s'\n", PATH_KEY, buffer);
    fprintf(fp, "%s=%d\n", FONT_SCALE_KEY, _fontScale);
    fprintf(fp, "%s=%d\n", MUTE_AUDIO_KEY, opt_mute_audio);
    fprintf(fp, "%s=%d\n", OPT_IDE_KEY, opt_ide);
    fprintf(fp, "%s=%d\n", THEME_KEY, g_themeId);
    for (int i = 0; environ[i] != nullptr; i++) {
      char *env = environ[i];
      if (strstr(env, SERVER_SOCKET_KEY) != nullptr ||
          strstr(env, SERVER_TOKEN_KEY) != nullptr ||
          strstr(env, FONT_ID_KEY) != nullptr) {
        fprintf(fp, "%s\n", env);
      } else if (strstr(env, LOAD_MODULES_KEY) != nullptr) {
        if (strstr(env, LOAD_MODULES_KEY "=2") != nullptr)  {
          fprintf(fp, "%s=1\n", LOAD_MODULES_KEY);
        } else {
          fprintf(fp, "%s\n", env);
        }
      }
    }
    fclose(fp);
  }
}

void Runtime::runPath(const char *path) {
  pthread_mutex_lock(&_mutex);
  setLoadPath(path);
  auto *event = new MAEvent();
  event->type = EVENT_TYPE_EXIT;
  _eventQueue->push(event);
  ALooper_wake(_looper);
  pthread_mutex_unlock(&_mutex);
}

void Runtime::handleKeyEvent(MAEvent &event) {
  switch (event.nativeKey) {
  case AKEYCODE_ENDCALL:
    delete [] _systemMenu;
    _systemMenu = nullptr;
    break;
  case AKEYCODE_BACK:
    if (_keypadActive) {
      showKeypad(false);
    } else {
      setBack();
    }
    break;
  case AKEYCODE_MENU:
    showMenu();
    event.key = SB_KEY_MENU;
    break;
  case AKEYCODE_TAB:
    event.key = SB_KEY_TAB;
    break;
  case AKEYCODE_HOME:
    event.key = SB_KEY_KP_HOME;
    break;
  case AKEYCODE_MOVE_HOME:
    event.key = SB_KEY_HOME;
    break;
  case AKEYCODE_MOVE_END:
    event.key = SB_KEY_END;
    break;
  case AKEYCODE_INSERT:
    event.key = SB_KEY_INSERT;
    break;
  case AKEYCODE_NUMPAD_MULTIPLY:
    event.key = SB_KEY_KP_MUL;
    break;
  case AKEYCODE_NUMPAD_ADD:
    event.key = SB_KEY_KP_PLUS;
    break;
  case AKEYCODE_NUMPAD_SUBTRACT:
    event.key = SB_KEY_KP_MINUS;
    break;
  case AKEYCODE_PAGE_UP:
  case AKEYCODE_VOLUME_UP:
    event.key = SB_KEY_PGUP;
    break;
  case AKEYCODE_PAGE_DOWN:
  case AKEYCODE_VOLUME_DOWN:
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
    event.key = SB_KEY_DELETE;
    break;
  case AKEYCODE_FORWARD_DEL:
    event.key = SB_KEY_DELETE;
    break;
  case AKEYCODE_DEL:
    event.key = SB_KEY_BACKSPACE;
    break;
  case AKEYCODE_ENTER:
    event.key = SB_KEY_ENTER;
    break;
  case GBOARD_KEY_QUESTION:
    event.key = '?';
    break;
  case AKEYCODE_ESCAPE:
    event.key = SB_KEY_ESCAPE;
    break;
  case AKEYCODE_BREAK:
    event.key = SB_KEY_BREAK;
    break;
  default:
    if (event.nativeKey >= AKEYCODE_F1 && event.nativeKey <= AKEYCODE_F12) {
      for (int fn = 0; fn < 12; fn++) {
        if (event.nativeKey == AKEYCODE_F1 + fn) {
          event.key = SB_KEY_F(fn + 1);
          break;
        }
      }
    } else if (event.nativeKey != 0 && event.nativeKey < 127 && event.nativeKey != event.key) {
      // avoid translating keys send from onUnicodeChar
      event.key = getUnicodeChar(event.nativeKey, event.key);
    }
    break;
  }
  trace("native:%d sb:%d", event.nativeKey, event.key);
  if (isRunning() && event.key) {
    dev_pushkey(event.key);
  }
}

void Runtime::optionsBox(StringList *items) {
  logEntered();

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);

  jstring elem = env->NewStringUTF(items->get(0)->c_str());
  jclass stringClass = env->GetObjectClass(elem);
  jobjectArray array = env->NewObjectArray(items->size(), stringClass, elem);
  for (int i = 1; i < items->size(); i++) {
    elem = env->NewStringUTF(items->get(i)->c_str());
    env->SetObjectArrayElement(array, i, elem);
  }
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "optionsBox", "([Ljava/lang/String;)V");
  env->CallVoidMethod(_app->activity->clazz, methodId, array);

  for (int i = 0; i < items->size(); i++) {
    env->DeleteLocalRef(env->GetObjectArrayElement(array, i));
  }
  env->DeleteLocalRef(clazz);
  env->DeleteLocalRef(array);
  env->DeleteLocalRef(stringClass);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::pause(int timeout) {
  if (timeout == -1) {
    pollEvents(true);
    if (hasEvent()) {
      MAEvent *event = popEvent();
      processEvent(*event);
      delete event;
    }
  } else {
    uint32_t startTime = dev_get_millisecond_count();
    while (true) {
      pollEvents(false);
      if (isBreak()) {
        break;
      } else if (hasEvent()) {
        MAEvent *event = popEvent();
        processEvent(*event);
        delete event;
      }
      usleep(1000);
      if (timeout > 0 && (dev_get_millisecond_count() - startTime) > timeout) {
        break;
      }
    }
  }
}

void Runtime::pollEvents(bool blocking) {
  int events;
  android_poll_source *source;
  ALooper_pollOnce(blocking || !_hasFocus ? -1 : 0, nullptr, &events, (void **)&source);
  if (source != nullptr) {
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
    if (!hasEvent() || _eventQueue->peek()->type == EVENT_TYPE_POINTER_DRAGGED) {
      // drain any motion events so we only return the latest
      while (hasEvent()) {
        delete popEvent();
      }
      _output->flush(true);
      pollEvents(true);
    }
    break;
  case 2:
    _output->flush(false);
    pause(WAIT_INTERVAL);
    break;
  default:
    pollEvents(false);
  }

  MAEvent event;
  if (hasEvent()) {
    MAEvent *nextEvent = popEvent();
    processEvent(*nextEvent);
    event = *nextEvent;
    delete nextEvent;
  } else {
    event.type = 0;
  }
  return event;
}

void Runtime::processEvent(MAEvent &event) {
  switch (event.type) {
  case EVENT_TYPE_SCREEN_CHANGED:
    if (_graphics->resize()) {
      resize();
    } else {
      alert("System error: Failed to resize screen");
    }
    break;
  case EVENT_TYPE_KEY_PRESSED:
    handleKeyEvent(event);
    break;
  case EVENT_TYPE_EXIT:
    setExit(false);
    break;
  default:
    handleEvent(event);
    break;
  }
}

void Runtime::setString(const char *methodName, const char *value) {
  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jbyteArray valueByteArray = newByteArray(env, value);
  jmethodID methodId = env->GetMethodID(clazz, methodName, "([B)V");
  env->CallVoidMethod(_app->activity->clazz, methodId, valueByteArray);
  env->DeleteLocalRef(valueByteArray);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::showKeypad(bool show) {
  logEntered();
  _keypadActive = show;

  JNIEnv *env;
  _app->activity->vm->AttachCurrentThread(&env, nullptr);
  jclass clazz = env->GetObjectClass(_app->activity->clazz);
  jmethodID methodId = env->GetMethodID(clazz, "showKeypad", "(Z)V");
  env->CallVoidMethod(_app->activity->clazz, methodId, show);
  env->DeleteLocalRef(clazz);
  _app->activity->vm->DetachCurrentThread();
}

void Runtime::onResize(int width, int height, int imeState) {
  logEntered();
  if (_graphics != nullptr) {
    int w = _graphics->getWidth();
    int h = _graphics->getHeight();
    if (w != width || h != height) {
      trace("Resized from %d %d to %d %d [ime=%d]", w, h, width, height, imeState);
      ALooper_acquire(_app->looper);
      if (imeState != 0) {
        // in android 16+ when resize also knows whether the ime (keypad) is active
        _keypadActive = (imeState > 0);
      }
      _graphics->setSize(width, height);
      auto *maEvent = new MAEvent();
      maEvent->type = EVENT_TYPE_SCREEN_CHANGED;
      runtime->pushEvent(maEvent);
      ALooper_wake(_app->looper);
      ALooper_release(_app->looper);
    }
  }
}

void Runtime::onRunCompleted() {
  if (!_mainBas) {
    const char *storage = getenv("EXTERNAL_DIR");
    if (!storage) {
      storage = getenv("INTERNAL_DIR");
    }
    if (storage) {
      setenv("HOME_DIR", storage, 1);
      chdir(storage);
    }
  }
}

void Runtime::onUnicodeChar(int ch) {
  auto *maEvent = new MAEvent();
  maEvent->type = EVENT_TYPE_KEY_PRESSED;
  maEvent->nativeKey = ch;
  maEvent->key = ch;
  ALooper_acquire(_app->looper);
  pushEvent(maEvent);
  ALooper_wake(_app->looper);
  ALooper_release(_app->looper);
}

char *Runtime::getClipboardText() {
  char *result;
  String text = getStringBytes("getClipboardText");
  if (!text.empty()) {
    result = strdup(text.c_str());
  } else {
    result = nullptr;
  }
  return result;
}

int Runtime::getFontId() {
  int result = 0;
  Properties<String *> settings;
  if (loadSettings(settings)) {
    String *s = settings.get(FONT_ID_KEY);
    if (s) {
      result = s->toInteger();
    }
  }
  return result;
}

//
// System platform methods
//
bool System::getPen3() {
  processEvents(0);
  return _buttonPressed;
}

void System::completeKeyword(int index) const {
  if (get_focus_edit() && isEditing()) {
    const char *help = get_focus_edit()->completeKeyword(index);
    if (help) {
      runtime->alert(help);
      runtime->getOutput()->redraw();
    }
  }
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

void maPushEvent(MAEvent *maEvent) {
  runtime->pushEvent(maEvent);
}

void maWait(int timeout) {
  runtime->pause(timeout);
}

void maShowVirtualKeyboard() {
  runtime->showKeypad(true);
}

void maHideVirtualKeyboard() {
  runtime->showKeypad(false);
}

//
// sbasic implementation
//
int osd_devinit(void) {
  runtime->setRunning(true);
  return 1;
}

int osd_devrestore(void) {
  runtime->setRunning(false);
  return 0;
}

void osd_audio(const char *path) {
  runtime->playAudio(path);
}

void osd_sound(int frq, int dur, int vol, int bgplay) {
  if (dur > 0) {
    runtime->playTone(frq, dur, vol, bgplay);
  }
}

void osd_clear_sound_queue() {
  runtime->clearSoundQueue();
}

void osd_beep(void) {
  osd_sound(1000, 30, 100, 0);
  osd_sound(500, 30, 100, 0);
}
