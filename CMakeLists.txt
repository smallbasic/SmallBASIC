# SmallBASIC
# Copyright(C) 2025 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

set(VERSION "12.29")
cmake_minimum_required(VERSION 3.10)
project(SmallBASIC VERSION ${VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cmake -DWITH_DEBUG=ON
option(WITH_DEBUG "Enable debug build" OFF)

function(writeConfig)
  execute_process(
    COMMAND date +"%a, %d %b %Y"
    OUTPUT_VARIABLE BUILD_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(BUILD_DATE "${BUILD_DATE}")

  execute_process(
    COMMAND bash -c "awk -F '.' '{printf \"0x%02d%02d%02d\", \$1, \$2, \$3}' <<< \"${VERSION}\""
    OUTPUT_VARIABLE SB_DWORD_VER
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(SB_DWORD_VER "${SB_DWORD_VER}")

  configure_file(
    ${CMAKE_SOURCE_DIR}/ide/config.h.in
    ${CMAKE_BINARY_DIR}/config.h
  )
endfunction()

function(checkDebugMode)
  if(WITH_DEBUG)
    set(SANITIZER_FLAGS "-fstack-protector-all -fno-omit-frame-pointer -fsanitize=address,undefined")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    message(STATUS "Debug mode enabled via --with-debug")
  endif()
endfunction()

function(buildSDL)
  # Initialize variables
  set(TARGET "Building SDL version.")
  set(PACKAGE_CFLAGS "")
  set(PACKAGE_LIBS "")

  # check dependencies using pkg-config
  find_package(SDL3 REQUIRED)
  find_package(PkgConfig REQUIRED)

  pkg_check_modules(SDL3 REQUIRED sdl3)
  include_directories(${SDL3_INCLUDE_DIRS})
  add_compile_options(${SDL3_CFLAGS_OTHER})

  pkg_check_modules(FREETYPE2 REQUIRED freetype2)
  include_directories(${FREETYPE2_INCLUDE_DIRS})
  add_compile_options(${FREETYPE2_CFLAGS_OTHER})

  # check for xxd command
  find_program(XXD_EXECUTABLE xxd)
  if(NOT XXD_EXECUTABLE)
    message(FATAL_ERROR "xxd command not installed: configure failed.")
  endif()

  # platform-specific configurations
  if(WIN32)
    set(PACKAGE_CFLAGS "${PACKAGE_CFLAGS} -D__USE_MINGW_ANSI_STDIO")
    add_compile_definitions(_USE_MATH_DEFINES)
    add_compile_definitions(_Win32)
    set(PACKAGE_LIBS "${PACKAGE_LIBS} -lwsock32 -lws2_32 -static-libgcc -static-libstdc++")
  elseif(APPLE)
    pkg_check_modules(FONTCONFIG REQUIRED fontconfig>=2.2)
    set(PACKAGE_CFLAGS "${PACKAGE_CFLAGS} ${FONTCONFIG_CFLAGS}")
    set(PACKAGE_LIBS "${PACKAGE_LIBS} -ldl ${FONTCONFIG_LIBS}")
  else()
    pkg_check_modules(FONTCONFIG REQUIRED fontconfig>=2.2)
    include_directories(${FONTCONFIG_INCLUDE_DIRS})
    add_compile_options(${FONTCONFIG_CFLAGS_OTHER})
    execute_process(
      COMMAND ${XXD_EXECUTABLE} -i images/sb-desktop-128x128.png
      OUTPUT_FILE ${CMAKE_BINARY_DIR}/src/platform/sdl/icon.h
    )
  endif()

  # Set compiler flags
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PACKAGE_CFLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -std=c++14")

  # Define SDL version
  execute_process(
    COMMAND pkg-config --modversion sdl3
    OUTPUT_VARIABLE _SDL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # message("SDL version: ${_SDL_VERSION}")
  set(BUILD_DATE `date +"%a, %d %b %Y"`)
  set(_SDL_VERSION ${_SDL_VERSION})
  set(_SDL ON)
  set(_UnixOS ON)
  set(IMPL_DEV_READ ON)
  set(IMPL_LOG_WRITE ON)

  # Process assets
  execute_process(
    COMMAND ${XXD_EXECUTABLE} -i src/platform/android/app/src/main/assets/main.bas
    OUTPUT_FILE ${CMAKE_BINARY_DIR}/src/platform/sdl/main_bas.h
  )
  writeConfig()
endfunction()

message("generating kwp.h")
execute_process(
  COMMAND bash -c "(cd ../documentation && g++ -o build_kwp build_kwp.cpp && ./build_kwp > ../src/ui/kwp.h)"
)

checkDebugMode()
buildSDL()

set(COMMON_DIR ${CMAKE_SOURCE_DIR}/src/common)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/lib)
set(UI_DIR ${CMAKE_SOURCE_DIR}/src/ui)
set(SDL_DIR ${CMAKE_SOURCE_DIR}/src/platform/sdl)

set(SOURCES
  ${LIB_DIR}/match.c
  ${LIB_DIR}/str.c
  ${LIB_DIR}/matrix.c
  ${LIB_DIR}/xpm.c
  ${LIB_DIR}/lodepng/lodepng.cpp
  ${COMMON_DIR}/bc.c
  ${COMMON_DIR}/blib.c
  ${COMMON_DIR}/blib_func.c
  ${COMMON_DIR}/blib_math.c
  ${COMMON_DIR}/brun.c
  ${COMMON_DIR}/ceval.c
  ${COMMON_DIR}/random.c
  ${COMMON_DIR}/eval.c
  ${COMMON_DIR}/plugins.c
  ${COMMON_DIR}/fmt.c
  ${COMMON_DIR}/kw.c
  ${COMMON_DIR}/proc.c
  ${COMMON_DIR}/sberr.c
  ${COMMON_DIR}/scan.c
  ${COMMON_DIR}/str.c
  ${COMMON_DIR}/tasks.c
  ${COMMON_DIR}/hashmap.c
  ${COMMON_DIR}/var_map.c
  ${COMMON_DIR}/var_eval.c
  ${COMMON_DIR}/keymap.c
  ${COMMON_DIR}/units.c
  ${COMMON_DIR}/var.c
  ${COMMON_DIR}/device.c
  ${COMMON_DIR}/blib_db.c
  ${COMMON_DIR}/blib_graph.c
  ${COMMON_DIR}/blib_sound.c
  ${COMMON_DIR}/screen.c
  ${COMMON_DIR}/system.c
  ${COMMON_DIR}/random.c
  ${COMMON_DIR}/eval.c
  ${COMMON_DIR}/file.c
  ${COMMON_DIR}/ffill.c
  ${COMMON_DIR}/fs_serial.c
  ${COMMON_DIR}/fs_socket_client.c
  ${COMMON_DIR}/fs_stream.c
  ${COMMON_DIR}/g_line.c
  ${COMMON_DIR}/geom.c
  ${COMMON_DIR}/inet2.c
  ${COMMON_DIR}/pfill.c
  ${COMMON_DIR}/plot.c
  ${UI_DIR}/graphics.cpp
  ${UI_DIR}/ansiwidget.cpp
  ${UI_DIR}/window.cpp
  ${UI_DIR}/screen.cpp
  ${UI_DIR}/system.cpp
  ${UI_DIR}/form.cpp
  ${UI_DIR}/inputs.cpp
  ${UI_DIR}/textedit.cpp
  ${UI_DIR}/keypad.cpp
  ${UI_DIR}/image.cpp
  ${UI_DIR}/lodepng_codec.cpp
  ${UI_DIR}/strlib.cpp
  ${UI_DIR}/audio.cpp
  ${SDL_DIR}/main.cpp
  ${SDL_DIR}/display.cpp
  ${SDL_DIR}/runtime.cpp
  ${SDL_DIR}/settings.cpp
  ${SDL_DIR}/editor.cpp
  ${SDL_DIR}/syswm.cpp
)

include_directories(
  ${SDL3_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}
)

add_executable(sbasicg ${SOURCES})

target_link_libraries(sbasicg ${SDL3_LIBRARIES} m)
target_link_libraries(sbasicg ${FREETYPE2_LIBRARIES})
target_link_libraries(sbasicg ${FONTCONFIG_LIBRARIES})
