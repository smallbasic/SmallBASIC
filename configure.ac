dnl 
dnl Configure script for SmallBASIC
dnl
dnl Copyright(C) 2001-2014 Chris Warren-Smith.
dnl
dnl This program is distributed under the terms of the GPL v2.0
dnl Download the GNU Public License (GPL) from www.gnu.org
dnl

AC_INIT([smallbasic], [0.11.9])
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
AC_ARG_ENABLE(gtk,
  AS_HELP_STRING([--enable-gtk],[build gtk version(default=no)]),
  [ac_build_gtk="yes"],
  [ac_build_gtk="no"])

AC_ARG_ENABLE(fltk,
  AS_HELP_STRING([--enable-fltk],[build fltk version(default=no)]),
  [ac_build_fltk="yes"],
  [ac_build_fltk="no"])

AC_ARG_ENABLE(sdl,
  AS_HELP_STRING([--enable-sdl],[build SDL version(default=no)]),
  [ac_build_sdl="yes"],
  [ac_build_sdl="no"])

AC_ARG_ENABLE(qt,
  AS_HELP_STRING([--enable-qt],[build QT version(default=no)]),
  [ac_build_qt="yes"],
  [ac_build_qt="no"])

AC_ARG_ENABLE(mosync,
  AS_HELP_STRING([--enable-mosync],[build common library for Mosync(default=no)]),
  [ac_build_mosync="yes"],
  [ac_build_mosync="no"])

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
   esac
   AM_CONDITIONAL(WITH_WIN32, test x"$win32" = "xyes")

   dnl backlinking support for modules
   if test $win32 = no; then
     LDFLAGS="${LDFLAGS} -export-dynamic"
   fi
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
     CFLAGS="-g -O0"
     CXXFLAGS="-g -O0"
     AC_DEFINE(_DEBUG, 1, [debugging build enabled])
   else
     CFLAGS="-O3 -Os"
     CXXFLAGS="-O3 -Os"
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

   if test x$ac_build_mosync = xyes; then
     have_pcre="no"
   fi
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

function checkSearch() {
   have_saerch="maybe"
   AC_CHECK_HEADER(search.h, [have_search="yes"], [have_search="no"])
   if test x$have_search = xyes ; then
      AC_DEFINE(HAVE_SEARCH_H, 1, [Found GNU search.h])
   fi
}

function defaultConditionals() {
   AM_CONDITIONAL(WITH_HILDON, false)
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
}

function checkHILDON() {
   # copied from 
   # https://stage.maemo.org/svn/maemo/projects/tools/branches/maemopad/chinook/configure.ac
 
   # Hildon library dependsncies
   PKG_CHECK_MODULES(HILDON, hildon-1 hildon-fm-2 hildon-help conbtdialogs)
   AC_SUBST(HILDON_LIBS)
   AC_SUBST(HILDON_CFLAGS)
 
   # Only used for the .service file path
   PKG_CHECK_MODULES(DBUS, dbus-glib-1 >= 0.60) 
 
   # OSSO application framework dependencies
   PKG_CHECK_MODULES(OSSO, osso-af-settings >= 0.8.5 libosso >= 0.9.17
                     gnome-vfs-2.0 >= 2.8.4.11 gnome-vfs-module-2.0 >= 2.8.4.11)
   AC_SUBST(OSSO_LIBS)
   AC_SUBST(OSSO_CFLAGS)
 
   # To make application visible in maemo Task Navigator it needs a Desktop
   # file for the application.
   # D-BUS service file is needed to be able to launch the maemo application
   # and connect it to D-BUS services.
   # The following line defines install directories for these files.
   desktopentrydir=`$PKG_CONFIG osso-af-settings --variable=desktopentrydir`
   serviceentrydir=`$PKG_CONFIG osso-af-settings --variable=dbusservicedir`
 
   # Application locale install directory
   localedir=`$PKG_CONFIG osso-af-settings --variable=localedir`
 
   # Application pixmaps install directory
   pixmapdir=`$PKG_CONFIG osso-af-settings --variable=hildonpixmapdir`
 
   # Application icon install directories
   icon_26x26dir=$datadir/icons/hicolor/26x26/hildon
   icon_34x34dir=$datadir/icons/hicolor/34x34/hildon
   icon_40x40dir=$datadir/icons/hicolor/40x40/hildon
   icon_50x50dir=$datadir/icons/hicolor/50x50/hildon
   icon_scalabledir=$datadir/icons/hicolor/scalable/hildon
 
   # Hildon control panel plugin install directories
   pluginlibdir=`$PKG_CONFIG hildon-control-panel --variable=plugindir`
   plugindesktopentrydir=`$PKG_CONFIG hildon-control-panel --variable=plugindesktopentrydir`
 
   # Define as variables in Makefiles
   AC_SUBST(desktopentrydir)
   AC_SUBST(serviceentrydir)
   AC_SUBST(localedir)
   AC_SUBST(pixmapdir)
   AC_SUBST(icon_26x26dir)
   AC_SUBST(icon_34x34dir)
   AC_SUBST(icon_40x40dir)
   AC_SUBST(icon_50x50dir)
   AC_SUBST(icon_scalabledir)
   AC_SUBST(pluginlibdir)
   AC_SUBST(plugindesktopentrydir)
 
   AC_DEFINE_UNQUOTED([LOCALEDIR], "${localedir}", [Runtime locale catalog files path])
   AC_DEFINE_UNQUOTED([PIXMAPDIR], "${pixmapdir}", [Runtime pixmap files path])
 
   PACKAGE_CFLAGS="${PACKAGE_CFLAGS} ${HILDON_CFLAGS} ${OSSO_CFLAGS}"
   PACKAGE_LIBS="${PACKAGE_LIBS} ${HILDON_LIBS} ${OSSO_LIBS}"
   ln -s src/gtk/debian
}

function buildGTK() {
   TARGET="Building GTK version."
 
   dnl disable build for hildon
   AC_ARG_ENABLE(hildon,
     AS_HELP_STRING([--enable-hildon],[disable hildon build (default=yes)]),
     [ac_hildon_build="yes"],
     [ac_hildon_build="no"])

   AM_CONDITIONAL(WITH_HILDON, test "x$ac_hildon_build" = "xyes")
   AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)

   if test "x$ac_hildon_build" = "xyes"; then
      AC_DEFINE(USE_HILDON, 1, [Define if building for hildon/maemo/n770.])
      checkHILDON
   else
      dnl check for gtk libraries
      pkg_modules="gtk+-2.0 >= 2.0.0"
      PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])

      dnl Checks for glade-2
      AC_CHECK_PROG(have_glade, glade-2, [yes], [no])

      if test "${have_glade}" = "yes" ; then
         dnl build the user interface 
         glade-2 -w src/platform/gtk/sbgtk.glade
         AC_DEFINE(gtk_widget_unref, g_object_unref, [hack for glade-2.])
      else
         AC_MSG_ERROR([
           glade-2 not found: configure failed.
         ])
      fi

      dnl check for dbus
      PKG_CHECK_MODULES(DBUS, [dbus-1 >= 0.35 dbus-glib-1 >= 0.35], [
        PACKAGE_CFLAGS="${PACKAGE_CFLAGS} ${DBUS_CFLAGS}"
        PACKAGE_LIBS="${PACKAGE_LIBS} ${DBUS_LIBS}"
        AC_DEFINE(USE_DBUS, 1, [use dbus-glib])
      ])
   fi
 
   dnl preconfigured values for GTK build
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(KBHIT_PWR_CONSERVE, 1, [Conserve power in dev_kbhit()])
   AC_DEFINE(DRV_SOUND, 1, [Driver implements functions in drvsound.h])
   AC_DEFINE(IMPL_IMAGE, 1, [Driver implements image commands])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(GTK_DISABLE_DEPRECATED, 1, [avoid building obsolete code)])
   AC_DEFINE(HILDON_DISABLE_DEPRECATED, 1, [avoid building obsolete code)])
   AC_DEFINE(IMPL_HTML, 1, [Driver implements the html command])
   AC_DEFINE(OS_PREC64, 1, [64 bit variables])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(DRV_BEEP, 1, [Use the driver based beep function])

   BUILD_SUBDIRS="src/common src/platform/gtk/src src/platform/gtk/data src/platform/gtk/icons"
   AC_SUBST(BUILD_SUBDIRS)
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

   FLTK_CXXFLAGS="${PACKAGE_CFLAGS} `fltk2-config --cxxflags` -Wall -Wunused -fno-exceptions"
   PACKAGE_LIBS="${PACKAGE_LIBS} `fltk2-config --ldflags --use-images`"

   dnl do not depend on cygwin.dll under cygwin build
   case "${host_os}" in
     *mingw* | cygwin*)
     FLTK_CXXFLAGS="-I`cygpath -m /usr/local/include` ${FLTK_CXXFLAGS} -mms-bitfields"
     PACKAGE_LIBS="-L`cygpath -m /usr/local/lib` ${PACKAGE_LIBS} -lwsock32 -static-libgcc"
     AC_DEFINE(_Win32, 1, [Windows build])
     AC_DEFINE(__addr_t_defined, 1, [Avoid conflict in cygwin/types.h])
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
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])
   AC_DEFINE(USE_CLIB, 1, [Implements clibmgr.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_IMAGE, 1, [Driver implements image commands])
   AC_DEFINE(IMPL_HTML, 1, [Driver implements the html command])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_DEV_ENV, 1, [Driver implements dev_env funcs])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(OS_PREC64, 1, [64 bit variables])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(DRV_BEEP, 1, [Use the driver based beep function])
   AC_DEFINE(IMPL_OSD_SOUND, 1, [Driver implements osd_sound()]) 

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

   AC_MSG_CHECKING(for SDL_image)
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
     #include "SDL2/SDL_image.h"
     ]], [[]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_ERROR([libsdl2-image-dev not installed: configure failed])])

   # Check fontconfig configuration
   PKG_CHECK_MODULES(FONTCONFIG, fontconfig >= 2.2)
   AC_SUBST(FONTCONFIG_CFLAGS)
   AC_SUBST(FONTCONFIG_LIBS)

   PACKAGE_CFLAGS="${PACKAGE_CFLAGS} ${FONTCONFIG_CFLAGS} `sdl2-config --cflags` `freetype-config --cflags`"
   PACKAGE_LIBS="${PACKAGE_LIBS} ${FONTCONFIG_LIBS} `sdl2-config --libs` `freetype-config --libs` -ldl"

   dnl preconfigured values for SDL build
   AC_DEFINE(_SDL, 1, [Defined when building SDL version])
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_OSD_SOUND, 1, [Driver implements osd_sound()]) 
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(OS_PREC64, 1, [64 bit variables])
   AC_DEFINE(DRV_BEEP, 1, [Use the driver based beep function])

   BUILD_SUBDIRS="src/common src/platform/sdl"
   AC_SUBST(BUILD_SUBDIRS)
   (cd ide/android/assets && xxd -i main.bas > ../../../src/platform/sdl/main_bas.h)
}

function buildQT() {
   TARGET="Building QT version."
 
   dnl Checks for qmake
   AC_CHECK_PROG(have_qt, qmake, [yes], [no])
   if test "${have_qt}" = "no" ; then
     AC_MSG_ERROR([You need QT version 4.* configure failed.])
   fi
 
   dnl avoid using MSCRT versions of printf for long double
   case "${host_os}" in
     cygwin*)
     PACKAGE_CFLAGS="${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO"
   esac

   defaultConditionals
 
   dnl preconfigured values for QT build
   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_QT, 1, [Defined for QT build.])
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_IMAGE, 1, [Driver implements image commands])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_DEV_ENV, 1, [Driver implements dev_env funcs])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(OS_PREC64, 1, [64 bit variables])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(DRV_BEEP, 1, [Use the driver based beep function])
   AC_DEFINE(IMPL_OSD_SOUND, 1, [Driver implements osd_sound()])
   AC_DEFINE(IMPL_DEV_RUN, 1, [Driver implements dev_run()])
   AC_DEFINE(IMPL_DEV_CIRCLE, 1, [Driver implements circle/elipse funcs])

   BUILD_SUBDIRS="src/common src/platform/qt"
   AC_SUBST(BUILD_SUBDIRS)

   desktopentrydir='$(datarootdir)'/applications
   AC_SUBST(desktopentrydir)

   dnl generate qt project files
   (cd src/platform/qt && qmake -o Makefile.qt)
}

function buildMosync() {
   TARGET="Building common library for Mosync."

   defaultConditionals

   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(HAVE_SEARCH_H, 1, [Found GNU search.h])
   AC_DEFINE(HAVE_TDESTROY, 1, [Found GNU search.h])
   AC_DEFINE(INET_UNSUP, 1, [inet not supported.])
   AC_DEFINE(RUN_UNSUP, 1, [run not supported.])
   AC_DEFINE(SERIAL_UNSUP, 1, [serial not supported.])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_IMAGE, 1, [Driver implements image commands])
   AC_DEFINE(HAVE_MALLOC_USABLE_SIZE, 0, [Newlib version causes mosync stack panic])
   AC_DEFINE(CPU_BIGENDIAN, 1, [Mosync uses big-endian])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(NONSTANDARD_PORT, 1, [sys.h should include portdefs.h])
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])
   AC_DEFINE(_MOSYNC, 1, [Defined for Mosync build.])

   dnl Checks for mosync
   AC_CHECK_PROG(have_mosync, pipe-tool, [yes], [no])
 
   PACKAGE_CFLAGS="-MMD -S -DMAPIP -O2 -MF "

   dnl find out where mosync is installed
   win32=no
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
        win32="yes"
   esac
   if test $win32 = yes; then
     MOSYNC_HOME="`which xgcc | cygpath -m -f - | sed 's/\/bin\/xgcc//'`"
     EXTN=".exe"
   else
     dnl see: http://www.mosync.com/documentation/manualpages/building-mosync-source-linux
     MOSYNC_HOME=${MOSYNCDIR}
     EXTN=""
   fi
   AC_SUBST(MOSYNC_HOME)
   AC_SUBST(EXTN)

   BUILD_SUBDIRS="src/platform/mosync/common"
   AC_SUBST(BUILD_SUBDIRS)
}

function buildTizen() {
   TARGET="Building common library for Tizen."

   defaultConditionals

   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_TIZEN, 1, [Defined for Tizen build.])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(HAVE_SEARCH_H, 1, [Found GNU search.h])
   AC_DEFINE(HAVE_TDESTROY, 1, [Found GNU search.h])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_IMAGE, 1, [Driver implements image commands])
   AC_DEFINE(HAVE_MALLOC_USABLE_SIZE, 0, [Newlib version causes mosync stack panic])
   AC_DEFINE(CPU_BIGENDIAN, 1, [Tizen uses big-endian])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])

   BUILD_SUBDIRS="src/platform/tizen/common"
   AC_SUBST(BUILD_SUBDIRS)
}

function buildAndroid() {
   TARGET="Building common library for Android."

   defaultConditionals

   AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
   AC_DEFINE(_ANDROID, 1, [Defined for Android build.])
   AC_DEFINE(IMPL_DEV_READ, 1, [Implement dev_read()])
   AC_DEFINE(DEV_EVENTS_OSD, 1, [dev_events() implemented using osd_events().])
   AC_DEFINE(IMPL_DEV_GETS, 1, [Driver implements dev_gets()])
   AC_DEFINE(IMPL_DEV_DELAY, 1, [Driver implements dev_delay()])
   AC_DEFINE(IMPL_IMAGE, 1, [Driver implements image commands])
   AC_DEFINE(HAVE_MALLOC_USABLE_SIZE, 0, [Newlib version causes mosync stack panic])
   AC_DEFINE(CPU_BIGENDIAN, 1, [Android uses big-endian])
   AC_DEFINE(NO_SCAN_ERROR_PROMPT, 1, [No prompt for detailed scan report])
   AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])
   AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])

   BUILD_SUBDIRS="src/platform/android/common"
   AC_SUBST(BUILD_SUBDIRS)
}

function buildConsole() {
   win32=no
   case "${host_os}" in
     *mingw* | pw32* | cygwin*)
        win32="yes"
   esac

   AM_CONDITIONAL(WITH_HILDON, false)
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
         AC_CHECK_HEADER(ncurses/termcap.h, [], [AC_MSG_ERROR("install libncurses-dev")])
         AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
         AC_DEFINE(USE_TERM_IO, 1, [Uses terminal-io functions.])
         AC_DEFINE(DEV_EVENTS_OSD, 0, [dev_events() implemented using osd_events().])
         AC_DEFINE(_CygWin, 1, [Building a minimal dev_null based command-line version.])
         AC_DEFINE(__CYGWIN__, 1, [as above])
         AC_DEFINE(OS_PREC64, 1, [64 bit variables])
         AC_DEFINE(INET_UNSUP, 1, [inet not supported.])
         PACKAGE_LIBS="${PACKAGE_LIBS} -lm -lgcc -lncurses"
         BUILD_SUBDIRS="src/common src/platform/cygwin"
      else
         dnl preconfigured values for cygwin mingw console build
         TARGET="Building Cygwin MinGW console version."
         AC_DEFINE(__MINGW32__, 1, [as above])
         AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
         AC_DEFINE(USE_TERM_IO, 0, [Does not use terminal-io functions.])
         AC_DEFINE(IMPL_EMPTY_TERM_EVENTS, 1, [Implement a dummy term_events()])
         PACKAGE_LIBS="${PACKAGE_LIBS} -mconsole -lmingw32"
         BUILD_SUBDIRS="src/common src/platform/mingw"
      fi
      AC_DEFINE(_Win32, 1, [Windows build])
      AC_DEFINE(__addr_t_defined, 1, [Avoid conflict in cygwin/types.h])
   else 
      dnl preconfigured values for unix console build
      TARGET="Building Unix console version."
      AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
      AC_DEFINE(USE_TERM_IO, 1, [Uses terminal-io functions.])
      AC_DEFINE(DEV_EVENTS_OSD, 0, [dev_events() implemented using osd_events().])
      AC_DEFINE(DRV_SOUND, 1, [Driver implements functions in drvsound.h])
      AC_DEFINE(UNIX_MEMMGR, 1, [Unix memory manager])
      AC_DEFINE(OS_PREC64, 1, [64 bit variables])
      AC_CHECK_HEADER(readline/readline.h, [], [AC_MSG_ERROR("install libreadline-dev")])
      PACKAGE_LIBS="${PACKAGE_LIBS} -lm -ldl -lpthread -lncurses -lreadline"
      BUILD_SUBDIRS="src/common src/platform/unix"
   fi

   AC_SUBST(BUILD_SUBDIRS)
}

function buildDist() {
   TARGET="Building source code release."
   defaultConditionals
   BUILD_SUBDIRS="src/common src/platform/gtk/src src/platform/gtk/data src/platform/gtk/icons"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/fltk"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/sdl"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/cygwin"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/unix"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/mingw"
   BUILD_SUBDIRS="${BUILD_SUBDIRS} src/platform/qt"
   AC_SUBST(BUILD_SUBDIRS)
}

OS_NAME=`uname -a`

AC_DEFINE_UNQUOTED(OS_NAME, "${OS_NAME}", [SB system variable])
AC_DEFINE(HAVE_C_MALLOC, 1, [allocate memory with regular malloc calls])

if test x$ac_build_dist = xyes; then
   buildDist
elif test x$ac_build_gtk = xyes; then
   buildGTK
elif test x$ac_build_fltk = xyes; then
   buildFLTK
elif test x$ac_build_sdl = xyes; then
   buildSDL
elif test x$ac_build_qt = xyes; then
   buildQT
elif test x$ac_build_mosync = xyes; then
   buildMosync
elif test x$ac_build_tizen = xyes; then
   buildTizen
elif test x$ac_build_android = xyes; then
   buildAndroid
else
   buildConsole
fi  

checkSearch
checkPCRE
checkDebugMode
checkForWindows

AC_CONFIG_FILES([
Makefile
src/common/Makefile
src/platform/gtk/src/Makefile
src/platform/gtk/data/sbasic.desktop
src/platform/gtk/data/com.nokia.sbasic.service
src/platform/gtk/data/Makefile
src/platform/gtk/icons/Makefile
src/platform/fltk/Makefile
src/platform/sdl/Makefile
src/platform/qt/Makefile
src/platform/cygwin/Makefile
src/platform/mingw/Makefile
src/platform/unix/Makefile
src/platform/mosync/common/Makefile
src/platform/tizen/common/Makefile
src/platform/android/common/Makefile
])
AC_OUTPUT

echo
echo ${TARGET}
echo

if test x$ac_build_dist = xyes; then
   make dist
fi

dnl End of configure.ac
