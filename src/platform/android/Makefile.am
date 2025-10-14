# SmallBASIC
# Copyright(C) 2001-2014 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

#
# For investigating crash reports in play-studio, for example:
# aarch64-linux-android-addr2line -e symbols/v12.31-87/arm64-v8a/libsmallbasic.so.dbg -f -C 0x00000000000d2dd0
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
	(./gradlew clean :app:bundle)
	@echo "Saving debug symbols..."
	@VERSION_CODE=$$(sed -n 's/^[[:space:]]*versionCode[[:space:]]*\([0-9]*\).*/\1/p' app/build.gradle); \
	VERSION_NAME=$$(sed -n "s/^[[:space:]]*versionName[[:space:]]*['\"]\\([^'\"]*\\)['\"].*/\\1/p" app/build.gradle); \
	SYMBOL_DIR="symbols/v$${VERSION_NAME}-$${VERSION_CODE}"; \
	mkdir -p "$${SYMBOL_DIR}"; \
	if [ -d app/build/intermediates/native_debug_metadata/release/extractReleaseNativeDebugMetadata/out ]; then \
		cp -r app/build/intermediates/native_debug_metadata/release/extractReleaseNativeDebugMetadata/out/* "$${SYMBOL_DIR}/"; \
		echo "Symbols saved to $${SYMBOL_DIR}/"; \
		ls -lh "$${SYMBOL_DIR}"/*/*.dbg; \
	else \
		echo "Warning: debug symbols not found"; \
	fi

library:
	(./gradlew clean assemble)
	(mkdir -p dist)
	(cp -R app/build/intermediates/stripped_native_libs/release/out/lib/ dist)
	(cp app/build/intermediates/dex/release/minifyReleaseWithR8/classes.dex dist)

# launch WebServerTest in intellij
start-ui:
	(cd app/src/main/assets/webui && valgrind --leak-check=full ../../../../../../web/sbasicw -p 3000 -a 'api' -o 'http://localhost:8080')
