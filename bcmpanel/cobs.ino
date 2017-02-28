// http://www.jacquesf.com/2011/03/consistent-overhead-byte-stuffing/
size_t cobs_decode(const uint8_t * input, size_t length, uint8_t * output, const uint8_t delim)
{
  size_t read_index = 0;
  size_t write_index = 0;
  uint8_t code;
  uint8_t i;

  while (read_index < length)
  {
    code = input[read_index];

    if (read_index + code > length && code != 1)
    {
      return 0;
    }

    read_index++;

    for (i = 1; i < code; i++)
    {
      output[write_index++] = input[read_index++];
    }
    if (code != 0xFF && read_index != length)
    {
      //output[write_index++] = '\0';
      output[write_index++] = delim;
    }
  }

  return write_index;
}

size_t frdecode(String in_data, uint8_t* out_data, uint8_t out_data_size) {
  uint8_t cobs_decoder_buffer1[128];
  uint8_t cobs_decoder_buffer2[128];

  in_data.getBytes(cobs_decoder_buffer1, min(1 + in_data.length(), sizeof(cobs_decoder_buffer1)));
  return cobs_decode(cobs_decoder_buffer1, min(in_data.length(), out_data_size), out_data, 0);
  
//  Serial.print(F("frdec "));
//  Serial.print(in_data.length(), DEC);
//  Serial.print(F(" a"));
//  Serial.print(cobs_decoder_buffer1[97], HEX);
//  Serial.print(F(" b"));
  size_t l = cobs_decode(cobs_decoder_buffer1, in_data.length(), cobs_decoder_buffer2, 59);
//  Serial.print(l, DEC);
//  Serial.print(F(" g"));
//  Serial.print(min(l, out_data_size), DEC);
//  Serial.print(F(" c"));
//  Serial.print(cobs_decoder_buffer2[96], HEX);
  l = cobs_decode(cobs_decoder_buffer2, min(l, out_data_size), out_data, 0);
//  Serial.print(F(" d"));
//  Serial.print(l, DEC);
//  Serial.print(F(" e"));
//  Serial.print(out_data_size, DEC);
//  Serial.print(F(" f"));
//  Serial.print(out_data[95], HEX);
//  Serial.print(F(" h"));
//  Serial.print(in_data.charAt(97), HEX);
//  Serial.println();
  
  return l;
}

