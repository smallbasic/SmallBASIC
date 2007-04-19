# SmallBASIC
# Copyright(C) 2001-2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = @BUILD_SUBDIRS@

EXTRA_DIST =                 \
    README.TXT               \
	sync/gtk/sbgtk.glade     \
	sync/gtk/sbgtk.gladep    \
	sync/sdl/fonts/BI1.c     \
	sync/sdl/fonts/BI2.c     \
	sync/sdl/fonts/BI5.c     \
	sync/sdl/fonts/BI7.c     \
	sync/sdl/fonts/psf2c.c


deb:
	fakeroot dpkg-buildpackage
