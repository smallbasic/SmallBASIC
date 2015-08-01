# SmallBASIC
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = @BUILD_SUBDIRS@

EXTRA_DIST =                                    \
    AUTHORS                                     \
    ChangeLog                                   \
    configure.in                                \
    NEWS                                        \
    README                                      \
    autogen.sh                                  \
    documentation/export_csv.bas                \
    documentation/sbasic_ref.csv                \
    documentation/HOWTO/HOWTO-DOCUMENT.TXT      \
    documentation/HOWTO/HOWTO-PORT.TXT          \
    documentation/HOWTO/DEVELOP.TXT             \
    documentation/LICENSE                       \
    documentation/README                        \
    documentation/README.UNIX                   \
    images/logo.gif                             \
    images/sb16x16.png                          \
    images/sb32x32.png                          \
    images/sb-desktop-16x16.png                 \
    images/sb-desktop-32x32.png                 \
    ide/smallbasic.lang                         \
    ide/small-basic-mode.el                     \
    ide/smallbasic.syn                          \
    rpm/README                                  \
    rpm/SPECS/opensuse.spec

install-exec-hook:
	(mkdir -p $(DESTDIR)$(pkgdatadir) &&                        \
   mkdir -p $(DESTDIR)$(pkgdatadir)/plugins &&                \
   mkdir -p $(DESTDIR)$(pkgdatadir)/ide &&                    \
   mkdir -p $(DESTDIR)$(pkgdatadir)/samples &&                \
   cp documentation/sbasic_ref.csv $(DESTDIR)$(pkgdatadir) && \
   cp plugins/*.* $(DESTDIR)$(pkgdatadir)/plugins &&          \
   cp ide/*.* $(DESTDIR)$(pkgdatadir)/ide)

deb:
	fakeroot dpkg-buildpackage

test:
	(cd @TEST_DIR@ && make test)

release:
	(cd $(SUBDIRS) && make release)

# produce a stack dump
ndk-stack:
	adb logcat | ${NDK}/ndk-stack -sym src/platform/android/obj/local/armeabi

