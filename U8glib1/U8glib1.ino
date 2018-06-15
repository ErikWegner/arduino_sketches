#include <U8glib.h>

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_FAST); // Dev 0, Fast I2C / TWI

void setup(void) {
  u8g.setFont(u8g_font_5x8); // font instellen.
}

void loop(void) {
  // picture loop
  u8g.firstPage();  
  do {
    u8g.drawStr(0, 10, "Hallo Wereld!"); // print tekst.
  } while( u8g.nextPage() );

  delay(100); // rebuild the picture after some delay
}
