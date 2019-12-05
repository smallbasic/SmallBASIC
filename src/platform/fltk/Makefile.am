# SmallBASIC for FLTK
# Copyright(C) 2001-2019 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS =                                                 \
  -I$(top_builddir)/src -I.                                   \
  -DPACKAGE_PREFIX=\""$(prefix)"\"                            \
  -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"                      \
  -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  \
  @PACKAGE_CFLAGS@ @FLTK_CXXFLAGS@

EXTRA_DIST =                      \
  $(desktopentry_DATA)

bin_PROGRAMS = sbasici

sbasici_SOURCES =                 \
  ../../ui/ansiwidget.cpp         \
  ../../ui/system.cpp             \
  ../../ui/window.cpp             \
  ../../ui/screen.cpp             \
  ../../ui/form.cpp               \
  ../../ui/inputs.cpp             \
  ../../ui/image.cpp              \
  ../../ui/strlib.cpp             \
  ../../ui/textedit.cpp           \
  ../../ui/audio.cpp              \
  display.cxx display.h           \
  runtime.cxx runtime.h           \
  HelpWidget.cxx HelpWidget.h     \
  HelpView.cxx HelpView.h         \
  TtyWidget.cxx TtyWidget.h       \
  MainWindow.cxx MainWindow.h     \
  BasicEditor.cxx BasicEditor.h   \
  EditorWidget.cxx EditorWidget.h \
  FileWidget.cxx FileWidget.h     \
  Profile.cxx Profile.h           \
  utils.cxx utils.h

sbasici_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

sbasici_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a

iconsdir = $(datadir)/icons/hicolor/128x128/apps
icons_DATA = ../../../images/sb-desktop-128x128.png
desktopdir = $(datadir)/applications
desktop_DATA = ../sdl/smallbasic.desktop

if WITH_WIN32
sbasici_LDADD += win.res
sbasici_DEPENDENCIES += win.res
win.res : win.rc
	$(WINDRES) $< -O coff -o $@
endif
