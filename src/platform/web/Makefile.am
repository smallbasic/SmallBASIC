# SmallBASIC web server version
# Copyright(C) 2001-2016 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I$(top_builddir)/src -I. @PACKAGE_CFLAGS@
bin_PROGRAMS = sbasicw
sbasicw_SOURCES = main.cpp canvas.cpp ../../ui/strlib.cpp
sbasicw_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@
sbasicw_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a
