# SmallBASIC for FLTK
# Copyright(C) 2001-2008 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

INCLUDES =							\
  -I../ -I.							\
  -DPACKAGE_PREFIX=\""$(prefix)"\"				\
  -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"			\
  -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
  @FLTK_CXXFLAGS@

EXTRA_DIST =					\
    README.TXT					\
    sbfltk.rc					\
    $(desktopentry_DATA)

bin_PROGRAMS = sbasici

sbasici_SOURCES =				\
    HelpWidget.cxx HelpWidget.h			\
    MainWindow.cxx MainWindow.h			\
    EditorWindow.cxx EditorWindow.h		\
    dev_fltk.cxx				\
    StringLib.cxx StringLib.h			\
    blib_fltk_ui.cxx

sbasici_LDADD = -L$(top_srcdir)/src -lsb_common @PACKAGE_LIBS@

sbasici_DEPENDENCIES = ../libsb_common.a

iconsdir = $(datadir)/icons/hicolor/48x48/apps
icons_DATA = ../../images/smallbasic.png
desktopentry_DATA = smallbasic.desktop

if WITH_WIN32
sbasici_LDADD += sbfltk.res
sbfltk.res : sbfltk.rc
	$(WINDRES) $< -O coff -o $@
endif
