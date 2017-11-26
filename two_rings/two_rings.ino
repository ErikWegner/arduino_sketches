#include <TimerOne.h>

#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN 9 // Digital IO pin connected to the NeoPixels.
#define PIXEL_PER_RING 12
#define PIXEL_COUNT 24
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint8_t rot_gruen[] = {B00110000,B00110000,B00110000,B00110000,B00110000,B00110000,
B00001100,B00001100,B00001100,B00001100,B00001100,B00001100};
uint8_t blue[] = {B00110100,B00110100,B00110100,B00110100,B00110100,B00110100,
B00000111,B00000111,B00000111,B00000111,B00000111,B00000111};

int volatile p = 0;
int volatile array_switch = 0;
int volatile brightness = 16;
uint8_t volatile *cp = rot_gruen;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Timer1.initialize(60000);
  Timer1.attachInterrupt(gears);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void gears() {
  uint8_t a = *(cp+p);
  for (uint8_t b = 1; b <= PIXEL_PER_RING; b++) {
    uint8_t c = *(cp + ((p+b)%(PIXEL_PER_RING)));
    if (c != a) {
      strip.setPixelColor((b)%(PIXEL_PER_RING), ((c & B00110000) >> 4) * brightness, ((c & B00001100) >> 2) * brightness, ((c & B00000011)) * brightness);
      strip.setPixelColor(PIXEL_COUNT - 1 - ((b + PIXEL_PER_RING / 2 + 11)%(PIXEL_PER_RING)), ((c & B00110000) >> 4) * brightness, ((c & B00001100) >> 2) * brightness, ((c & B00000011)) * brightness);
    }
    a = c;
  }
  p = p + 1;
  if (p >= PIXEL_PER_RING) {
    p = 0;
    array_switch = array_switch + 1;
    Serial.println(array_switch);
  }
  if (array_switch == 10) {
    cp = blue;
  }
  if (array_switch >= 20) {
    cp = rot_gruen;
    array_switch = 0;
  }
  strip.show();
}

