#!/bin/bash

declare -a IMAGE_FILES=(\
 "proicons-cut.png"\
 "proicons-copy.png"\
 "proicons-clipboard-paste.png"\
 "proicons-save.png"\
 "proicons-bug-play.png"\
 "proicons-book-info-2.png"\
 "proicons-backspace.png"\
 "proicons-arrow-enter.png"\
 "proicons-search.png"\
)

echo "#pragma once" > keypad_icons.h
echo "// https://procode-software.github.io/proicons/icons" >> keypad_icons.h
for imageFile in "${IMAGE_FILES[@]}"
do
    name=`ls -1 ${imageFile} | sed -e 's/proicons-/img-/' | sed -e 's/\.png//'`
    xxd -n ${name} -i ${imageFile}  >> keypad_icons.h
done

cp keypad_icons.h ../../src/ui

