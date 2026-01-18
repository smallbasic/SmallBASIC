# SmallBASIC for SDL
# Copyright(C) 2001-2026 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

AM_CPPFLAGS = -I$(top_srcdir)/src -I. @PACKAGE_CFLAGS@

EXTRA_DIST = $(desktopentry_DATA)

bin_PROGRAMS = sbasicg

sbasicg_SOURCES =           \
  ../../lib/lodepng/lodepng.cpp ../../lib/lodepng/lodepng.h \
  ../../ui/graphics.cpp     \
  ../../ui/ansiwidget.cpp   \
  ../../ui/window.cpp       \
  ../../ui/screen.cpp       \
  ../../ui/system.cpp       \
  ../../ui/form.cpp         \
  ../../ui/inputs.cpp       \
  ../../ui/textedit.cpp     \
  ../../ui/image.cpp        \
  ../../ui/strlib.cpp       \
  ../../ui/audio.cpp        \
  ../../ui/keypad.cpp       \
  ../../ui/image_codec.cpp  \
  main.cpp                  \
  display.cpp               \
  runtime.cpp               \
  settings.cpp              \
  editor.cpp                \
  syswm.cpp

sbasicg_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

sbasicg_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a

iconsdir = $(datadir)/icons/hicolor/128x128/apps
icons_DATA = ../../../images/io.github.smallbasic.SmallBASIC.png

desktopdir = $(datadir)/applications
desktop_DATA = io.github.smallbasic.SmallBASIC.desktop

metainfodir = $(datadir)/metainfo
metainfo_DATA = io.github.smallbasic.SmallBASIC.metainfo.xml

if WITH_WIN32
sbasicg_LDADD += win.res
sbasicg_DEPENDENCIES += win.res
win.res : win.rc
	$(WINDRES) $< -O coff -o $@
endif
