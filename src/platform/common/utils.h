// This file is part of SmallBASIC
//
// Copyright(C) 2001-2013 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef UTILS_H
#define UTILS_H

typedef unsigned char  U8;
typedef unsigned short U16;
typedef signed short   S16;
typedef unsigned long  U32;
typedef signed long    S32;

#ifndef MAX
 #define MAX(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef MIN
 #define MIN(a,b) ((a>b) ? (b) : (a))
#endif

#if defined(_FLTK)
 #define DEFAULT_FOREGROUND 0
 #define DEFAULT_BACKGROUND 0xecedef
 #define HANDLE_SCREEN_BUFFER HANDLE_SCREEN + 1
#else
 #define DEFAULT_FOREGROUND 0xa1a1a1
 #define DEFAULT_BACKGROUND 0
 #define HANDLE_SCREEN_BUFFER HANDLE_SCREEN
#endif

#define USER_MESSAGE_EXIT 1000

#define OUTSIDE_RECT(px, py, x, y, w, h) \
  (px < (x) || py < (y) || px > ((x)+(w)) || py > ((y)+(h)))

#if defined(VARIANT_MOSYNC_EMULATOR)
 #define _DEBUG
#endif

#if defined(_DEBUG)
 #if defined(MAPIP)
  #include <mavsprintf.h>
  #define trace lprintfln
 #elif defined(_FLTK)
  extern "C" void trace(const char *format, ...);
 #elif defined (_TIZEN)
  #include <FBaseLog.h>
  #define trace AppLog
 #elif defined (_ANDROID)
  #include <android/log.h>
  #define trace(...) __android_log_print(ANDROID_LOG_INFO, \
                                         "smallbasic", __VA_ARGS__)
 #endif
#else
 #define trace(...)
#endif

#define logEntered() trace("%s entered (%s %d)", \
                           __FUNCTION__, __FILE__, __LINE__);
#define logLeaving() trace("%s leaving (%s %d)", \
                           __FUNCTION__, __FILE__, __LINE__);

#define C_LINKAGE_BEGIN extern "C" {
#define C_LINKAGE_END }

#endif

