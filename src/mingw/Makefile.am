# SmallBASIC command line version
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = -I../ -I. -I/usr/include @PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

sbasic_SOURCES =				\
  ../console_main.c				\
  ../dev_null.c

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/src -lsb_common -lwsock32

sbasic_DEPENDENCIES = ../libsb_common.a