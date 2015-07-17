SmallBASIC is a fast and easy to learn BASIC language interpreter ideal for everyday calculations, scripts and prototypes. SmallBASIC includes trigonometric, matrices and algebra functions, a built in IDE, a powerful string library, system, sound, and graphic commands along with structured programming syntax

## Building the SDL version
```
 $ ./configure --enable-sdl
 $ make
```
 On windows, install tools:
 http://www.gtk.org/download/win32.php
```
 $ ./configure --host=i686-w64-mingw32 --enable-sdl
 $ make
```
 or for cross-compiling under linux:
```
 $ sudo apt-get install mingw-w64
 $ ./configure --host=i686-w64-mingw32 --prefix=/devsw/mingw --enable-sdl
```
Note: requires building SDL2 and freetype-2 into the prefix folder

## Building the non-graphical console version (cygwin or linux)
```
 $ ./configure && make
```
 Windows 32 bit mingw console:
```
 $./configure --host=i686-w64-mingw32 && make
```
 Windows 64 bit mingw console:
```
 $./configure --host=x86_64-w64-mingw32 && make
```
## Building the Android port

1. Setup .bashrc
```
export PATH=$PATH:~/android-sdk/depot_tools:~/android-sdk/android-sdk-linux/platform-tools/
export ANDROID_SDK_ROOT=~/android-sdk/android-sdk-linux
export TARGET_DEVICE=arm
export PLATFORM_PREFIX=~/android-sdk/android-ext/
export PATH=${PLATFORM_PREFIX}/bin:${PATH}
export NDK=~/android-sdk/android-ndk-r10d
export NDK_PLATFORM=android-19
export ANDROID_LOG_TAGS="DEBUG:I smallbasic:I AndroidRuntime:E *:S"
```

2. Build FreeType
First, prepare the cross-compiler from the NDK:
```
  $NDK_PATH/build/tools/make-standalone-toolchain.sh \
      --platform=$NDK_PLATFORM --install-dir=$PLATFORM_PREFIX --arch=arm
```
Then use it to cross-compile the tools:
```
  cd freetype-2.5.3/
  CFLAGS="-std=gnu99" ./configure --host=arm-linux-androideabi --prefix=/freetype --without-zlib --without-png --without-harfbuzz
  make
  make install DESTDIR=$(pwd)
```

3. setup config.h
```
./configure --enable-android
```

4. Build the native activity
```
$ cd src/platform/andoid/jni && $NDK/ndk-build NDK_DEBUG=0
$ cd - && cd ide/android/ && ant release install
```
Useful adb commands for debugging:
adb shell dumpsys cpuinfo
adb shell top -m 10

## Building the TIZEN port

1. Download and install the Tizen IDE

2. Add the following lines to .bashrc
```
TIZEN_SDK=${HOME}/tizen-sdk
TIZEN_ROOTSTRAP=${TIZEN_SDK}/platforms/mobile-3.0/rootstraps/mobile-3.0-device.native
export TIZEN_BIN=${TIZEN_SDK}/tools/arm-linux-gnueabi-gcc-4.5/bin
export TIZEN_LIBS=${HOME}/tizen-lib
export PATH=${PATH}:${TIZEN_BIN}:~/bin
export CHOST=arm-linux-gnueabi
export LDFLAGS="--sysroot=${TIZEN_ROOTSTRAP} -L${TIZEN_LIBS}/lib"
export CPPFLAGS="--sysroot=${TIZEN_ROOTSTRAP} -fmessage-length=0 -fPIC\
  -I${TIZEN_ROOTSTRAP}/usr/include -I${TIZEN_LIBS}/include"
export CFLAGS=${CPPFLAGS}
```
3. Build the common components
```
./configure --enable-tizen
```
4. Build the project using Tizen IDE

### .indent.pro settings
```
 -brf -nbap -br -brs -cdb -cdw -ce -cli0 -fca -i2 -l110 -lc110 -lp
 -nbbo -nbc -ncs -nip -npcs -npsl -nut -ppi0 -sc
```
