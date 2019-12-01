# Smarthome

## Build

    make SMING_ARCH=Esp8266 ENABLE_SSL=1 COM_SPEED_ESPTOOL=115200 -j9

## Flashing

    # First time only
    # make SPI_MODE=dio SPI_SIZE=4M COM_SPEED_ESPTOOL=115200 flashinit
    make SPI_MODE=dio SPI_SIZE=4M COM_SPEED_ESPTOOL=115200 flash

 