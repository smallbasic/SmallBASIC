# SmallBASIC
# Copyright(C) 2001-2015 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

SUBDIRS = @BUILD_SUBDIRS@

deb:
	fakeroot dpkg-buildpackage -b -uc -us

test:
	(cd @TEST_DIR@ && make test)

leak-test:
	(cd src/platform/console && make leak-test)

cppcheck:
	(cppcheck --quiet --enable=all src/common src/ui src/platform/android/jni src/platform/sdl)

covcheck:
	(make clean -s && \
   cov-build --dir cov-int ./configure --enable-sdl && \
   cov-build --dir cov-int make -s && \
   tar czvf smallbasic.tgz cov-int)

covcheck-android:
	(make clean -s && \
   cov-build --dir cov-int ./configure --enable-android && \
   cov-build --dir cov-int make check -s && \
   tar czvf smallbasic.tgz cov-int)

clangcheck:
	(make clean -s && \
   scan-build ./configure --enable-sdl && \
   scan-build -o scan make -s)

clangcheck-android:
	(make clean -s && \
   scan-build ./configure --enable-android && \
   scan-build -o scan make check -s)

check:
	(cd $(SUBDIRS) && make check)

release:
	(cd $(SUBDIRS) && make release)

# produce a stack dump
ndk-stack:
	adb logcat | ${NDK}/ndk-stack -sym src/platform/android/obj/local/armeabi

