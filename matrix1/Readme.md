This code uses a Teensy 3.5 and a 64x32 RGB matrix panel.

On starting up, the code tries to read the file `boot.pic` from the sd card and show the image on the panel.
The image is created using GIMP export _raw image data (*.data)_. The file uses 4 bytes per pixel: red, green, blue and a separator.

Afterwards, it tries to load boot.txt and execute the commands.

# Comands

## LABEL

LABEL declares a position within a script.

Example: `LABEL:Start`

## GOTO

GOTO jumps to a LABEL.

Example: `GOTO:Start`

## SHOW

The SHOW command loads an image and displays it.

Example: `SHOW:image1.data`

## SLEEP

To wait a definite amount of time, use the SLEEP command.

Example: `SLEEP:1s` to wait one second. `SLEEP:300ms` to wait 300 milliseconds.
