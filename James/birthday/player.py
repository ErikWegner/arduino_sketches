#!/usr/bin/env python

import sys
import math
import time
import serial
from PIL import Image # pacman -S python-pillow
import array

# python -m serial.tools.list_ports
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=5)
#ser = serial.Serial('COM7', 9600, timeout=5)

# Graphics
redimg = Image.open("happy.png")
redtext = redimg.load()
greenimg = Image.open("birthday.png")
greentext = greenimg.load()

# Internals

maxtrixwidth = 8
redposition = 0
greenposition = 0
redlength = redimg.size[0] - maxtrixwidth
greenlength = greenimg.size[0] - maxtrixwidth
looplength = max(redlength, greenlength)

# loop
def loop(position, image = 3):
    redposition = redlength * position / looplength
    greenposition = greenlength * position / looplength
    lines = array.array('H', [0, 0, 0, 0, 0, 0, 0, 0])
    for x in range(0,maxtrixwidth):
        for y in range(0,maxtrixwidth):
            redHigh = redtext[redposition + x, y]
            greenHigh = greentext[greenposition + x, y]
            color = (greenHigh * 2 + redHigh) & image
            # Turn on green LED.
            if (color == 2):
                lines[y] |= 1 << x; lines[y] &= ~(1 << (x+8));
            # Turn on red LED.
            if (color == 1):
                lines[y] |= 1 << (x+8);lines[y] &= ~(1 << x);
            # Turn on green and red LED.
            if (color == 3):
                lines[y] |= (1 << (x+8)) | (1 << x);
    data = "MDATA" + ','.join([str(line) for line in lines]) + ";"
    ser.write(data.encode('ascii'))
    ser.flush()

while(True):
    waiting = 1 / 20
    for image in [1,2, 3]:
        for position in range(0, looplength):
            loop(position, image)
            time.sleep(waiting)
            
