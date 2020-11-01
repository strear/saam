#!/bin/bash

clear
echo  
echo For complete pictures in terminal emulators:
echo  
echo  1. Maximize this window;
echo  2. Scoll the mouse wheel with Ctrl pressed, or use
echo     the trackpad or touch screen to zoom the text.
echo  
echo Press any key to start.
read -s -n 1
echo

WORK_DIR="$(dirname "${BASH_SOURCE[0]}")"
if ! [ -f "$WORK_DIR/imglist.txt" ]; then
  find "$WORK_DIR/*.jpg" -printf "%f\n" > "$WORK_DIR/imglist.txt"
fi
params=":$WORK_DIR/imglist.txt --interval=66 --media=$WORK_DIR/wave.mp3 $*"

echo -ne "\e]2;Where do you want to go today? ('95)\a"
"$WORK_DIR/../make/posix/saam" $params

echo -e '\e[0mPress Enter to exit. The script will quit in 30 seconds.'
read -s -t 30