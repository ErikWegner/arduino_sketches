uint8_t stringsplit(String s) {
  uint8_t bytes_read = 0;
  uint8_t stringlength = s.length();
  if (stringlength % 2 == 1) {
#if DEBUG == 1
    Serial.println(F("stringsplit for ") + s + F(" failed"));
#endif
    return 0;
  }
  for (uint8_t i = 0; i < stringlength; i += 2) {
    serialdatabuffer[bytes_read] = stringsplitToHex(
      s.charAt(i),
      s.charAt(i+1)
      );
    bytes_read++;
  }

#if DEBUG == 1
  Serial.print(F("bytesRead: "));
  Serial.println(bytes_read, DEC);
#endif
  
  return bytes_read;
}

int8_t stringsplitCharTo4b(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return -1;   // getting here is bad: it means the character was invalid
  }
}

uint8_t stringsplitToHex(char c1, char c2)
{
  byte a = stringsplitCharTo4b(c1);
  byte b = stringsplitCharTo4b(c2);
  if (a < 0 || b < 0) {
    return -1;  // an invalid hex character was encountered
  } else {
    return a * 16 + b;
  }
}
