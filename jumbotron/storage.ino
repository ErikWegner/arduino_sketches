#include <SdFat.h>
#include <SdCard/SdioCardEX.cpp>
#include <SdCard/SdioTeensy.cpp>
#include <FatLib/FatFile.cpp>
#include <FatLib/FatVolume.cpp>
#include <FatLib/FatFileSFN.cpp>
#include <FatLib/FatFileLFN.cpp>

#define sdErrorPrintAndReturn sdEx.errorPrint(&Serial); return;

SdFatSdioEX sdEx;

SdFile dirFile;
SdFile file;

void sdListDirectory() {
  if (!sdEx.begin()) {
    sdEx.errorPrint(&Serial);
    return;
  };

  if (!dirFile.open("/", O_READ)) {
    sdErrorPrintAndReturn
  }

  while (file.openNext(&dirFile, O_READ)) {
    if (!file.isSubDir() && !file.isHidden()) {

      // Print the file number and name.
      Serial.print(": ");
      file.printName(&Serial);
      Serial.println();
    }
    file.close();
  }
}

