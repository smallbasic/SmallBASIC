#!/bin/bash

# sudo apt install librsvg2-bin

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
 "arrow-left"\
 "arrow-right"\
 "arrow-down"\
 "arrow-download"\
 "arrow-up" \
 "arrow-upload" \
 "tag"
)

echo "#pragma once" > keypad_icons.h
echo "// https://procode-software.github.io/proicons/icons" >> keypad_icons.h
for imageFile in "${IMAGE_FILES[@]}"
do
    echo $imageFile
    curl -sLO https://raw.githubusercontent.com/ProCode-Software/proicons/refs/heads/main/icons/svg/${imageFile}.svg

    # set width, height, and fill
    sed -i 's/width="24" height="24"/width="50" height="50"/' ${imageFile}.svg
    sed -i 's/currentColor/#ffffff/g' ${imageFile}.svg

    # convert svg to png using ImageMagick
    # convert -background none ${imageFile}.svg ${imageFile}.png
    rsvg-convert -o ${imageFile}.png ${imageFile}.svg

    # convert png to byte array
    xxd -n img_${imageFile} -i ${imageFile}.png  >> keypad_icons.h
done

mv keypad_icons.h ../src/ui

