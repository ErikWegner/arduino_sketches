import serial
from datetime import datetime
import time

# python -m serial.tools.list_ports
ser = serial.Serial('/dev/cu.usbmodem641', 57600, timeout=5)


line = ser.readline()
print(line);
while (True):
	now = datetime.now()

	led_hour = now.hour % 12
	led_minute = int(now.minute / 5)
	led_second = int(now.second / 5) 
	
	led_sec_intensity = 4 + 3*(now.second % 5)
	
	#print("%i,%i,%i,%i" % (led_hour, led_minute, led_second, led_sec_intensity));

	ser.write("LED 6,%i;" % (now.second % 2)); ser.flush()
	if led_hour == led_minute and led_minute == led_second:
		ser.write("ring %i,16,16,%i;" % (led_hour, led_sec_intensity)); ser.flush()
	else:
		if led_hour == led_minute:
			ser.write("ring %i,16,16,0;" % led_hour); ser.flush()
			ser.write("ring %i,0,0,%i;" % (led_second, led_sec_intensity)); ser.flush()
		else:
			if led_hour == led_second:
				ser.write("ring %i,16,0,%i;" % (led_hour, led_sec_intensity)); ser.flush()
				ser.write("ring %i,0,16,0;" % led_minute); ser.flush()
			else:
				if led_minute == led_second:
					ser.write("ring %i,16,0,0;" % led_hour); ser.flush()
					ser.write("ring %i,0,16,%i;" % (led_minute, led_sec_intensity)); ser.flush()
				else:
					ser.write("ring %i,16,0,0;" % led_hour); ser.flush()
					ser.write("ring %i,0,16,0;" % led_minute); ser.flush()
					ser.write("ring %i,0,0,%i;" % (led_second, led_sec_intensity)); ser.flush()
	ser.write("ring show;"); ser.flush()
	time.sleep(1);
	ser.write("ring %i,0,0,0;" % led_hour); ser.flush()
	ser.write("ring %i,0,0,0;" % led_minute); ser.flush()
	ser.write("ring %i,0,0,0;" % led_second); ser.flush()
