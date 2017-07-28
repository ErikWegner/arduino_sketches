This code uses a Teensy 3.5 and a 64x32 RGB matrix panel.

On starting up, the code tries to read the file `boot.pic` from the sd card and show the image on the panel.
The image is created using GIMP export _raw image data (*.data)_. The file uses 4 bytes per pixel: red, green, blue and a separator.
