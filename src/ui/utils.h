// This file is part of SmallBASIC
//
// Copyright(C) 2001-2019 Chris Warren-Smith.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#ifndef UTILS_H
#define UTILS_H

#ifndef MAX
 #define MAX(a,b) ((a<b) ? (b) : (a))
#endif
#ifndef MIN
 #define MIN(a,b) ((a>b) ? (b) : (a))
#endif

#define DEFAULT_FOREGROUND 0xa1a1a1
#define DEFAULT_BACKGROUND 0
#define HANDLE_SCREEN_BUFFER HANDLE_SCREEN
#define USER_MESSAGE_EXIT 1000

#define OUTSIDE_RECT(px, py, x, y, w, h) \
  (px < (x) || py < (y) || px > ((x)+(w)) || py > ((y)+(h)))

#if defined(_ANDROID)
 #include <android/log.h>
 #define deviceLog(...) __android_log_print(ANDROID_LOG_INFO, \
                        "smallbasic", __VA_ARGS__)
#elif defined(_SDL) || defined(_FLTK)
 void appLog(const char *format, ...);
 #define deviceLog(...) appLog(__VA_ARGS__)
#endif

#if defined(_DEBUG)
 #define trace(...) deviceLog(__VA_ARGS__)
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

