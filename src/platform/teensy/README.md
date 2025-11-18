# Build

## Build SmallBASIC

Download and build SmallBASIC as described [here](https://github.com/smallbasic/SmallBASIC).

## Configure Tennsy version

Tennsy 4 and Tennsy 4.1 are supported. Both using the same processor and running at the same speed.
But Tennsy 4.1 offers more features than Tennsy 4. Currently, you have to configure two files depending
which MCU you use.

1. `src/platform/teensy/Makefile.am` 
   `--mcu=TENNSY4` or `--mcu=TENNSY41`

   ```
   install: build/smallbasic.elf
     @build/modules/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -w -v -s build/smallbasic.hex && \
     sleep 1 && \
     lsusb | grep -i teensy
   ```

2. `src/platform/teensy/CMakeLists.txt`
   Comment/Uncomment settings for Teensy 4 / Tennsy 4.1

   ```
   # settings for teensy 4.0
   #set(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
   #set(TEENSY_SRC ${MODULES}/cores/teensy4)
   #set(MCU "IMXRT1062")
   #set(MCU_LD ${TEENSY_SRC}/imxrt1062.ld)
   #set(MCU_DEF "ARDUINO_TEENSY4")
   #set(CPU_OPTIONS "-mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb")

   # settings for teensy 4.1
   set(MODULES ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
   set(TEENSY_SRC ${MODULES}/cores/teensy4)
   set(MCU "IMXRT1062")
   set(MCU_LD ${TEENSY_SRC}/imxrt1062_t41.ld)
   set(MCU_DEF "ARDUINO_TEENSY41")
   set(CPU_OPTIONS "-mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb")
   ```

## Build firmware in Manjaro (arch)

```
$ sudo pacman -S arm-none-eabi-binutils arm-none-eabi-gcc arm-none-eabi-newlib libusb-compat
$ ./configure --enable-teensy
$ cd src/platform/teensy
$ ./setup.sh
$ make
```

> If setup.sh displays an error massage that cmake minimum version is not set,
> open `build/modules/CMSIS-DSP/CMakeLists.txt`
> and add in the beginning of the file `cmake_minimum_required(VERSION 4.1)`

## Upload the firmware

```
make install
```

or

```
build/modules/teensy_loader_cli/teensy_loader_cli --mcu=TEENSY41 -w -v -s build/smallbasic.hex
```

# Run your SMALLBASIC program

SMALLBASIC for Teensy offers three ways to upload and run a program.

1. Format a SD card using FAT32. Rename your program to `MAIN.BAS` and copy it to the SD card.
2. Include your program in the firmware. Replace `main.bas` in `src/platform/teensy` by your program and build
   the firmware.
3. Send your program via USB-serial connection to the Teensy. In Linux use ` cat YourProgram.bas > /dev/ttyACM0`
   Change `/dev/ttyACM0` to the USB-serial port of your Teensy.
4. Goto 3.

When the Teensy starts up, it will check in the above indicated order for your program. If it finds
a SD card and the SD card contains a file `MAIN.BAS`, it will execute it. Otherwise it will check, if a program
was included in the firmware. If no program was included, the Teensy will switch to interactive mode and waits
for a program upload via USB-serial.

While your program is running, the Teensy will check continuously if data is available at the USB-serial port.
If data is available for longer than one second your running program will be terminated and the queued data of the
USB-serial port will be interpreted as the new program. Once the upload is finished, the new program will be executed.
If you are using the USB-serial port for communication, read the queued data within one second. Alternately, you
can turn on/off this behavior during runtime.

If an error occurred, for example a syntax error, execution will stop and you have to read the error message from
the USB-serial port.

If the execution of your program comes to an end, for example when reaching the end of the program or when `STOP` is
called, the program is terminated and the next section in the above list is performed.

# Read output from your running program

The `PRINT` command can be used to print to the USB-serial port. To access the output, connect to the serial
port using, i.e. `PUTTY` or any other serial-port monitor / terminal. Using Linux, the easiest way is to run
`cat /dev/ttyACM0`
