# SmallBASIC
# Copyright(C) 2001-2007 Chris Warren-Smith. [http://tinyurl.com/ja2ss]
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = @BUILD_SUBDIRS@

EXTRA_DIST =					\
    AUTHORS					\
    ChangeLog					\
    configure.in				\
    NEWS					\
    README					\
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
    plugins/welcome.bas				\
    plugins/calc.bas				\
    plugins/calc.html				\
    plugins/comment_in.bas			\
    plugins/comment_out.bas			\
    plugins/dos2unix.bas			\
    plugins/image_demo.bas			\
    plugins/indent.bas				\
    plugins/logo.gif				\
    plugins/memoryTest.bas			\
    plugins/recent_files.bas			\
    plugins/snarf.bas				\
    plugins/sudoku.bas				\
    plugins/_survey.bas				\
    images/sb16x16.png				\
    images/sb32x32.png				\
    images/sb-desktop-16x16.png			\
    images/sb-desktop-32x32.png			\
    images/smallbasic.png			\
    ide/smallbasic.lang				\
    ide/small-basic-mode.el			\
    ide/smallbasic.syn

deb:
	fakeroot dpkg-buildpackage

install-exec-hook:
	mkdir -p $(pkgdatadir) &&				\
	mkdir -p $(pkgdatadir)/plugins &&			\
	mkdir -p $(pkgdatadir)/ide &&				\
	mkdir -p $(pkgdatadir)/samples &&			\
	cp documentation/sbasic_ref.csv $(pkgdatadir) &&	\
	cp plugins/*.* $(pkgdatadir)/plugins &&			\
	cp ide/*.* $(pkgdatadir)/ide &&				\
	cp -R samples/distro-examples/* $(pkgdatadir)/samples
