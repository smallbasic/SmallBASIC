#!/bin/bash

declare -a IMAGE_FILES=(\
 "cut"\
 "copy"\
 "clipboard-paste"\
 "save"\
 "bug-play"\
 "book-info-2"\
 "backspace"\
 "arrow-enter"\
 "search"\
 "layers"\
 "arrow-down"\
 "arrow-download"\
 "arrow-up" \
 "arrow-upload"
)

echo "#pragma once" > keypad_icons.h
echo "// https://procode-software.github.io/proicons/icons" >> keypad_icons.h
for imageFile in "${IMAGE_FILES[@]}"
do
    curl -sLO https://raw.githubusercontent.com/ProCode-Software/proicons/refs/heads/main/icons/svg/${imageFile}.svg

    # set width, height, and fill
    sed -i 's/width="24" height="24"/width="30" height="30"/' ${imageFile}.svg
    sed -i 's/currentColor/#ffffff/g' ${imageFile}.svg

    # convert svg to png using ImageMagick
    convert -background none ${imageFile}.svg ${imageFile}.png

    # convert png to byte array
    xxd -n img_${imageFile} -i ${imageFile}.png  >> keypad_icons.h
done

mv keypad_icons.h ../../src/ui

