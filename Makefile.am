# SmallBASIC
# Copyright(C) 2001-2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = @BUILD_SUBDIRS@

EXTRA_DIST =					\
    README.TXT					\
    autogen.sh					\
    src/gtk/sbgtk.glade				\
    src/gtk/sbgtk.gladep			\
    src/gtk/debian/changelog			\
    src/gtk/debian/control			\
    src/gtk/debian/copyright			\
    src/gtk/debian/rules			\
    src/gtk/debian/sbasic.links			\
    src/sdl/fonts/BI1.c				\
    src/sdl/fonts/BI2.c				\
    src/sdl/fonts/BI5.c				\
    src/sdl/fonts/BI7.c				\
    src/sdl/fonts/psf2c.c			\
    documentation/export_cvs.bas		\
    documentation/sbasic_ref.csv		\
    documentation/HOWTO/HOWTO-DOCUMENT.TXT	\
    documentation/HOWTO/HOWTO-PORT.TXT		\
    documentation/HOWTO/DEVELOP.TXT		\
    documentation/LICENSE			\
    documentation/README			\
    documentation/README.BCC			\
    documentation/README.DEV			\
    documentation/README.DOS			\
    documentation/README.HELIO			\
    documentation/README.PALM			\
    documentation/README.UNIX			\
    plugin/welcome.bas				\
    plugin/calc.bas				\
    plugin/calc.html				\
    plugin/comment_in.bas			\
    plugin/comment_out.bas			\
    plugin/dos2unix.bas				\
    plugin/image_demo.bas			\
    plugin/indent.bas				\
    plugin/logo.gif				\
    plugin/memoryTest.bas			\
    plugin/recent_files.bas			\
    plugin/snarf.bas				\
    plugin/sudoku.bas				\
    plugin/_survey.bas


deb:
	fakeroot dpkg-buildpackage

install-exec-hook:
	mkdir -p $(pkgdatadir) &&				\
	mkdir -p $(pkgdatadir)/plugin &&			\
	mkdir -p $(pkgdatadir)/ide &&				\
	mkdir -p $(pkgdatadir)/samples &&			\
	cp documentation/sbasic_ref.csv $(pkgdatadir) &&	\
	cp plugin/*.* $(pkgdatadir)/plugin &&			\
	cp ide/*.* $(pkgdatadir)/ide &&				\
	cp -R samples/distro-examples/* $(pkgdatadir)/samples
