// This file is part of SmallBASIC
//
// Copyright(C) 2001-2025 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"

#include "platform/android/jni/runtime.h"
#include "languages/messages.en.h"
#include "common/sys.h"

#define USB_OBJECT_ID 1001
#define USB_CLASS_ID 1002
#define BLUETOOTH_OBJECT_ID 1003
#define BLUETOOTH_CLASS_ID 1004
#define TAG_CONNECTED "[--tag-connected--]"
#define TAG_ERROR "[--tag-error--]"

extern Runtime *runtime;

static bool is_usb_object(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && (var->v.m.id == USB_OBJECT_ID);
}

static bool is_bluetooth_object(var_p_t var) {
  return var != nullptr && v_is_type(var, V_MAP) && (var->v.m.id == BLUETOOTH_OBJECT_ID);
}

static int get_integer_from_string(const char *methodName, const char *str, var_s *retval) {
  int result;
  auto value = runtime->getIntegerFromString(methodName, str);
  if (value == -1) {
    v_setstr(retval, ERR_CONNECTION);
    result = 0;
  } else {
    v_setint(retval, value);
    result = 1;
  }
  return result;
}

static int get_integer(const char *methodName, var_s *retval) {
  int result;
  auto value = runtime->getInteger(methodName);
  if (value == -1) {
    v_setstr(retval, ERR_CONNECTION);
    result = 0;
  } else {
    v_setint(retval, value);
    result = 1;
  }
  return result;
}

static int get_string(const char *methodName, var_s *retval) {
  int result;
  auto str = runtime->getString(methodName);
  if (strncmp(str, TAG_ERROR, STRLEN(TAG_ERROR)) == 0) {
    v_setstr(retval, ERR_CONNECTION);
    result = 0;
  } else {
    v_setstr(retval, str);
    result = 1;
  }
  return result;
}

static int cmd_usb_close(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_usb_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    runtime->getBoolean("usbClose");
    result = 1;
  }
  return result;
}

static int cmd_usb_description(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 1 || !is_usb_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    result = get_string("usbDescription", retval);
  }
  return result;
}

static int cmd_usb_receive(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_usb_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    result = get_string("usbReceive", retval);
  }
  return result;
}

static int cmd_usb_send(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 1 || !is_usb_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    if (v_is_type(args[0].var_p, V_STR)) {
      auto str = v_getstr(args[0].var_p);
      result = get_integer_from_string("usbSend", str, retval);
    } else {
      auto str = v_str(args[0].var_p);
      result = get_integer_from_string("usbSend", str, retval);
      free(str);
    }
  }
  return result;
}

static int cmd_usb_connect(int argc, slib_par_t *args, var_t *retval) {
  int result = 0;

  if (argc < 1 || argc > 3 || !v_is_type(args[0].var_p, V_INT)) {
    v_setstr(retval, "Expected: vendorId");
  } else {
    runtime->getOutput()->redraw();
    android_app *app = runtime->getApp();

    JNIEnv *env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);
    int vendorId = v_getint(args[0].var_p);
    int baud = argc >= 2 ? (int)v_getint(args[1].var_p) : 0;
    int timeout = argc == 3 ? (int)v_getint(args[2].var_p) : -1;
    jclass clazz = env->GetObjectClass(app->activity->clazz);
    const char *signature = "(III)Ljava/lang/String;";
    jmethodID methodId = env->GetMethodID(clazz, "usbConnect", signature);
    auto jstr = (jstring)env->CallObjectMethod(app->activity->clazz, methodId, vendorId, baud, timeout);
    const char *str = env->GetStringUTFChars(jstr, JNI_FALSE);

    if (strncmp(str, TAG_CONNECTED, STRLEN(TAG_CONNECTED)) == 0) {
      map_init(retval);
      retval->v.m.id = USB_OBJECT_ID;
      retval->v.m.cls_id = USB_CLASS_ID;
      v_create_callback(retval, "close", cmd_usb_close);
      v_create_callback(retval, "description", cmd_usb_description);
      v_create_callback(retval, "receive", cmd_usb_receive);
      v_create_callback(retval, "send", cmd_usb_send);
      result = 1;
    } else {
      v_setstr(retval, str);
      result = 0;
    }

    env->ReleaseStringUTFChars(jstr, str);
    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(clazz);
    app->activity->vm->DetachCurrentThread();
  }
  return result;
}

static int cmd_request(int argc, slib_par_t *args, var_t *retval) {
  int result = 0;
  if (argc != 1 && argc != 3) {
    v_setstr(retval, "Expected 1 or 3 arguments");
  } else if (!v_is_type(args[0].var_p, V_STR)) {
    v_setstr(retval, "Invalid endPoint");
  } else if (argc == 3 && !v_is_type(args[1].var_p, V_STR) && !v_is_type(args[1].var_p, V_MAP)) {
    v_setstr(retval, "Invalid postData");
  } else if (argc == 3 && !v_is_type(args[2].var_p, V_STR)) {
    v_setstr(retval, "Invalid apiKey");
  } else {
    runtime->getOutput()->redraw();
    android_app *app = runtime->getApp();

    JNIEnv *env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);
    auto endPoint = env->NewStringUTF(v_getstr(args[0].var_p));
    auto data = env->NewStringUTF(argc < 2 ? "" : v_getstr(args[1].var_p));
    auto apiKey = env->NewStringUTF(argc < 3 ? "" : v_getstr(args[2].var_p));

    jclass clazz = env->GetObjectClass(app->activity->clazz);
    const char *signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";
    jmethodID methodId = env->GetMethodID(clazz, "request", signature);
    auto jstr = (jstring)env->CallObjectMethod(app->activity->clazz, methodId, endPoint, data, apiKey);
    const char *str = env->GetStringUTFChars(jstr, JNI_FALSE);

    v_setstr(retval, str);
    result = strncmp(str, "error: [", 8) == 0 ? 0 : 1;

    env->ReleaseStringUTFChars(jstr, str);
    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(endPoint);
    env->DeleteLocalRef(data);
    env->DeleteLocalRef(apiKey);
    app->activity->vm->DetachCurrentThread();
  }
  return result;
}

static int cmd_bluetooth_close(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_bluetooth_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    runtime->getBoolean("bluetoothClose");
    result = 1;
  }
  return result;
}

static int cmd_bluetooth_connected(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_bluetooth_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    result = get_integer("bluetoothConnected", retval);
  }
  return result;
}

static int cmd_bluetooth_description(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_bluetooth_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    result = get_string("bluetoothDescription", retval);
  }
  return result;
}

static int cmd_bluetooth_receive(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 0 || !is_bluetooth_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    result = get_string("bluetoothReceive", retval);
  }
  return result;
}

static int cmd_bluetooth_send(var_s *self, int argc, slib_par_t *args, var_s *retval) {
  int result;
  if (argc != 1 || !is_bluetooth_object(self)) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    if (v_is_type(args[0].var_p, V_STR)) {
      auto str = v_getstr(args[0].var_p);
      result = get_integer_from_string("bluetoothSend", str, retval);
    } else {
      auto str = v_str(args[0].var_p);
      result = get_integer_from_string("bluetoothSend", str, retval);
      free(str);
    }
  }
  return result;
}

static int cmd_bluetooth_connect(int argc, slib_par_t *args, var_t *retval) {
  int result = 0;

  if (argc != 1 || !v_is_type(args[0].var_p, V_STR)) {
    v_setstr(retval, "Invalid device name");
  } else {
    runtime->getOutput()->redraw();
    android_app *app = runtime->getApp();

    JNIEnv *env;
    app->activity->vm->AttachCurrentThread(&env, nullptr);
    auto deviceName = env->NewStringUTF(v_getstr(args[0].var_p));
    jclass clazz = env->GetObjectClass(app->activity->clazz);
    const char *signature = "(Ljava/lang/String;)Ljava/lang/String;";
    jmethodID methodId = env->GetMethodID(clazz, "bluetoothConnect", signature);
    auto jstr = (jstring)env->CallObjectMethod(app->activity->clazz, methodId, deviceName);
    const char *str = env->GetStringUTFChars(jstr, JNI_FALSE);

    if (strncmp(str, TAG_CONNECTED, STRLEN(TAG_CONNECTED)) == 0) {
      map_init(retval);
      retval->v.m.id = BLUETOOTH_OBJECT_ID;
      retval->v.m.cls_id = BLUETOOTH_CLASS_ID;
      v_create_callback(retval, "close", cmd_bluetooth_close);
      v_create_callback(retval, "connected", cmd_bluetooth_connected);
      v_create_callback(retval, "description", cmd_bluetooth_description);
      v_create_callback(retval, "receive", cmd_bluetooth_receive);
      v_create_callback(retval, "send", cmd_bluetooth_send);
      result = 1;
    } else {
      v_setstr(retval, str);
      result = 0;
    }

    env->ReleaseStringUTFChars(jstr, str);
    env->DeleteLocalRef(jstr);
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(deviceName);
    app->activity->vm->DetachCurrentThread();
  }
  return result;
}

static int cmd_location(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (argc != 0) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    String location = runtime->getString("getLocation");
    map_parse_str(location.c_str(), location.length(), retval);
    result = 1;
  }
  return result;
}

static int cmd_sensor(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (argc != 0) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    runtime->setSensorData(retval);
    result = 1;
  }
  return result;
}

static int cmd_gps_on(int argc, slib_par_t *args, var_t *retval) {
  runtime->getBoolean("requestLocationUpdates");
  return 1;
}

static int cmd_gps_off(int argc, slib_par_t *args, var_t *retval) {
  runtime->getBoolean("removeLocationUpdates");
  return 1;
}

static int cmd_sensor_on(int argc, slib_par_t *args, var_t *retval) {
  int result = 0;
  if (argc == 1) {
    int sensor = v_getint(args[0].var_p);
    if (sensor >= 0 && sensor < MAX_SENSORS) {
      result = runtime->enableSensor(sensor);
    }
  }
  if (!result) {
    v_setstr(retval, "sensor not active");
  }
  return result;
}

static int cmd_sensor_off(int argc, slib_par_t *args, var_t *retval) {
  runtime->disableSensor();
  return 1;
}

static int cmd_tts_speak(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (opt_mute_audio) {
    result = 1;
  } else if (argc != 1) {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  } else {
    if (v_is_type(args[0].var_p, V_STR)) {
      auto str = v_getstr(args[0].var_p);
      runtime->speak(str);
    } else {
      auto str = v_str(args[0].var_p);
      runtime->speak(str);
      free(str);
    }
    result = 1;
  }
  return result;
}

static int cmd_tts_pitch(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (argc == 1 && (v_is_type(args[0].var_p, V_NUM) ||
                    v_is_type(args[0].var_p, V_INT))) {
    runtime->setFloat("setTtsPitch", v_getreal(args[0].var_p));
    result = 1;
  } else {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  }
  return result;
}

static int cmd_tts_speech_rate(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (argc == 1 && (v_is_type(args[0].var_p, V_NUM) ||
                    v_is_type(args[0].var_p, V_INT))) {
    runtime->setFloat("setTtsRate", v_getreal(args[0].var_p));
    result = 1;
  } else {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  }
  return result;
}

static int cmd_tts_lang(int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (argc == 1 && v_is_type(args[0].var_p, V_STR)) {
    runtime->setString("setTtsLocale", v_getstr(args[0].var_p));
    result = 1;
  } else {
    v_setstr(retval, ERR_PARAM);
    result = 0;
  }
  return result;
}

static int cmd_tts_off(int argc, slib_par_t *args, var_t *retval) {
  runtime->getBoolean("setTtsQuiet");
  return 1;
}

struct LibProcs {
  const char *name;
  int (*command)(int, slib_par_t *, var_t *retval);
} lib_procs[] = {
  {"GPS_ON", cmd_gps_on},
  {"GPS_OFF", cmd_gps_off},
  {"SENSOR_ON", cmd_sensor_on},
  {"SENSOR_OFF", cmd_sensor_off},
  {"TTS_PITCH", cmd_tts_pitch},
  {"TTS_RATE", cmd_tts_speech_rate},
  {"TTS_LANG", cmd_tts_lang},
  {"TTS_OFF", cmd_tts_off},
  {"SPEAK", cmd_tts_speak}
};

struct LibFuncs {
  const char *name;
  int (*command)(int, slib_par_t *, var_t *retval);
} lib_funcs[] = {
  {"LOCATION", cmd_location},
  {"SENSOR", cmd_sensor},
  {"REQUEST", cmd_request},
  {"OPENUSBSERIAL", cmd_usb_connect},
  {"OPENBLUETOOTH", cmd_bluetooth_connect}
};

extern "C" int sblib_proc_count(void) {
  return (sizeof(lib_procs) / sizeof(lib_procs[0]));
}

extern "C" int sblib_proc_getname(int index, char *proc_name) {
  int result;
  if (index < sblib_proc_count()) {
    strcpy(proc_name, lib_procs[index].name);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

extern "C" int sblib_proc_exec(int index, int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (index < sblib_proc_count()) {
    result = lib_procs[index].command(argc, args, retval);
  } else {
    result = 0;
  }
  return result;
}

extern "C" int sblib_func_count(void) {
  return (sizeof(lib_funcs) / sizeof(lib_funcs[0]));
}

extern "C" int sblib_func_getname(int index, char *proc_name) {
  int result;
  if (index < sblib_func_count()) {
    strcpy(proc_name, lib_funcs[index].name);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

extern "C" int sblib_func_exec(int index, int argc, slib_par_t *args, var_t *retval) {
  int result;
  if (index < sblib_func_count()) {
    result = lib_funcs[index].command(argc, args, retval);
  } else {
    result = 0;
  }
  return result;
}

extern "C" void sblib_free(int cls_id, int id) {
  if (cls_id == USB_CLASS_ID && id == USB_OBJECT_ID) {
    // when a 'usb' variable falls out of scope
    runtime->getBoolean("usbClose");
  } else if (cls_id == BLUETOOTH_CLASS_ID && id == BLUETOOTH_OBJECT_ID) {
    // when a 'bluetooth' variable falls out of scope
    runtime->getBoolean("bluetoothClose");
  }
}

extern "C" void sblib_close() {
  runtime->getBoolean("closeLibHandlers");
  runtime->disableSensor();
}
