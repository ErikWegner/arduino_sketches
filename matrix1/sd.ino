/* === Error messages === */
prog_char ERROR_SD_BootImageNotFound[] PROGMEM = "02 Img not found";
prog_char ERROR_SD_ImageLengthMismatch[] PROGMEM = "03 Img corrupt";

/* === includes === */
#include "SdFat.h"

/* === global variables === */
SdFatSdio sdEx; // handler for sd card
uint8_t sd_read_buffer[8192]; // buffer to read a file into
size_t nb = 8192; // exptected size of an image
File scriptfile; // file handle
bool scriptfileOpen = false;

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


int sd_loadImage(uint8_t buf[], size_t buffer_length, String filename) {
  File file; // file handle
  char filename2[64];
  filename.toCharArray(filename2, 64);
  if (!file.open(filename2, O_READ)) {
    error(ERROR_SD_BootImageNotFound);
    return -1;
  }

  if ((int)nb != file.read(buf, buffer_length)) {
    error(ERROR_SD_ImageLengthMismatch);
    return -2;
  }

  return 0;
}

void loadScriptFromSd(String filename) {
#if DEBUG == 1
  Serial.print("loadScriptFromSd(\"");
  Serial.print(filename);
  Serial.println("\")");
#endif

  char filename2[64];
  filename.toCharArray(filename2, 64);
  if (scriptfile.open(filename2)) {
    scriptfileOpen = true;
  }
}

String sd_loadNextLine() {
  if (scriptfileOpen) {
    const uint8_t linelength = 255;
    char linefeed = '\n';
    char line[linelength + 1];
    uint16_t bytescount = scriptfile.fgets(line, linelength, &linefeed);
    if (bytescount > 0) {
      return String(line);
    }
  }

  return 0;
}


