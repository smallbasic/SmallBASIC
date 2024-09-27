SmallBASIC is a fast and easy to learn BASIC language interpreter ideal for everyday calculations, scripts and prototypes. SmallBASIC includes trigonometric, matrices and algebra functions, a built in IDE, a powerful string library, system, sound, and graphic commands along with structured programming syntax.

## Building the SDL version

### LINUX

#### Install packages

Ubuntu

```
sudo apt-get install git autotools-dev automake gcc g++ libsdl2-dev libfreetype6-dev libfontconfig1-dev xxd
```

Manjaro (Arch)

```
sudo pacman -S gcc make autoconf automake sdl2 freetype2 fontconfig pkgconf vim
```

#### Initial setup

```
 $ git clone https://github.com/smallbasic/SmallBASIC.git
 $ cd SmallBASIC
 $ git submodule update --init
 $ sh autogen.sh
```

#### Build

```
 $ ./configure --enable-sdl
 $ make
```

#### Install (optional)

```
 $ sudo make install 
```

This will install `sbasicg` in `/usr/local/bin`

#### Run

```
 $ cd ./src/platform/sdl/
 $ sbasicg
```

#### Build the Debian package (optional)

```
 $ sudo apt-get install dpkg-dev build-essential debhelper
 $ make deb
```

### WINDOWS

Install tools: `https://www.gtk.org/download/windows.php`

```
 $ ./configure --host=i686-w64-mingw32 --enable-sdl
 $ make
```

### Cross-compiling for Windows under Linux

```
 $ sudo apt-get install mingw-w64
 $ ./configure --host=i686-w64-mingw32 --prefix=/devsw/mingw --enable-sdl
```

Note: requires building SDL2 and freetype-2 into the prefix folder

### MacOSX

#### Install packages

Open the terminal window, then type the following commands at the prompt:

```
$ brew install sdl2
$ brew install freetype
$ brew link --overwrite freetype
$ brew install fontconfig
$ brew install autotools
$ brew install automake
$ brew install autoconf
```

#### Initial setup

```
$ git clone https://github.com/smallbasic/SmallBASIC.git
$ cd SmallBASIC
$ git submodule update --init
$ sh autogen.sh
```

#### Build

```
$ ./configure --enable-sdl
$ make
```

#### Run

```
$ cd /src/platform/sdl
$ ./sbasicg
```

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

## Building web-server sbasicw

### Linux

#### Install microhttp library

Ubuntu
```
 $ sudo apt install libmicrohttpd-dev
```

Manjaro
```
 $ sudo pacman -S libmicrohttpd
```

#### Build

```
 $ ./configure --enable-web
 $ make
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

## Building the FLTK version

#### Install and build FLTK 1.4

```
$ cd ~/github
$ git clone https://github.com/fltk/fltk.git
$ sudo make install
```

#### Build

```
$ cd ~/github/SmallBASIC
$ ./configure --enable-fltk
$ make -s
```

## .indent.pro settings
```
 -brf -nbap -br -brs -cdb -cdw -ce -cli0 -fca -i2 -l110 -lc110 -lp
 -nbbo -nbc -ncs -nip -npcs -npsl -nut -ppi0 -sc
```
