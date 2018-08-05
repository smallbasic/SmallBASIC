# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

all-am: Makefile

build-test: ndk-build-test
	(./gradlew installDebug)

test: build-test
	(adb -a logcat -c && \
   adb -a shell am start net.sourceforge.smallbasic/net.sourceforge.smallbasic.MainActivity && \
   adb -a logcat DEBUG:I smallbasic:I AndroidRuntime:E *:S)

clean-ant:
	(./gradlew clean)

clean-objs: clean-ant
	(rm -rf obj/ libs/)

clean-am: clean-objs
	(cd jni && ${NDK}/ndk-build clean)

check:
	(cd jni && ${NDK}/ndk-build NDK_DEBUG=1)

ndk-build-release:
	(cd jni && ${NDK}/ndk-build NDK_DEBUG=0)

release:
	(./gradle assemble)

