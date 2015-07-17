dnl
dnl Configure script for SmallBASIC
dnl
dnl Copyright(C) 2001-2014 Chris Warren-Smith.
dnl
dnl This program is distributed under the terms of the GPL v2.0
dnl Download the GNU Public License (GPL) from www.gnu.org
dnl

AC_INIT([smallbasic], [0.11.19])
AC_CONFIG_SRCDIR([configure.ac])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_PROG_CXX
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

TARGET=""

dnl define build arguments
AC_ARG_ENABLE(fltk,
  AS_HELP_STRING([--enable-fltk],[build fltk version(default=no)]),
  [ac_build_fltk="yes"],
  [ac_build_fltk="no"])

AC_ARG_ENABLE(sdl,
  AS_HELP_STRING([--enable-sdl],[build SDL version(default=no)]),
  [ac_build_sdl="yes"],
  [ac_build_sdl="no"])

AC_ARG_ENABLE(tizen,
  AS_HELP_STRING([--enable-tizen],[build common library for Tizen(default=no)]),
  [ac_build_tizen="yes"],
  [ac_build_tizen="no"])

AC_ARG_ENABLE(android,
  AS_HELP_STRING([--enable-android],[build common library for Android(default=no)]),
  [ac_build_android="yes"],
  [ac_build_android="no"])

AC_ARG_ENABLE(dist,
  AS_HELP_STRING([--enable-dist],[prepare to run make dist(default=no)]),
  [ac_build_dist="yes"],
  [ac_build_dist="no"])

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
     dnl backlinking support for modules
     LDFLAGS="${LDFLAGS} -export-dynamic"
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
     CFLAGS="${CFLAGS} -g -O0"
     CXXFLAGS="${CXXFLAGS} -g -O0"
     AC_DEFINE(_DEBUG, 1, [debugging build enabled])
   fi
   AC_SUBST(CFLAGS)
}

function checkPCRE() {
   AC_CHECK_PROG(have_pcre, pcre-config, [yes], [no])

   dnl supported under linux only for now
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
     have_pcre="no"
   esac

   if test x$ac_build_tizen = xyes; then
     have_pcre="no"
   fi
   if test x$ac_build_android = xyes; then
     have_pcre="no"
   fi

   if test "${have_pcre}" = "yes" ; then
     AC_DEFINE(USE_PCRE, 1, [match.c used with libpcre.])
     PACKAGE_LIBS="${PACKAGE_LIBS} `pcre-config --libs`"
   fi
}

function defaultConditionals() {
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
}

function buildFLTK() {
   TARGET="Building FLTK version."

   dnl Checks for FLTK 2.x
   AC_CHECK_PROG(have_fltk, fltk2-config, [yes], [no])

   dnl do we HAVE the needed FLTK API VERSION ?
   HAVE_API_VERSION=no

   if test "${have_fltk}" = "yes" ; then
     FLTK_API_VERSION="`fltk2-config --api-version`"
     if test "${FLTK_API_VERSION}" = "2.0" ; then
       HAVE_API_VERSION=yes
     fi
   fi

   if test "${HAVE_API_VERSION}" = "no" ; then
       AC_MSG_ERROR([
     You need FLTK version 2.* installed with
     the program fltk2-config in your PATH in order to build sbfltk
     See the FLTK homepage at http://www.fltk.org/.

     configure failed.
   ])
   fi

   dnl avoid using MSCRT versions of printf for long double
   case "${host_os}" in
     *mingw* | cygwin*)
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO"
   esac

   FLTK_CXXFLAGS="${PACKAGE_CFLAGS} `fltk2-config --cxxflags` -fno-exceptions"
   PACKAGE_LIBS="${PACKAGE_LIBS} `fltk2-config --ldflags --use-images`"

   dnl do not depend on cygwin.dll under cygwin build
   case "${host_os}" in
     *mingw* | cygwin*)
     FLTK_CXXFLAGS="-I`cygpath -m /usr/local/include` ${FLTK_CXXFLAGS} -mms-bitfields"
     PACKAGE_LIBS="-L`cygpath -m /usr/local/lib` ${PACKAGE_LIBS} -lwsock32 -static-libgcc"
     AC_DEFINE(_Win32, 1, [Windows build])
   esac

   defaultConditionals

   dnl check for sdl-config for sound output
   AC_CHECK_PROG(have_sdl, sdl-config, [yes], [no])
   if test "${have_sdl}" = "yes" ; then
     FLTK_CXXFLAGS="${FLTK_CXXFLAGS} `sdl-config --cflags`"
     PACKAGE_LIBS="${PACKAGE_LIBS} `sdl-config --libs`"
     AC_DEFINE(HAVE_SDL, 1, [Windows build])
   fi

   dnl preconfigured values for FLTK build
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_FLTK, 1, [Defined for FLTK build.])
   AC_DEFINE(USE_CLIB, 1, [Implements clibmgr.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_DEV_ENV, 1, [Driver implements dev_env funcs])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/common src/platform/fltk"
   AC_SUBST(BUILD_SUBDIRS)
   AC_SUBST(FLTK_CXXFLAGS)

   desktopentrydir='$(datarootdir)'/applications
   AC_SUBST(desktopentrydir)

   dnl generate kwp.h
   (cd src/platform/fltk && g++ -o build_kwp build_kwp.cxx && ./build_kwp)
}

function buildSDL() {
   TARGET="Building SDL version."
   defaultConditionals

   AC_CHECK_PROG(have_sdl, sdl2-config, [yes], [no])
   if test "${have_sdl}" = "no" ; then
     AC_MSG_ERROR([libsdl2-dev not installed: configure failed.])
   fi

   AC_CHECK_PROG(have_freetype2, freetype-config, [yes], [no])
   if test "${have_freetype2}" = "no" ; then
     AC_MSG_ERROR([libfreetype6-dev not installed: configure failed.])
   fi

   case "${host_os}" in
     *mingw* | cygwin*)
     dnl avoid using MSCRT versions of printf for long double
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO"

     dnl do not depend on cygwin.dll under cygwin build
     PACKAGE_CFLAGS="-I`cygpath -m /usr/local/include` ${PACKAGE_CFLAGS} -mms-bitfields"
     PACKAGE_LIBS="-L`cygpath -m /usr/local/lib` ${PACKAGE_LIBS} -lwsock32 -lws2_32 -static-libgcc -static-libstdc++"
     AC_DEFINE(_Win32, 1, [Windows build])
     ;;

     *)
     # Unix - Check fontconfig configuration
     PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.2)
     AC_SUBST(FONTCONFIG_CFLAGS)
     AC_SUBST(FONTCONFIG_LIBS)

     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} ${FONTCONFIG_CFLAGS}"

     dnl backlinking support for modules
     PACKAGE_LIBS="${PACKAGE_LIBS} -ldl"
     PACKAGE_LIBS="${PACKAGE_LIBS} ${FONTCONFIG_LIBS}"
   esac

   PACKAGE_CFLAGS="${PACKAGE_CFLAGS} `sdl2-config --cflags` `freetype-config --cflags`"
   case "${host_os}" in
     *mingw*)
     PACKAGE_LIBS="-Wl,-Bstatic ${PACKAGE_LIBS} `sdl2-config --static-libs` `freetype-config --libs`"
     ;;

     *)
      PACKAGE_LIBS="${PACKAGE_LIBS} `sdl2-config --libs` `freetype-config --libs`"
    esac

   dnl preconfigured values for SDL build
   AC_DEFINE(_SDL, 1, [Defined when building SDL version])
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/common src/platform/sdl"
   AC_SUBST(BUILD_SUBDIRS)
   (cd ide/android/assets && xxd -i main.bas > ../../../src/platform/sdl/main_bas.h)
   (cd documentation && g++ -o build_kwp build_kwp.cpp && ./build_kwp > ../src/ui/kwp.h)
}

function buildTizen() {
   TARGET="Building common library for Tizen."

   defaultConditionals

   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_TIZEN, 1, [Defined for Tizen build.])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(CPU_BIGENDIAN, 1, [Tizen uses big-endian])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/platform/tizen/common"
   AC_SUBST(BUILD_SUBDIRS)
}

function buildAndroid() {
   TARGET="Building for Android."

   defaultConditionals

   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_ANDROID, 1, [Defined for Android build.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(CPU_BIGENDIAN, 1, [Android uses big-endian])

   BUILD_SUBDIRS="src/platform/android"
   AC_SUBST(BUILD_SUBDIRS)

   TEST_DIR="src/platform/android"
   AC_SUBST(TEST_DIR)

   (cd documentation && g++ -o build_kwp build_kwp.cpp && ./build_kwp > ../src/ui/kwp.h)
}

function buildConsole() {
   win32=no
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
        win32="yes"
   esac

   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, test $win32 = yes)
   AC_DEFINE(BUILD_CONSOLE, 1, [Building a console based system.])

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
         AC_DEFINE(DEV_EVENTS_OSD, 0, [dev_events() implemented using osd_events().])
         AC_DEFINE(_CygWin, 1, [Building a minimal dev_null based command-line version.])
         AC_DEFINE(__CYGWIN__, 1, [as above])
         AC_DEFINE(INET_UNSUP, 1, [inet not supported.])
         PACKAGE_LIBS="${PACKAGE_LIBS} -lm -lgcc"
         BUILD_SUBDIRS="src/common src/platform/unix"
      else
         dnl preconfigured values for cygwin mingw console build
         TARGET="Building Cygwin MinGW console version."
         AC_DEFINE(__MINGW32__, 1, [as above])
         AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
         PACKAGE_LIBS="${PACKAGE_LIBS} -mconsole -lmingw32 -lwsock32"
         BUILD_SUBDIRS="src/common src/platform/unix"
      fi
      AC_DEFINE(_Win32, 1, [Windows build])
   else
      dnl preconfigured values for unix console build
      TARGET="Building Unix console version."
      AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
      AC_DEFINE(USE_TERM_IO, 0, [Uses terminal-io functions.])
      AC_DEFINE(DEV_EVENTS_OSD, 0, [dev_events() implemented using osd_events().])
      PACKAGE_LIBS="${PACKAGE_LIBS} -lm -ldl -lpthread"
      BUILD_SUBDIRS="src/common src/platform/unix"
      TEST_DIR="src/platform/unix"
      AC_SUBST(TEST_DIR)
   fi

   AC_SUBST(BUILD_SUBDIRS)
}

function buildDist() {
   TARGET="Building source code release."
   defaultConditionals
   BUILD_SUBDIRS="src/common src/platform/gtk/src src/platform/gtk/data src/platform/gtk/icons"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/fltk"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/sdl"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/unix"
   AC_SUBST(BUILD_SUBDIRS)
}

if test x$ac_build_dist = xyes; then
   buildDist
elif test x$ac_build_fltk = xyes; then
   buildFLTK
elif test x$ac_build_sdl = xyes; then
   buildSDL
elif test x$ac_build_tizen = xyes; then
   buildTizen
elif test x$ac_build_android = xyes; then
   buildAndroid
else
   buildConsole
fi

checkPCRE
checkDebugMode
checkForWindows

PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -Wall -Wno-unused-result"

AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

AC_CONFIG_FILES([
Makefile
src/common/Makefile
src/platform/fltk/Makefile
src/platform/sdl/Makefile
src/platform/unix/Makefile
src/platform/tizen/common/Makefile
src/platform/android/Makefile
])
AC_OUTPUT

echo
echo ${TARGET}
echo "CFLAGS=${CFLAGS}"
echo "CXXFLAGS=${CXXFLAGS}"
echo "PACKAGE_LIBS=${PACKAGE_LIBS}"
echo

if test x$ac_build_dist = xyes; then
   make dist
fi

dnl End of configure.ac
