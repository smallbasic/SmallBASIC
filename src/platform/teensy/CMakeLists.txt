# SmallBASIC for teensy mcu
# Copyright(C) 2025 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

cmake_minimum_required(VERSION 3.10)
option(TEENSY40 "Build for teensy 4.0 instead of 4.1" OFF)
project(SmallBASIC)
include(ExternalProject)
set(CMAKE_SYSTEM_NAME Generic)
set(TARGET "smallbasic")

if(TEENSY40)
  message("Building for TEENSY 4.0")
  set(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
  set(TEENSY_SRC ${MODULES}/cores/teensy4)
  set(MCU "IMXRT1062")
  set(MCU_LD ${TEENSY_SRC}/imxrt1062.ld)
  set(MCU_DEF "ARDUINO_TEENSY40")
  set(CPU_OPTIONS "-mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb")
else()
  message("Building for TEENSY 4.1")
  set(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
  set(TEENSY_SRC ${MODULES}/cores/teensy4)
  set(MCU "IMXRT1062")
  set(MCU_LD ${TEENSY_SRC}/imxrt1062_t41.ld)
  set(MCU_DEF "ARDUINO_TEENSY41")
  set(CPU_OPTIONS "-mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb")
endif()

# Preprocessor flags for both C and C++
add_compile_options(-g -O2 -ffunction-sections -fdata-sections -DENABLE_LOGGING
  -DF_CPU=600000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -D__${MCU}__ -DARDUINO=10813 -DTEENSYDUINO=159 -D${MCU_DEF})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CPU_OPTIONS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPU_OPTIONS} -std=gnu++17 -felide-constructors -fno-exceptions -fpermissive -fno-rtti -Wno-error=narrowing")
set(CMAKE_EXE_LINKER_FLAGS "-Os -Wl,--gc-sections,--relax ${SPECS} ${CPU_OPTIONS} -T${MCU_LD}")

# Set the toolchain for cross-compilation
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_SIZE arm-none-eabi-size)

# Include the module directories
add_subdirectory(libs)
add_subdirectory(common)

# Add module includes
include_directories(${TEENSY_SRC})
include_directories(${MODULES}/ADC)
include_directories(${MODULES}/Adafruit-GFX-Library)
include_directories(${MODULES}/Adafruit_BusIO)
include_directories(${MODULES}/SPI)
include_directories(${MODULES}/Adafruit_SSD1306)
include_directories(${MODULES}/USBHost_t36)
include_directories(${MODULES}/Wire)
include_directories(${MODULES}/SD/src)
include_directories(${MODULES}/SdFat/src)

# include config.h and common headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../..)

set(SOURCES
  ../../ui/strlib.cpp
  src/noop.c
  src/device.cpp
  src/main.cpp
  src/module.cpp
  src/teensy.cpp
  src/ssd1306.cpp
)

# Add executable with the list of source files
add_executable(${TARGET}.elf ${SOURCES})

# enable all warnings in main.cpp
target_compile_options(${TARGET}.elf PRIVATE -Wall)

# Link against the teensy and CMSIS DSP libraries
target_link_libraries(${TARGET}.elf common libs ${MODULES}/CMSIS-DSP/build/Source/libCMSISDSP.a -lm)

# Post-build step to create HEX file using objcopy
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ${TARGET}.elf ${TARGET}.hex
  COMMENT "Generating ${TARGET}.hex from ${TARGET}.elf"
)

# Optional: Show size of the ELF file after building (equivalent to $(SIZE) $<)
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
  COMMAND ${CMAKE_SIZE} ${TARGET}.elf
  COMMENT "Showing size of ${TARGET}.elf"
)
