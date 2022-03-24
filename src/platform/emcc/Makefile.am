# SmallBASIC command line version
# Copyright(C) 2001-2022 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

#
# Setup:
#
# $ git clone https://github.com/emscripten-core/emsdk.git
# $ emsdk update
# $ emsdk install latest
# $ emsdk activate latest
# $ . ./emsdk_env.sh
#
# Build:
#
# $ emconfigure ./configure --enable-emcc
# $ emmake make
#

AM_CPPFLAGS = -I$(top_builddir)/src -I. @PACKAGE_CFLAGS@

CLEANFILES = sbasic.js sbasic.wasm

bin_PROGRAMS = sbasic.html

sbasic_html_SOURCES =																				\
  ../../lib/lodepng/lodepng.cpp ../../lib/lodepng/lodepng.h	\
  ../../ui/ansiwidget.cpp																		\
  ../../ui/window.cpp																				\
  ../../ui/screen.cpp																				\
  ../../ui/system.cpp																				\
  ../../ui/form.cpp																					\
  ../../ui/inputs.cpp																				\
  ../../ui/textedit.cpp																			\
  ../../ui/image.cpp																				\
  ../../ui/strlib.cpp																				\
  ../../ui/audio.cpp																				\
  runtime.h runtime.cpp																			\
  canvas.h canvas.cpp																				\
  main.cpp

sbasic_html_LDFLAGS = -O3 -lembind --shell-file shell.html -s ASYNCIFY=1 --preload-file basic

sbasic_html_LDADD = -L$(top_srcdir)/src/common -lsb_common @PACKAGE_LIBS@

sbasic_html_DEPENDENCIES = $(top_srcdir)/src/common/libsb_common.a
