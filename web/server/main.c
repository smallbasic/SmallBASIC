// This file is part of SmallBASIC
//
// Copyright(C) 2001-2016 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include <jni.h>
#include <string.h>

JNIEXPORT jstring JNICALL Java_net_sourceforge_smallbasic_WebServer_execute
  (JNIEnv *env, jclass jclazz, jstring str) {
  const char *fileName = (*env)->GetStringUTFChars(env, str, NULL);
  char msg[60] = "Hello ";
  strcat(msg, fileName);
  puts(msg);

  (*env)->ReleaseStringUTFChars(env, str, fileName);
  return (*env)->NewStringUTF(env, msg);
}

