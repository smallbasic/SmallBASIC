# SmallBASIC for FLTK
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

EXTRA_DIST =					\
    README.TXT					\
    sbfltk.rc  					\
    release/readme.html				\
    release/sbasic.nsi				\
    release/welcome.bas				\
    release/Bas-Home/calc.bas			\
    release/Bas-Home/calc.html			\
    release/Bas-Home/comment_in.bas		\
    release/Bas-Home/comment_out.bas		\
    release/Bas-Home/CVS			\
    release/Bas-Home/dos2unix.bas		\
    release/Bas-Home/image_demo.bas		\
    release/Bas-Home/indent.bas			\
    release/Bas-Home/logo.gif			\
    release/Bas-Home/memoryTest.bas		\
    release/Bas-Home/recent_files.bas		\
    release/Bas-Home/snarf.bas			\
    release/Bas-Home/sudoku.bas			\
    release/Bas-Home/_survey.bas

bin_PROGRAMS = sbfltk

sbfltk_SOURCES =				\
    Fl_Help_Widget.cpp HelpWidget.h             \
    MainWindow.cpp MainWindow.h                 \
    EditorWindow.cpp EditorWindow.h             \
    dev_fltk.cpp                                \
    StringLib.cpp StringLib.h                   \
    blib_fltk_ui.cpp

sbfltk_LDADD = -L$(top_srcdir)/sync -lsb_common @PACKAGE_LIBS@

if WITH_WIN32
sbfltk_LDADD += sbfltk.res
sbfltk.res : sbfltk.rc
	$(WINDRES) $< -O coff -o $@
endif
