# SmallBASIC for teensy mcu
# Copyright(C) 2025 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

tidy:
	@cd build && make tidy

build/modules/cores/teensy4/Makefile:
	./setup.sh

build/Makefile: build/modules/cores/teensy4/Makefile
	@mkdir -p build
	@cd build && cmake ..

src/main_bas.h: main.bas
	@xxd -i $< > $@

build/smallbasic.elf : \
	build/Makefile \
	src/noop.c \
	src/main.cpp \
	src/device.cpp \
	src/teensy.cpp \
	src/ssd1306.cpp \
	src/main_bas.h
	@cd build && make -j 32

all: build/smallbasic.elf

install: build/smallbasic.elf
	@build/modules/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -w -v -s build/smallbasic.hex && \
	sleep 1 && \
	lsusb | grep -i teensy

install40: build/smallbasic.elf
	@build/modules/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY40 -w -v -s build/smallbasic.hex && \
	sleep 1 && \
	lsusb | grep -i teensy
