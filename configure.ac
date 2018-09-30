dnl
dnl Configure script for SmallBASIC
dnl
dnl Copyright(C) 2001-2018 Chris Warren-Smith.
dnl
dnl This program is distributed under the terms of the GPL v2.0
dnl Download the GNU Public License (GPL) from www.gnu.org
dnl

AC_INIT([smallbasic], [0.12.14])
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

   AC_CHECK_PROG(have_freetype2, freetype-config, [yes], [no])
   if test "${have_freetype2}" = "no" ; then
     AC_MSG_ERROR([libfreetype6-dev not installed: configure failed.])
   fi

   case "${host_os}" in
     *mingw* | cygwin*)
     dnl avoid using MSCRT versions of printf for long double
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO"

     dnl do not depend on cygwin.dll under cygwin build
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -mms-bitfields"
     PACKAGE_LIBS="${PACKAGE_LIBS} -lwsock32 -lws2_32 -static-libgcc -static-libstdc++"
     PACKAGE_LIBS="${PACKAGE_LIBS} -Wl,-Bstatic -Wl,-Map=output.map"
     PACKAGE_LIBS="${PACKAGE_LIBS} `sdl2-config --static-libs` `freetype-config --libs`"
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
     PACKAGE_LIBS="${PACKAGE_LIBS} `sdl2-config --libs` `freetype-config --libs`"
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
     PACKAGE_LIBS="-static-libgcc ${PACKAGE_LIBS} `sdl2-config --static-libs` `freetype-config --libs`"
   esac

   PACKAGE_CFLAGS="${PACKAGE_CFLAGS} `sdl2-config --cflags` `freetype-config --cflags` -fno-exceptions"
   CXXFLAGS="${CXXFLAGS} -fno-rtti -std=c++11"

   dnl preconfigured values for SDL build
   AC_DEFINE(_SDL, 1, [Defined when building SDL version])
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

   BUILD_SUBDIRS="src/common src/platform/sdl"
   AC_SUBST(BUILD_SUBDIRS)
   (cd src/platform/android/app/src/main/assets && xxd -i main.bas > ../../../../../../../src/platform/sdl/main_bas.h)
   (cd documentation && g++ -o build_kwp build_kwp.cpp && ./build_kwp > ../src/ui/kwp.h)
}

function buildAndroid() {
   TARGET="Building for Android."

   defaultConditionals

   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_ANDROID, 1, [Defined for Android build.])
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

   AC_C_BIGENDIAN(
     AC_DEFINE(CPU_BIGENDIAN, 1, [machine is big-endian]),
     AC_DEFINE(CPU_LITTLE_ENDIAN, 1, [machine is little-endian]),
     AC_MSG_ERROR(unknown endianess),
     AC_MSG_ERROR(universial endianess not supported)
   )

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
         PACKAGE_LIBS="${PACKAGE_LIBS} -mconsole -lmingw32 -lwsock32 -lws2_32 -static-libgcc"
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

   AC_SUBST(BUILD_SUBDIRS)

   (cd documentation && g++ -o build_kwp build_kwp.cpp && ./build_kwp > ../src/ui/kwp.h)
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
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(USE_TERM_IO, 0, [dont use the termios library.])
   AC_DEFINE(IMPL_DEV_ENV, 1, [Driver implements dev_env funcs])
   AC_SUBST(BUILD_SUBDIRS)
}

if test x$ac_build_sdl = xyes; then
   buildSDL
elif test x$ac_build_android = xyes; then
   buildAndroid
elif test x$ac_build_web = xyes; then
   buildWeb
else
   buildConsole
fi

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
src/platform/sdl/Makefile
src/platform/web/Makefile
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
