#!/usr/bin/env python

import sys
import math
import time
import serial


#ser = serial.Serial('/dev/cu.usbmodem641', 9600, timeout=5)
ser = serial.Serial('COM7', 9600, timeout=5)

def send(v):
    out = bytes(chr(65+v), "ascii")
    print("send %s" % out);
    ser.write(out)
    ser.flush()

def steuereAnzeige(restdauer, dauerJeLed):
    roteLED = 0
    restLED = 14
    if restdauer > 14:
        roteLED = math.ceil((restdauer - 14) / dauerJeLed)
    else:
        restLED = restdauer

    print("....x....x....x....x....")
    g = ""
    for i in range(1,25):

        if i < 15 and restLED > (i-1):
            if i < 5:
                g += "ü"
            else:
                g += "y"
        if i > 14 and (i-15) < roteLED:
            g += "o"

    print (g);
    send(roteLED + restLED)

d = 2.4
r = 38

steuereAnzeige(int(sys.argv[1]), float(sys.argv[2]))
