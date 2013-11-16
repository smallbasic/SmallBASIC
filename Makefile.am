# SmallBASIC
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = @BUILD_SUBDIRS@

EXTRA_DIST =																		\
    AUTHORS																			\
    ChangeLog																		\
    configure.in																\
    NEWS																				\
    README																			\
    autogen.sh																	\
    src/platform/gtk/sbgtk.glade								\
    src/platform/gtk/sbgtk.gladep								\
    src/platform/gtk/debian/changelog						\
    src/platform/gtk/debian/control							\
    src/platform/gtk/debian/copyright						\
    src/platform/gtk/debian/rules								\
    src/platform/gtk/debian/sbasic.links				\
    src/platform/sdl/fonts/BI1.c								\
    src/platform/sdl/fonts/BI2.c								\
    src/platform/sdl/fonts/BI5.c								\
    src/platform/sdl/fonts/BI7.c								\
    src/platform/sdl/fonts/psf2c.c							\
    documentation/export_csv.bas								\
    documentation/sbasic_ref.csv								\
    documentation/HOWTO/HOWTO-DOCUMENT.TXT			\
    documentation/HOWTO/HOWTO-PORT.TXT					\
    documentation/HOWTO/DEVELOP.TXT							\
    documentation/LICENSE												\
    documentation/README												\
    documentation/README.BCC										\
    documentation/README.DEV										\
    documentation/README.DOS										\
    documentation/README.HELIO									\
    documentation/README.PALM										\
    documentation/README.UNIX										\
    plugins/publish.bas													\
    plugins/comment_in.bas											\
    plugins/comment_out.bas											\
    plugins/dos2unix.bas												\
    plugins/indent.bas													\
    plugins/help.bas														\
    translator/SB.g															\
    translator/Makefile													\
    translator/Translator.java									\
    images/logo.gif															\
    images/sb16x16.png													\
    images/sb32x32.png													\
    images/sb-desktop-16x16.png									\
    images/sb-desktop-32x32.png									\
    ide/smallbasic.lang													\
    ide/small-basic-mode.el											\
    ide/smallbasic.syn													\
    rpm/README																	\
    rpm/SPECS/opensuse.spec

deb:
	fakeroot dpkg-buildpackage

install-exec-hook:																	\
	(mkdir -p $(pkgdatadir) &&												\
	 mkdir -p $(pkgdatadir)/plugins &&								\
	 mkdir -p $(pkgdatadir)/ide &&										\
	 mkdir -p $(pkgdatadir)/samples &&								\
	 cp documentation/sbasic_ref.csv $(pkgdatadir) &&	\
	 cp plugins/*.* $(pkgdatadir)/plugins &&					\
	 cp ide/*.* $(pkgdatadir)/ide)

test:
	(cd $(SUBDIRS) && make test)
