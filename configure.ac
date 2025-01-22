dnl
dnl Configure script for SmallBASIC
dnl
dnl Copyright(C) 2001-2020 Chris Warren-Smith.
dnl
dnl This program is distributed under the terms of the GPL v2.0
dnl Download the GNU Public License (GPL) from www.gnu.org
dnl

AC_INIT([smallbasic], [12.28])
AC_CONFIG_SRCDIR([configure.ac])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

TARGET=""

dnl define build arguments
AC_ARG_ENABLE(emcc,
  AS_HELP_STRING([--enable-emcc],[build emscripten version(default=no)]),
  [ac_build_emcc="yes"],
  [ac_build_emcc="no"])

AC_ARG_ENABLE(sdl,
  AS_HELP_STRING([--enable-sdl],[build SDL version(default=no)]),
  [ac_build_sdl="yes"],
  [ac_build_sdl="no"])

AC_ARG_ENABLE(android,
  AS_HELP_STRING([--enable-android],[build common library for Android(default=no)]),
  [ac_build_android="yes"],
  [ac_build_android="no"])

AC_ARG_ENABLE(web,
  AS_HELP_STRING([--enable-web],[build Web server version(default=no)]),
  [ac_build_web="yes"],
  [ac_build_web="no"])

AC_ARG_ENABLE(fltk,
  AS_HELP_STRING([--enable-fltk],[build fltk version(default=no)]),
  [ac_build_fltk="yes"],
  [ac_build_fltk="no"])

AC_ARG_ENABLE(dist,
  AS_HELP_STRING([--enable-dist],[prepare to run make dist(default=no)]),
  [ac_build_dist="yes"],
  [ac_build_dist="no"])

AC_ARG_ENABLE(teensy,
  AS_HELP_STRING([--enable-teensy],[build teensy version(default=no)]),
  [ac_build_teensy="yes"],
  [ac_build_teensy="no"])

function checkForWindows() {
   win32=no
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
     win32="yes"
     AC_CHECK_TOOL(WINDRES, windres, :)
     ;;
     *darwin*)
     ;;
     *)
   esac
   AM_CONDITIONAL(WITH_WIN32, test x"$win32" = "xyes")
}

function checkDebugMode() {
   AC_MSG_CHECKING([if debug mode is enabled])
   AC_ARG_WITH(debug,
   [  --with-debug            Build the debug version default=no],
       [with_debug=$withval],
       [with_debug=no])
   AC_MSG_RESULT([$with_debug])
   if test "$with_debug" = "yes" || test "$with_debug" = "full"
   then
     CFLAGS="${CFLAGS} -g -O0 -fstack-protector-all"
     CXXFLAGS="${CXXFLAGS} -g -O0 -fstack-protector-all"
     AC_DEFINE(_DEBUG, 1, [debugging build enabled])
   fi
   AC_SUBST(CFLAGS)
}

function checkProfiling() {
   AC_MSG_CHECKING([if profile generate is enabled])
   AC_ARG_WITH(profile-generate,
   [  --with-profile-generate          Build the profile generate default=no],
       [with_prof_gen=$withval],
       [with_prof_gen=no])
   AC_MSG_RESULT([$with_prof_gen])
   if test "$with_prof_gen" = "yes"
   then
     CFLAGS="${CFLAGS} -fprofile-generate"
     CXXFLAGS="${CXXFLAGS} -fprofile-generate"
   else
     AC_MSG_CHECKING([if profile use is enabled])
     AC_ARG_WITH(profile-use,
     [  --with-profile-use          Build the profile use default=no],
         [with_prof_use=$withval],
         [with_prof_use=no])
     AC_MSG_RESULT([$with_prof_use])
     if test "$with_prof_use" = "yes"
     then
       CFLAGS="${CFLAGS} -fprofile-use"
       CXXFLAGS="${CXXFLAGS} -fprofile-use"
     fi
   fi
}

function checkPCRE() {
   AC_CHECK_PROG(have_pcre, pcre-config, [yes], [no])

   dnl supported under linux only for now
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
     have_pcre="no"
   esac

   if test x$ac_build_android = xyes; then
     have_pcre="no"
   fi

   if test x$ac_build_emcc = xyes; then
     have_pcre="no"
   fi

   if test x$ac_build_teensy = xyes; then
     have_pcre="no"
   fi

   if test "${have_pcre}" = "yes" ; then
     AC_DEFINE(USE_PCRE, 1, [match.c used with libpcre.])
     PACKAGE_LIBS="${PACKAGE_LIBS} `pcre-config --libs`"
   fi
}

function checkTermios() {
   AC_CHECK_HEADERS([termios.h], [have_termios_h=yes; break;])

   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
     have_termios_h="no"
   esac

   if test x$ac_build_android = xyes; then
     have_termios_h="no"
   fi

   if test x$ac_build_teensy = xyes; then
     have_termios_h="no"
   fi

   if test "${have_termios_h}" = "yes" ; then
     AC_DEFINE(USE_TERM_IO, 1, [use the termios library.])
   fi
}

function defaultConditionals() {
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
}

function buildSDL() {
   TARGET="Building SDL version."
   defaultConditionals

   AC_CHECK_PROG(have_sdl, sdl2-config, [yes], [no])
   if test "${have_sdl}" = "no" ; then
     AC_MSG_ERROR([libsdl2-dev not installed: configure failed.])
   fi

   SAVED_CPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS `pkg-config freetype2 --cflags`"
   AC_CHECK_HEADERS([ft2build.h], [], [AC_MSG_ERROR([libfreetype6-dev not installed: configure failed.])])
   CPPFLAGS="$SAVED_CPPFLAGS"

   AC_CHECK_PROG(have_xxd, xxd, [yes], [no])
   if test "${have_xxd}" = "no" ; then
     AC_MSG_ERROR([xxd command not installed: configure failed.])
   fi

   case "${host_os}" in
     *mingw* | cygwin*)
     dnl avoid using MSCRT versions of printf for long double
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO"

     AC_DEFINE(_USE_MATH_DEFINES, 1, [for M_PI in math.h])
     AC_DEFINE(realpath(F, R), _fullpath(R, F, PATH_MAX), [add missing realpath for system.cpp])

     dnl do not depend on cygwin.dll under cygwin build
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -mms-bitfields"
     PACKAGE_LIBS="${PACKAGE_LIBS} -lwsock32 -lws2_32 -static-libgcc -static-libstdc++"
     PACKAGE_LIBS="${PACKAGE_LIBS} -Wl,-Bstatic -Wl,-Map=output.map"
     PACKAGE_LIBS="${PACKAGE_LIBS} `sdl2-config --static-libs` `pkg-config freetype2 --libs --static`"
     AC_DEFINE(_Win32, 1, [Windows build])
     ;;

     *darwin*)
     # OSX Check fontconfig configuration
     PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.2)
     AC_SUBST(FONTCONFIG_CFLAGS)
     AC_SUBST(FONTCONFIG_LIBS)

     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} ${FONTCONFIG_CFLAGS}"

     dnl backlinking support for modules
     PACKAGE_LIBS="${PACKAGE_LIBS} -ldl"
     PACKAGE_LIBS="${PACKAGE_LIBS} ${FONTCONFIG_LIBS}"
     PACKAGE_LIBS="${PACKAGE_LIBS} `sdl2-config --libs` `pkg-config freetype2 --libs`"
     ;;

     *)
     # Unix - Check fontconfig configuration
     PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.2)
     AC_SUBST(FONTCONFIG_CFLAGS)
     AC_SUBST(FONTCONFIG_LIBS)

     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} ${FONTCONFIG_CFLAGS}"

     (cd images && xxd -i sb-desktop-128x128.png > ../src/platform/sdl/icon.h)

     HAVE_SDL2_STATIC=yes
     AC_MSG_CHECKING([if installed SDL2 supports static libs])
     AC_PATH_PROGS_FEATURE_CHECK([SDL2_STATIC_TMP], [sdl2-config],
       AS_IF([`"$ac_path_SDL2_STATIC_TMP" --static-libs  2>&1 | sed '/Usage/!{q1};' > /dev/null`], [HAVE_SDL2_STATIC=no]))
     AC_MSG_RESULT([$HAVE_SDL2_STATIC])

     if test "x$HAVE_SDL2_STATIC" = "xno"; then
        SDL_LIBS=`sdl2-config --libs`
     else
        SDL_LIBS=`sdl2-config --static-libs`
     fi

     dnl backlinking support for modules
     PACKAGE_LIBS="${PACKAGE_LIBS} -ldl -no-pie"
     PACKAGE_LIBS="${PACKAGE_LIBS} ${FONTCONFIG_LIBS}"

     PACKAGE_LIBS="-static-libgcc ${PACKAGE_LIBS} ${SDL_LIBS} `pkg-config freetype2 --libs`"
   esac

   PACKAGE_CFLAGS="${PACKAGE_CFLAGS} `sdl2-config --cflags` `pkg-config freetype2 --cflags` -fno-exceptions"
   CXXFLAGS="${CXXFLAGS} -fno-rtti -std=c++14"

   dnl preconfigured values for SDL build
   AC_DEFINE_UNQUOTED([_SDL_VERSION], ["`sdl2-config --version` "], [SDL library version])
   AC_DEFINE(_SDL, 1, [Defined when building SDL version])
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/common src/platform/sdl"
   AC_SUBST(BUILD_SUBDIRS)
   (cd src/platform/android/app/src/main/assets && xxd -i main.bas > ../../../../../../../src/platform/sdl/main_bas.h)
}

function buildAndroid() {
   defaultConditionals

   AC_MSG_CHECKING([if library mode is enabled])
   AC_ARG_WITH(library,
   [  --with-library            Build the android as a library default=no],
       [with_library=$with_library],
       [with_library=no])
   AC_MSG_RESULT([$with_library])
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_ANDROID, 1, [Defined for Android build.])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/platform/android"
   AC_SUBST(BUILD_SUBDIRS)

   TEST_DIR="src/platform/android"
   AC_SUBST(TEST_DIR)
   if test $with_library = no; then
      (cd src/platform/android/webui && npm run build)
      TARGET="Building for Android."
   else
      TARGET="Building for Android library."
      AC_DEFINE(_ANDROID_LIBRARY, 1, [android library build enabled])
   fi
}

function buildConsole() {
   win32=no
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
        win32="yes"
   esac

   AC_C_BIGENDIAN(
     AC_DEFINE(CPU_BIGENDIAN, 1, [machine is big-endian]),
     AC_DEFINE(CPU_LITTLE_ENDIAN, 1, [machine is little-endian]),
     AC_MSG_ERROR(unknown endianess),
     AC_MSG_ERROR(universial endianess not supported)
   )

   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, test $win32 = yes)

   if test $win32 = yes; then
      dnl test whether to build using mingw
      AC_MSG_CHECKING(for standard cygwin gcc PATH)
      have_cygwin=no
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include "sys/cygwin.h"
        ]], [[
        ]])],[
        have_cygwin=yes
      ],[])
      AC_MSG_RESULT($have_cygwin)
      if test x$have_cygwin = xyes; then
         dnl preconfigured values for cygwin console build (depends on cygwin.dll)
         TARGET="Building Cygwin console version."
         AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
         AC_DEFINE(_CygWin, 1, [Building a minimal dev_null based command-line version.])
         AC_DEFINE(__CYGWIN__, 1, [as above])
         AC_DEFINE(INET_UNSUP, 1, [inet not supported.])
         PACKAGE_LIBS="${PACKAGE_LIBS} -lm -lgcc"
         BUILD_SUBDIRS="src/common src/platform/console"
      else
         dnl preconfigured values for cygwin mingw console build
         TARGET="Building Cygwin MinGW console version."
         AC_DEFINE(__MINGW32__, 1, [as above])
         AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
         PACKAGE_LIBS="${PACKAGE_LIBS} -Wl,-Bstatic -mconsole -lwsock32 -lws2_32 -static-libgcc"
         BUILD_SUBDIRS="src/common src/platform/console"
      fi
      AC_DEFINE(_Win32, 1, [Windows build])
   else
      dnl preconfigured values for unix console build
      TARGET="Building Unix console version."
      AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
      PACKAGE_LIBS="${PACKAGE_LIBS} -lm -ldl -lpthread"
      BUILD_SUBDIRS="src/common src/platform/console"
      TEST_DIR="src/platform/console"
      AC_SUBST(TEST_DIR)
   fi

   AC_DEFINE(_CONSOLE, 1, [Defined for console build.])
   AC_SUBST(BUILD_SUBDIRS)
}

function buildWeb() {
   if test $host_os = 'mingw32'; then
     TARGET="Building Ming32 Web server version."
     AC_DEFINE(__MINGW32__, 1, [as above])
     AC_DEFINE(_Win32, 1, [Windows build])
     PACKAGE_CFLAGS="-I$prefix/include ${PACKAGE_CFLAGS} -mms-bitfields -D__USE_MINGW_ANSI_STDIO"
     PACKAGE_LIBS="${PACKAGE_LIBS} -L$prefix/lib -Wl,-Bstatic -mconsole -lmingw32 -lwsock32 "
     PACKAGE_LIBS="${PACKAGE_LIBS} -lmicrohttpd -lws2_32 -static-libgcc -static-libstdc++"
   else
     TARGET="Building Web server version."
     PACKAGE_LIBS="${PACKAGE_LIBS} -lm -ldl -lpthread -lmicrohttpd"
     AC_CHECK_HEADERS([microhttpd.h], [], [AC_MSG_ERROR([microhttpd is not installed])])
   fi
   BUILD_SUBDIRS="src/common src/platform/web"
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(USE_TERM_IO, 0, [dont use the termios library.])
   AC_DEFINE(IMPL_DEV_ENV, 1, [Driver implements dev_env funcs])
   AC_SUBST(BUILD_SUBDIRS)
}

function buildEmscripten() {
   TARGET="Building Emscripten version."
   BUILD_SUBDIRS="src/common src/platform/emcc"
   AC_CHECK_PROG(have_xxd, xxd, [yes], [no])
   if test "${have_xxd}" = "no" ; then
     AC_MSG_ERROR([xxd command not installed: configure failed.])
   fi
   AC_CHECK_HEADERS([emscripten.h], [], [AC_MSG_ERROR([emscripten is not installed])])
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_EMCC, 1, [Defined when building emscripten version])
   AC_DEFINE(USE_TERM_IO, 0, [dont use the termios library.])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_SUBST(BUILD_SUBDIRS)
   (cd src/platform/android/app/src/main/assets && xxd -i main.bas > ../../../../../../../src/platform/emcc/main_bas.h)
}

function buildTEENSY() {
   TARGET="Building teensy version."
   BUILD_SUBDIRS="src/platform/teensy"
   AC_CHECK_PROG(have_xxd, xxd, [yes], [no])
   if test "${have_xxd}" = "no" ; then
     AC_MSG_ERROR([xxd command not installed: configure failed.])
   fi
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_MCU, 1, [Micro controller based builds])
   AC_DEFINE(USE_TERM_IO, 0, [dont use the termios library.])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_SUBST(BUILD_SUBDIRS)
   (cd src/platform/teensy && xxd -i main.bas > src/main_bas.h)
}

function buildFLTK() {
   TARGET="Building FLTK version."

   dnl Checks for FLTK 1.x
   AC_CHECK_PROG(have_fltk, fltk-config, [yes], [no])

   AC_CHECK_PROG(have_xxd, xxd, [yes], [no])
   if test "${have_xxd}" = "no" ; then
     AC_MSG_ERROR([xxd command not installed: configure failed.])
   fi

   dnl avoid using MSCRT versions of printf for long double
   case "${host_os}" in
     *mingw* | cygwin*)
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO"
   esac

   FLTK_CXXFLAGS="${PACKAGE_CFLAGS} `fltk-config --cxxflags`"
   FLTK_CXXFLAGS="${FLTK_CXXFLAGS} -fno-exceptions -fno-rtti -std=c++14 -Wno-unknown-pragmas"
   PACKAGE_LIBS="${PACKAGE_LIBS} `fltk-config --ldstaticflags --use-images`"

   dnl do not depend on cygwin.dll under cygwin build
   case "${host_os}" in
     *mingw* | cygwin*)
     FLTK_CXXFLAGS="${FLTK_CXXFLAGS} -mms-bitfields"
     PACKAGE_LIBS="-Wl,-Bstatic ${PACKAGE_LIBS} -lwsock32 -lws2_32 -static-libgcc -static-libstdc++"
     AC_DEFINE(_Win32, 1, [Windows build])
     AC_DEFINE(_USE_MATH_DEFINES, 1, [for M_PI in math.h])
     AC_DEFINE(realpath(F, R), _fullpath(R, F, PATH_MAX), [add missing realpath for system.cpp])
     ;;

     *darwin*)
     (cd images && xxd -i sb-desktop-128x128.png > ../src/platform/fltk/icon.h)
     ;;

     *)
     (cd images && xxd -i sb-desktop-128x128.png > ../src/platform/fltk/icon.h)
     PACKAGE_LIBS="${PACKAGE_LIBS} -no-pie"
     ;;
   esac

   defaultConditionals

   dnl preconfigured values for FLTK build
   AC_DEFINE_UNQUOTED([_FLTK_VERSION], ["`fltk-config --version` "], [FLTK library version])
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_FLTK, 1, [Defined for FLTK build.])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/common src/platform/fltk"
   AC_SUBST(BUILD_SUBDIRS)
   AC_SUBST(FLTK_CXXFLAGS)

   desktopentrydir='$(datarootdir)'/applications
   AC_SUBST(desktopentrydir)

   dnl generate kwp.h
   (cd src/platform/fltk && g++ -o build_kwp build_kwp.cxx && ./build_kwp)
}

if test x$ac_build_sdl = xyes; then
   buildSDL
elif test x$ac_build_android = xyes; then
   buildAndroid
elif test x$ac_build_web = xyes; then
   buildWeb
elif test x$ac_build_fltk = xyes; then
   buildFLTK
elif test x$ac_build_emcc = xyes; then
   buildEmscripten
elif test x$ac_build_teensy = xyes; then
   buildTEENSY
else
   buildConsole
fi

(cd documentation && g++ -o build_kwp build_kwp.cpp && ./build_kwp > ../src/ui/kwp.h)

checkPCRE
checkTermios
checkDebugMode
checkProfiling
checkForWindows

CFLAGS="${CFLAGS} -std=gnu11"
PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -Wall -Wno-unused-result"
BUILD_DATE=`date +"%a, %d %b %Y"`
AC_DEFINE_UNQUOTED([BUILD_DATE],["$BUILD_DATE"],[Build date])

SB_DWORD_VER=`awk -F "." '{printf "0x%02d%02d%02d", $1,$2,$3}' <<< ${PACKAGE_VERSION}`
AC_DEFINE_UNQUOTED([SB_DWORD_VER],[$SB_DWORD_VER],[SB_DWORD_VER])

AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

dnl change default aru setting to avoid warning
ARFLAGS=cr
AC_SUBST(ARFLAGS)

dnl check missing functions
AC_CHECK_FUNC([strlcpy], [AC_DEFINE([HAVE_STRLCPY], [1], [Define if strlcpy exists.])])
AC_CHECK_FUNC([strlcat], [AC_DEFINE([HAVE_STRLCAT], [1], [Define if strlcat exists.])])

AC_CONFIG_FILES([
Makefile
src/common/Makefile
src/platform/android/Makefile
src/platform/console/Makefile
src/platform/emcc/Makefile
src/platform/sdl/Makefile
src/platform/web/Makefile
src/platform/fltk/Makefile
src/platform/teensy/Makefile
])
AC_OUTPUT

echo
echo ${TARGET}
echo "CFLAGS=${CFLAGS}"
echo "CXXFLAGS=${CXXFLAGS}"
echo "PACKAGE_LIBS=${PACKAGE_LIBS}"

if test x$ac_build_sdl = xyes; then
   echo
   echo "sdl2: `sdl2-config --version`"
   echo "freetype: `pkg-config --version freetype2`"
fi

echo

if test x$ac_build_dist = xyes; then
   make dist
fi

dnl End of configure.ac
