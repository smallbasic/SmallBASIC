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

release:
	(cd $(SUBDIRS) && make release)

# produce a stack dump
ndk-stack:
	adb logcat | ${NDK}/ndk-stack -sym src/platform/android/obj/local/armeabi

