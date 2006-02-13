# SmallBASIC command line version
# Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = \
     -I../ -I. \
	-DPACKAGE_PREFIX=\""$(prefix)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = sbasic

sbasic_SOURCES =								\
  ../console_main.c								\
  ../dev_null.c									\
  ../dev_term.c ../dev_term.h

sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/sync -lsb_common

