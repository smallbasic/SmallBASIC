# SmallBASIC command line version
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I$(top_builddir)/src -I. @PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

sbasic_SOURCES =						\
  ../unix/console_main.c		\
  ../unix/dev_null.c

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/src/common -lsb_common -lwsock32

sbasic_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a
