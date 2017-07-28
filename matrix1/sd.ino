/* === Error messages === */
prog_char ERROR_SD_BootImageNotFound[] PROGMEM = "02 Img not found";
prog_char ERROR_SD_ImageLengthMismatch[] PROGMEM = "03 Img corrupt";

/* === includes === */
#include "SdFat.h"

/* === global variables === */
SdFatSdio sdEx; // handler for sd card
File file; // file handle
uint8_t sd_read_buffer[8192]; // buffer to read a file into
size_t nb = 8192; // exptected size of an image

/* === global functions === */
void error(const char text[]); // show error

/* === local functions === */

int sd_init() {
  if (!sdEx.begin()) {
    return -1;
  }
  sdEx.chvol();

  return 0;
}


int sd_loadBootImage(uint8_t buf[], size_t buffer_length) {
  if (!file.open("boot.pic", O_READ)) {
    error(ERROR_SD_BootImageNotFound);
    return -1;
  }

  if ((int)nb != file.read(buf, buffer_length)) {
    error(ERROR_SD_ImageLengthMismatch);
    return -2;
  }

  return 0;
}

