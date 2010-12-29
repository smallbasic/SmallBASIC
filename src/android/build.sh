#!/bin/bash

NDK_BUILD="/home/chris/opt/android-ndk-r5/ndk-build"
ANT="ant debug"
ADB="/home/chris/opt/android-sdk-linux_x86/platform-tools/adb"
INSTALL="${ADB} install -r bin/SmallBASIC-debug.apk"

mkdir -p src

${NDK_BUILD} && ${ANT} && ${INSTALL}
