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

fuzz-test:
	(cd src/platform/console && make fuzz-test)

cppcheck:
	(cppcheck --quiet --enable=all src/common src/ui src/platform/android/jni src/platform/sdl src/platform/fltk)

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
   scan-build ./configure && \
   scan-build -o scan make -s)

clangcheck-fltk:
	(make clean -s && \
   scan-build ./configure --enable-fltk && \
   scan-build -o scan make -s)

clangcheck-sdl:
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

appImage:
	~/apps/linuxdeploy-x86_64.AppImage --appdir=AppDir --executable=./src/platform/fltk/sbasici && \
	~/apps/appimagetool-x86_64.AppImage AppDir

#
# flatpak install flathub org.flatpak.Builder
# sudo apt install flatpak-builder
# https://docs.flathub.org/docs/for-app-authors/submission
#
# inspect build-dir/files (or /app from flatpack-shell) for additional "cleanup" items
#

# starting from scratch, download and build all dependencies
flatpak-build:
	flatpak-builder --force-clean --ccache build-dir io.github.smallbasic.SmallBASIC.json

# development workflow
flatpak-run:
	make -s -j$(nproc) && \
  flatpak-builder --user --install --force-clean --ccache --disable-download build-dir io.github.smallbasic.SmallBASIC.json && \
	flatpak run io.github.smallbasic.SmallBASIC

# debugging specific flatpak issues
flatpak-debug:
	make -s -j$(nproc) && \
	flatpak-builder --build-only --force-clean --ccache --disable-download --disable-updates build-dir io.github.smallbasic.SmallBASIC.json && \
	flatpak-builder --run build-dir io.github.smallbasic.SmallBASIC.json gdb sbasicg

# to inspect the built components - requires commenting deletion of xxd
flatpak-shell:
	flatpak-builder --build-shell=sbasicg build-dir io.github.smallbasic.SmallBASIC.json

# how to invoke the command line sbasic
flatpak-command:
	flatpak run --filesystem=`pwd` --command=sbasic io.github.smallbasic.SmallBASIC $1

# preparing for release
flatpak-lint1:
	flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest io.github.smallbasic.SmallBASIC.json

flatpak-lint2:
	flatpak run --command=flatpak-builder-lint org.flatpak.Builder builddir build-dir

# extra step 1 preparing for release
flatpak-create-repo:
	flatpak-builder --repo=repo build-dir io.github.smallbasic.SmallBASIC.json

# extra step 2 preparing for release
flatpak-lint3:
		flatpak run --command=flatpak-builder-lint org.flatpak.Builder repo repo

EXTRA_DIST =                                                 \
documentation/build_kwp.cpp                                  \
documentation/sbasic_ref.csv                                 \
images/sb-desktop-128x128.png                                \
src/common/hotspots.h                                        \
src/common/inet2.c                                           \
src/include/module.h                                         \
src/include/osd.h                                            \
src/include/var.h                                            \
src/include/var_map.h                                        \
src/languages/chars.en.h                                     \
src/languages/keywords.en.c                                  \
src/languages/messages.en.h                                  \
src/lib/jsmn/jsmn.h                                          \
src/lib/lodepng/lodepng.cpp                                  \
src/lib/lodepng/lodepng.h                                    \
src/lib/maapi.h                                              \
src/lib/match.c                                              \
src/lib/match.h                                              \
src/lib/matrix.c                                             \
src/lib/miniaudio/miniaudio.h                                \
src/lib/stb/stb_textedit.h                                   \
src/lib/str.c                                                \
src/lib/str.h                                                \
src/lib/xpm.c                                                \
src/platform/android/Makefile.am                             \
src/platform/android/Makefile.in                             \
src/platform/android/app/src/main/assets/main.bas            \
src/platform/android/app/src/main/assets/samples/welcome.bas \
src/platform/android/jni/Android.mk                          \
src/platform/android/jni/Application.mk                      \
src/platform/android/jni/common/Android.mk                   \
src/platform/android/jni/display.cpp                         \
src/platform/android/jni/display.h                           \
src/platform/android/jni/freetype/Android.mk                 \
src/platform/android/jni/main.cpp                            \
src/platform/android/jni/runtime.cpp                         \
src/platform/android/jni/runtime.h                           \
src/platform/android/webui/package.json                      \
src/platform/android/webui/src/App.jsx                       \
src/platform/android/webui/index.html                        \
src/platform/console/Makefile.am                             \
src/platform/console/Makefile.in                             \
src/platform/console/decomp.c                                \
src/platform/console/device.cpp                              \
src/platform/console/image.cpp                               \
src/platform/console/input.cpp                               \
src/platform/console/io.github.smallbasic.appdata.xml        \
src/platform/console/io.github.smallbasic.desktop            \
src/platform/console/main.cpp                                \
src/platform/emcc/Makefile.am                                \
src/platform/emcc/Makefile.in                                \
src/platform/emcc/canvas.cpp                                 \
src/platform/emcc/canvas.h                                   \
src/platform/emcc/icon.h                                     \
src/platform/emcc/keymap.h                                   \
src/platform/emcc/main.cpp                                   \
src/platform/emcc/main_bas.h                                 \
src/platform/emcc/runtime.cpp                                \
src/platform/emcc/runtime.h                                  \
src/platform/emcc/sbasic.html                                \
src/platform/emcc/shell.html                                 \
src/platform/fltk/BasicEditor.cxx                            \
src/platform/fltk/BasicEditor.h                              \
src/platform/fltk/EditorWidget.cxx                           \
src/platform/fltk/EditorWidget.h                             \
src/platform/fltk/FileWidget.cxx                             \
src/platform/fltk/FileWidget.h                               \
src/platform/fltk/HelpView.cxx                               \
src/platform/fltk/HelpView.h                                 \
src/platform/fltk/HelpWidget.cxx                             \
src/platform/fltk/HelpWidget.h                               \
src/platform/fltk/MainWindow.cxx                             \
src/platform/fltk/MainWindow.h                               \
src/platform/fltk/Makefile.am                                \
src/platform/fltk/Makefile.in                                \
src/platform/fltk/Profile.cxx                                \
src/platform/fltk/Profile.h                                  \
src/platform/fltk/TtyWidget.cxx                              \
src/platform/fltk/TtyWidget.h                                \
src/platform/fltk/build_kwp.cxx                              \
src/platform/fltk/display.cxx                                \
src/platform/fltk/display.h                                  \
src/platform/fltk/icon.h                                     \
src/platform/fltk/io.github.smallbasic.appdata.xml           \
src/platform/fltk/io.github.smallbasic.desktop               \
src/platform/fltk/kwp.h                                      \
src/platform/fltk/runtime.cxx                                \
src/platform/fltk/runtime.h                                  \
src/platform/fltk/utils.cxx                                  \
src/platform/fltk/utils.h                                    \
src/platform/fltk/win.rc                                     \
src/platform/sdl/Makefile.am                                 \
src/platform/sdl/Makefile.in                                 \
src/platform/sdl/display.cpp                                 \
src/platform/sdl/display.h                                   \
src/platform/sdl/editor.cpp                                  \
src/platform/sdl/icon.h                                      \
src/platform/sdl/io.github.smallbasic.appdata.xml            \
src/platform/sdl/io.github.smallbasic.desktop                \
src/platform/sdl/keymap.h                                    \
src/platform/sdl/main.cpp                                    \
src/platform/sdl/main_bas.h                                  \
src/platform/sdl/runtime.cpp                                 \
src/platform/sdl/runtime.h                                   \
src/platform/sdl/settings.cpp                                \
src/platform/sdl/settings.h                                  \
src/platform/sdl/syswm.cpp                                   \
src/platform/sdl/syswm.h                                     \
src/platform/sdl/win.rc                                      \
src/platform/web/Makefile.am                                 \
src/platform/web/Makefile.in                                 \
src/platform/web/canvas.cpp                                  \
src/platform/web/canvas.h                                    \
src/platform/web/main.cpp                                    \
src/ui/ansiwidget.cpp                                        \
src/ui/ansiwidget.h                                          \
src/ui/audio.cpp                                             \
src/ui/audio.h                                               \
src/ui/canvas.h                                              \
src/ui/form.cpp                                              \
src/ui/graphics.cpp                                          \
src/ui/graphics.h                                            \
src/ui/image.cpp                                             \
src/ui/image.h                                               \
src/ui/inputs.cpp                                            \
src/ui/inputs.h                                              \
src/ui/rgb.h                                                 \
src/ui/screen.cpp                                            \
src/ui/screen.h                                              \
src/ui/shape.h                                               \
src/ui/strlib.cpp                                            \
src/ui/strlib.h                                              \
src/ui/system.cpp                                            \
src/ui/system.h                                              \
src/ui/textedit.cpp                                          \
src/ui/textedit.h                                            \
src/ui/theme.h                                               \
src/ui/utils.h                                               \
src/ui/window.cpp
