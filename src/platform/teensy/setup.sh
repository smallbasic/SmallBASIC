#!/bin/bash

DIR=`pwd`

(mkdir -p build/modules && cd build/modules && \
	git clone https://github.com/PaulStoffregen/cores.git && \
	git clone https://github.com/PaulStoffregen/SPI.git && \
	git clone https://github.com/PaulStoffregen/Wire.git && \
	git clone https://github.com/PaulStoffregen/USBHost_t36.git && \
	git clone https://github.com/PaulStoffregen/SdFat.git && \
	git clone https://github.com/PaulStoffregen/teensy_loader_cli.git && \
	git clone https://github.com/pedvide/ADC && \
	git clone https://github.com/ARM-software/CMSIS_5.git && \
	git clone https://github.com/ARM-software/CMSIS-DSP.git && \
	git clone https://github.com/adafruit/Adafruit_SSD1306.git && \
	git clone https://github.com/adafruit/Adafruit-GFX-Library && \
	git clone https://github.com/adafruit/Adafruit_BusIO)

(cd build/modules/CMSIS-DSP && \
    mkdir -p build && \
    cd build && \
    cmake .. -Wno-dev -DARM_CPU=cortex-m7 -DUSE_FPU=ON -DFLOAT_ABI=hard -DCMAKE_C_FLAGS="-I${DIR}/build/modules/CMSIS_5/CMSIS/Core/Include" && \
    cmake --build .)

(cd build/modules/teensy_loader_cli && make)

#
# (mkdir -p build && cd build && cmake .. && make)
#
