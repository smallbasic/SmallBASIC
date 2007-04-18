# SmallBASIC for SDL
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = -I../ -I. @PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

sbasic_SOURCES = ../console_main.c dev_sdl.c

sbasic_LDADD = -L$(top_srcdir)/sync -lsb_common @PACKAGE_LIBS@
