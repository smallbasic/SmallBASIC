dnl 
dnl "$Id: configure.in 604 2008-05-21 11:47:15Z zeeb90au $"
dnl 
dnl Configure script for SmallBASIC
dnl
dnl Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
dnl
dnl This program is distributed under the terms of the GPL v2.0
dnl Download the GNU Public License (GPL) from www.gnu.org
dnl

AC_INIT([smallbasic], [0.12.13])
AC_CONFIG_AUX_DIR(aux-dist)
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([mysql/mysql.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
LT_INIT([win32-dll])

# Checks for header files.
AC_CHECK_HEADER(mysql/mysql.h, [], [AC_MSG_ERROR("install mysql")])

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
LT_PREREQ([2.2])

BUILD_SUBDIRS="mysql example"
AC_SUBST(BUILD_SUBDIRS)

libdir=${libdir}/smallbasic

AC_CONFIG_FILES([
Makefile
example/Makefile
mysql/Makefile])

AC_OUTPUT
