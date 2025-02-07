#!/bin/bash

#
# Allows you to edit your SmallBASIC code in your favourite editor.
# When you save, the program is transferred to the teensy and run.
#
# This works with the INTERACTIVE mode build
#   $ cd build && make .. -DINTERACTIVE=ON
#
# Requires inotify-tools available via:
#   $ sudo apt install inotify-tools
#
# Example usage:
#   $ ./run.sh myapp.bas
#

if [ x$1 == "x" ]; then
    echo "usage: edit program.bas"
    exit
fi

if [ ! -e $1 ]; then
    echo "Program file not found:" $1
    exit
fi

if [ ! -c /dev/ttyACM0 ]; then
    echo "Teensy not found"
    exit
fi

function update() {
    echo "Sending" $1
    cat $1 > /dev/ttyACM0
    timeout 5 cat /dev/ttyACM0
}

update $1

echo "Waiting for changes"
while inotifywait -e modify $1; do
   echo "Pushing changes"
   update $1
done
