# SmallBASIC command line version
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = -I../ -I. -I/usr/include @PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

sbasic_SOURCES =																\
  ../unix/console_main.c												\
  ../unix/dev_null.c														\
  ../unix/dev_term.c ../unix/dev_term.h

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/src -lsb_common

sbasic_DEPENDENCIES = ../libsb_common.a
