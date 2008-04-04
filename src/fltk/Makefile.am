# SmallBASIC for FLTK
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES =							\
     -I../ -I.							\
     -DPACKAGE_PREFIX=\""$(prefix)"\"				\
     -DPACKAGE_DATA_DIR=\""$(datadir)"\"			\
     -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
     @PACKAGE_CFLAGS@

EXTRA_DIST =					\
    README.TXT					\
    sbfltk.rc		\

bin_PROGRAMS = sbasici

sbasici_SOURCES =				\
    Fl_Help_Widget.cpp HelpWidget.h		\
    MainWindow.cpp MainWindow.h			\
    EditorWindow.cpp EditorWindow.h		\
    dev_fltk.cpp				\
    StringLib.cpp StringLib.h			\
    blib_fltk_ui.cpp

sbasici_LDADD = -L$(top_srcdir)/src -lsb_common @PACKAGE_LIBS@

if WITH_WIN32
sbasici_LDADD += sbfltk.res
sbfltk.res : sbfltk.rc
	$(WINDRES) $< -O coff -o $@
endif
