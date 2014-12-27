#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 9 // Digital IO pin connected to the NeoPixels.
#define TONE_PIN 3
#define PIXEL_COUNT 12
#define DEFAULT_DELAY 100
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
int ledPin_R1 = 11;
int ledPin_R2 = 5;
int ledPin_Y1 = 12;
int ledPin_Y2 = 6;
int ledPin_G1 = 13;
int ledPin_G2 = 7;

void setup() {
  pinMode(ledPin_R1,OUTPUT);
  pinMode(ledPin_R2,OUTPUT);
  pinMode(ledPin_Y1,OUTPUT);
  pinMode(ledPin_Y2,OUTPUT);
  pinMode(ledPin_G1,OUTPUT);
  pinMode(ledPin_G2,OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  alloff();
  delay(1000);
  pixel1test();
  theaterChase(strip.Color(127, 127, 127), 75, 6); // White
  buildorangecircle();
  theaterChase(strip.Color(127, 64, 0), 50, 6); // White
  alloff();
  greenpower();
  delay(250);
}

void buildorangecircle() {
  // init
  int q = 2;
  int blue1pos = 1;
  int blue2pos = 0;
  int numorange = 0;
  int whiteloops = 25;
  
  int intensityloop = 0;
  int intensity = 8;
  
  // loop
  while(numorange < 5) {
    intensityloop = (intensityloop + 10) % 180;
    intensity = 12 * sin(intensityloop * 3.14 / 180.0) + 8;
    for (int i=0; i < PIXEL_COUNT; i=i+3) {
      strip.setPixelColor((i+q), intensity, intensity, intensity);    //turn every third pixel on
    }

    if (q == 0) {
      if (whiteloops > 0) whiteloops--;
      if (whiteloops == 0) {
        blue1pos = (blue1pos + 1) % PIXEL_COUNT;
        blue2pos = (blue2pos + 1) % PIXEL_COUNT;
        if (blue1pos == (9 + numorange * 3) % PIXEL_COUNT) {
          numorange++;
        }
      }
    }

    if (whiteloops == 0) {
      strip.setPixelColor(blue1pos, 0x0f, 0x24, 0x3e);
      strip.setPixelColor(blue2pos, 0x00, 0x00, 0xcc);
      for (int o=0; o < numorange; o++) {
        strip.setPixelColor((8 + o * 3) % PIXEL_COUNT, 0xe3, 0x6c, 0x09);
      }
    }

    strip.show();
    delay(75);

    for (int i=0; i < PIXEL_COUNT; i=i+3) {
      strip.setPixelColor(i+q, 0);        //turn every third pixel off
    }
    q = (q + 2) % 3;
  }
}

void all_LED_off() {
  all_LED(LOW, LOW, LOW, LOW, LOW, LOW);
}

void all_LED(bool s1, bool s2, bool s3, bool s4, bool s5, bool s6) {
  digitalWrite(ledPin_R1, s1);
  digitalWrite(ledPin_R2, s2);
  digitalWrite(ledPin_Y1, s3);
  digitalWrite(ledPin_Y2, s4);
  digitalWrite(ledPin_G1, s5);
  digitalWrite(ledPin_G2, s6);
}

void greenpower() {
  greenpower_rightturn();
  greenpower_leftturn();
  digitalWrite(ledPin_R1, HIGH);
  digitalWrite(ledPin_R2, HIGH);
  delay(DEFAULT_DELAY);
  greenpower_rightturn();
  greenpower_leftturn();
  digitalWrite(ledPin_Y1, HIGH);
  digitalWrite(ledPin_Y2, HIGH);
  delay(DEFAULT_DELAY);
  greenpower_rightturn();
  greenpower_leftturn();
  digitalWrite(ledPin_G1, HIGH);
  digitalWrite(ledPin_G2, HIGH);
  delay(DEFAULT_DELAY);

  greenpower_turnoneveryother();
  all_LED_off();
  greenpower_power();
}

void greenpower_power() {
  int i; 
  int p;
  for (p = 33; p < 128; p += 2) {
    for(i=3; i < PIXEL_COUNT; i+=2) {
      strip.setPixelColor(i, 0, p, 0);
    }
    strip.show();
    if (p%3==0) {
      tone(TONE_PIN, 26 + p * 4);
    }
    delay(10),
    noTone(TONE_PIN);
    delay(5);
  }
}

void greenpower_turnoneveryother() {
  int i;
  for(i=1; i < PIXEL_COUNT; i+=2) {
    strip.setPixelColor(i, 0, 32, 0);
    strip.show();
    tone(TONE_PIN, 262 + i * 40);
    delay(15),
    noTone(TONE_PIN);
    delay(DEFAULT_DELAY);
  }
}

void greenpower_rightturn() {
  int i;
  for(i=0; i < PIXEL_COUNT; i++) {
    strip.setPixelColor(i, 0, 32, 0);
    strip.show();
    tone(TONE_PIN, 262 + i * 40);
    delay(15),
    noTone(TONE_PIN);
    delay(30);
    strip.setPixelColor(i, 0, 0, 0);
    strip.show();
  }
}

void greenpower_leftturn() {
  int i;
  for(i=PIXEL_COUNT-1; i >= 0; i--) {
    strip.setPixelColor(i, 0, 32, 0);
    strip.show();
    tone(TONE_PIN, 262 + i * 40);
    delay(15),
    noTone(TONE_PIN);
    delay(30);
    strip.setPixelColor(i, 0, 0, 0);
    strip.show();
  }
}
void alloff() {
  strip.setPixelColor(0, 0x00, 0x00, 0x00);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait, uint8_t pixelskip) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < pixelskip; q++) {
      for (int i=0; i < strip.numPixels(); i=i+pixelskip) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+pixelskip) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void pixel1test() {
  strip.setPixelColor(0, 0xff, 0xff, 0xff);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0x00, 0x00, 0x00);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0x00, 0xb0, 0x50);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0x00, 0x00, 0x00);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0xc0, 0x00, 0x00);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0x00, 0x00, 0x00);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0x00, 0x00, 0xff);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
  strip.setPixelColor(0, 0x00, 0x00, 0x00);
  strip.setPixelColor(1, 0x00, 0x00, 0x00);
  strip.setPixelColor(2, 0x00, 0x00, 0x00);
  strip.setPixelColor(3, 0x00, 0x00, 0x00);
  strip.setPixelColor(4, 0x00, 0x00, 0x00);
  strip.setPixelColor(5, 0x00, 0x00, 0x00);
  strip.setPixelColor(6, 0x00, 0x00, 0x00);
  strip.setPixelColor(7, 0x00, 0x00, 0x00);
  strip.setPixelColor(8, 0x00, 0x00, 0x00);
  strip.setPixelColor(9, 0x00, 0x00, 0x00);
  strip.setPixelColor(10, 0x00, 0x00, 0x00);
  strip.setPixelColor(11, 0x00, 0x00, 0x00);
  strip.show();
  delay(DEFAULT_DELAY);
}












