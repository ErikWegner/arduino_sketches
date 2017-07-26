#include "SdFat.h"

SdFatSdio sdEx;


int sd_init() {
  if (!sdEx.begin()) {
    return -1;
  }
  sdEx.chvol();

  return 0;
}

