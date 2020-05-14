# Smarthome

## Build

    make config-clean
    make SMING_ARCH=Esp8266 ENABLE_SSL=1 ENABLE_CUSTOM_LWIP=2 COM_SPEED_ESPTOOL=115200 -j9

## Flashing

    # First time only
    # make SPI_MODE=dio SPI_SIZE=4M COM_SPEED_ESPTOOL=115200 flashinit
    make SPI_MODE=dio SPI_SIZE=4M COM_SPEED_ESPTOOL=115200 flash

## Connect to serial console

    python -m serial.tools.miniterm --raw --encoding ascii /dev/ttyUSB0 115200
