# SmallBASIC for FLTK
# Copyright(C) 2001-2006 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES = \
     -I../../ -I. \
	-DPACKAGE_PREFIX=\""$(prefix)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

AM_LDFLAGS = -L$(top_srcdir)/sync -lcommon

bin_PROGRAMS = sbfltk

sbfltk_SOURCES =								\
    Fl_Ansi_Window.cpp                          \
    Fl_Help_Widget.cpp                          \
    MainWindow.cpp                              \
    EditorWindow.cpp                            \
    dev_fltk.cpp                                \
    StringLib.cpp                               \
    blib_fltk_ui.cpp

sbfltk_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

