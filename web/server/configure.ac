dnl
dnl Configure script for SmallBASIC
dnl
dnl Copyright(C) 2001-2016 Chris Warren-Smith.
dnl
dnl This program is distributed under the terms of the GPL v2.0
dnl Download the GNU Public License (GPL) from www.gnu.org
dnl

AC_INIT([smallbasic], [0.12.7])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIRS([m4])

AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX

LT_INIT

AM_CONDITIONAL(WITH_CYGWIN_CONSOLE, false)
AC_DEFINE(_UnixOS, 1, [Building under Unix like systems.])
AC_DEFINE(DEV_EVENTS_OSD, 0, [dev_events() implemented using osd_events().])
AC_DEFINE(BUILD_CONSOLE, 1, [Building a console based system.])
AC_DEFINE(IMPL_LOG_WRITE, 1, [Driver implements lwrite()])

PACKAGE_CFLAGS="-Wall -Wno-unused-result -fPIC"
AC_SUBST(PACKAGE_CFLAGS)

BUILD_DATE=`date +"%a, %d %b %Y"`
AC_DEFINE_UNQUOTED([BUILD_DATE],["$BUILD_DATE"],[Build date])
SB_DWORD_VER=`awk -F "." '{printf "0x%02d%02d%02d", $1,$2,$3}' <<< ${PACKAGE_VERSION}`
AC_DEFINE_UNQUOTED([SB_DWORD_VER],[$SB_DWORD_VER],[SB_DWORD_VER])

AC_CONFIG_FILES([
Makefile
src/common/Makefile
])

AC_OUTPUT
