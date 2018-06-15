# IoT Scale

A scale with cloud logging, inspired by ct.de/yj2t

## Flashing

* Build a firmware image at https://nodemcu-build.com/. Use the master branch and includes the following modules: file, gpio, http, hx711, i2c, net, node, tmr, u8g, uart, wifi, tls. Enable fonts: font_6x10,font_gdb25n
* Download the `float.bin`.
* Download [esptool](https://github.com/espressif/esptool)
* Flash image: `esptool.py --port /dev/ttyUSB0 write_flash -fm dio 0x00000 nodemcu-master-10-modules-YYYY-MM-DD-hh-mm-ss-float.bin`

To remove the main script, execute `file.remove("init.lua");` in [ESPlorer](https://esp8266.ru/esplorer/)

## Wiring

* Baseboard: Nodemcu
* Pin D5: HX711 Clock
* Pin D6: HX711 Data
* Pin D1: OLED i2c Clock
* Pin D2: OLED i2c Data
