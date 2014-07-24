# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

all-am: Makefile ndk-build-test

ndk-build-test:
	(cd jni && ${NDK}/ndk-build NDK_DEBUG=1)

ant-build-test: ndk-build-test
	(cd ../../../ide/android && ant -quiet debug install)

test: ant-build-test
	(cd ../../../ide/android && \
   adb -a logcat -c && \
   adb -a shell am start net.sourceforge.smallbasic/net.sourceforge.smallbasic.MainActivity && \
   adb -a logcat)

clean-am: 
	(cd jni && ${NDK}/ndk-build clean)

