#include <jni.h>
#include <android/log.h>

JNIEnv *g_env;

extern "C" JNIEXPORT void JNICALL
  Java_net_sourceforge_smallbasic_ioio_IOIOUtility_init(JNIEnv *env, jclass clazz) {
    __android_log_print(ANDROID_LOG_INFO, "smallbasic", "init entered");
    g_env = env;
}
