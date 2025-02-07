# SmallBASIC for teensy mcu
# Copyright(C) 2025 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

set(MODULES_DIR ${CMAKE_SOURCE_DIR}/build/modules)

set(TEENSY_DIR ${MODULES_DIR}/cores/teensy4)
set(ADC_DIR ${MODULES_DIR}/ADC)
set(GFX_DIR ${MODULES_DIR}/Adafruit-GFX-Library)
set(SPI_DIR ${MODULES_DIR}/SPI)
set(SSD_DIR ${MODULES_DIR}/Adafruit_SSD1306)
set(USB_DIR ${MODULES_DIR}/USBHost_t36)
set(WIRE_DIR ${MODULES_DIR}/Wire)

set(SOURCES
  ${ADC_DIR}/ADC.cpp
  ${ADC_DIR}/ADC_Module.cpp
  ${ADC_DIR}/AnalogBufferDMA.cpp
  ${GFX_DIR}/Adafruit_GFX.cpp
  ${SPI_DIR}/SPI.cpp
  ${SSD_DIR}/Adafruit_SSD1306.cpp
  ${TEENSY_DIR}/CrashReport.cpp
  ${TEENSY_DIR}/DMAChannel.cpp
  ${TEENSY_DIR}/EventResponder.cpp
  ${TEENSY_DIR}/HardwareSerial.cpp
  ${TEENSY_DIR}/HardwareSerial1.cpp
  ${TEENSY_DIR}/HardwareSerial2.cpp
  ${TEENSY_DIR}/HardwareSerial3.cpp
  ${TEENSY_DIR}/HardwareSerial4.cpp
  ${TEENSY_DIR}/HardwareSerial5.cpp
  ${TEENSY_DIR}/HardwareSerial6.cpp
  ${TEENSY_DIR}/HardwareSerial7.cpp
  ${TEENSY_DIR}/HardwareSerial8.cpp
  ${TEENSY_DIR}/IPAddress.cpp
  ${TEENSY_DIR}/IntervalTimer.cpp
  ${TEENSY_DIR}/Print.cpp
  ${TEENSY_DIR}/Stream.cpp
  ${TEENSY_DIR}/Time.cpp
  ${TEENSY_DIR}/WMath.cpp
  ${TEENSY_DIR}/WString.cpp
  ${TEENSY_DIR}/analog.c
  ${TEENSY_DIR}/bootdata.c
  ${TEENSY_DIR}/clockspeed.c
  ${TEENSY_DIR}/debugprintf.c
  ${TEENSY_DIR}/delay.c
  ${TEENSY_DIR}/digital.c
  ${TEENSY_DIR}/eeprom.c
  ${TEENSY_DIR}/extmem.c
  ${TEENSY_DIR}/fuse.c
  ${TEENSY_DIR}/interrupt.c
  ${TEENSY_DIR}/keylayouts.c
  ${TEENSY_DIR}/libc.c
  ${TEENSY_DIR}/new.cpp
  ${TEENSY_DIR}/nonstd.c
  ${TEENSY_DIR}/pwm.c
  ${TEENSY_DIR}/rtc.c
  ${TEENSY_DIR}/serialEvent.cpp
  ${TEENSY_DIR}/serialEvent1.cpp
  ${TEENSY_DIR}/serialEvent2.cpp
  ${TEENSY_DIR}/serialEvent3.cpp
  ${TEENSY_DIR}/serialEvent4.cpp
  ${TEENSY_DIR}/serialEvent5.cpp
  ${TEENSY_DIR}/serialEvent6.cpp
  ${TEENSY_DIR}/serialEvent7.cpp
  ${TEENSY_DIR}/serialEvent8.cpp
  ${TEENSY_DIR}/serialEventUSB1.cpp
  ${TEENSY_DIR}/serialEventUSB2.cpp
  ${TEENSY_DIR}/sm_alloc_valid.c
  ${TEENSY_DIR}/sm_calloc.c
  ${TEENSY_DIR}/sm_free.c
  ${TEENSY_DIR}/sm_hash.c
  ${TEENSY_DIR}/sm_malloc.c
  ${TEENSY_DIR}/sm_malloc_stats.c
  ${TEENSY_DIR}/sm_pool.c
  ${TEENSY_DIR}/sm_realloc.c
  ${TEENSY_DIR}/sm_realloc_i.c
  ${TEENSY_DIR}/sm_realloc_move.c
  ${TEENSY_DIR}/sm_szalloc.c
  ${TEENSY_DIR}/sm_util.c
  ${TEENSY_DIR}/sm_zalloc.c
  ${TEENSY_DIR}/startup.c
  ${TEENSY_DIR}/tempmon.c
  ${TEENSY_DIR}/usb.c
  ${TEENSY_DIR}/usb_desc.c
  ${TEENSY_DIR}/usb_flightsim.cpp
  ${TEENSY_DIR}/usb_inst.cpp
  ${TEENSY_DIR}/usb_joystick.c
  ${TEENSY_DIR}/usb_keyboard.c
  ${TEENSY_DIR}/usb_midi.c
  ${TEENSY_DIR}/usb_mouse.c
  ${TEENSY_DIR}/usb_mtp.c
  ${TEENSY_DIR}/usb_rawhid.c
  ${TEENSY_DIR}/usb_seremu.c
  ${TEENSY_DIR}/usb_serial.c
  ${TEENSY_DIR}/usb_serial2.c
  ${TEENSY_DIR}/usb_serial3.c
  ${TEENSY_DIR}/usb_touch.c
  ${TEENSY_DIR}/yield.cpp
  ${USB_DIR}/SerEMU.cpp
  ${USB_DIR}/adk.cpp
  ${USB_DIR}/antplus.cpp
  ${USB_DIR}/digitizer.cpp
  ${USB_DIR}/ehci.cpp
  ${USB_DIR}/enumeration.cpp
  ${USB_DIR}/hid.cpp
  ${USB_DIR}/hub.cpp
  ${USB_DIR}/memory.cpp
  ${USB_DIR}/mouse.cpp
  ${USB_DIR}/print.cpp
  ${USB_DIR}/rawhid.cpp
  ${USB_DIR}/serial.cpp
  ${WIRE_DIR}/Wire.cpp
  ${WIRE_DIR}/WireIMXRT.cpp
  ${WIRE_DIR}/WireKinetis.cpp
)

add_library(libs STATIC ${SOURCES})

target_compile_options(libs PRIVATE -Wno-all -Wno-extra -Wno-pedantic)
target_include_directories(libs PRIVATE ${ADC_DIR})
target_include_directories(libs PRIVATE ${GFX_DIR})
target_include_directories(libs PRIVATE ${MODULES_DIR}/Adafruit_BusIO)
target_include_directories(libs PRIVATE ${MODULES_DIR}/SdFat/src)
target_include_directories(libs PRIVATE ${SPI_DIR})
target_include_directories(libs PRIVATE ${TEENSY_DIR})
target_include_directories(libs PRIVATE ${WIRE_DIR})

