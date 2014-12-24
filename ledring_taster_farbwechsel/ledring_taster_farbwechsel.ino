#include <Adafruit_NeoPixel.h>

#define buttonPin   8    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    9    // Digital IO pin connected to the NeoPixels.
#define INTENSITY 16
#define PIXEL_COUNT 12

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = LOW;
int lightposmax = PIXEL_COUNT * 5;
int lightpos = 0;
int r = INTENSITY, g = INTENSITY, b = INTENSITY;
int bi = 0;

void setup() {
  pinMode(buttonPin, INPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
//  Serial.begin(9600);

}
boolean buttonPressed = false;

void loop() {

/*  // Get current button state.
  bool newState = digitalRead(BUTTON_PIN);
  
  // Check if state changed from high to low (button press).
  if (newState == HIGH && oldState == LOW) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == HIGH) {*/
    if ((digitalRead(buttonPin)==HIGH)&&(buttonPressed==false)){
    buttonPressed=true;
  } 
  if (digitalRead(buttonPin)==LOW&&(buttonPressed==true)){
    buttonPressed=false;
      bi = (bi + 1) % 7;
      
      switch (bi) {
        case 0: r=INTENSITY; g=INTENSITY; b=INTENSITY; break;
        case 1: r=INTENSITY; g=0;   b=0;   break;
        case 2: r=INTENSITY; g=INTENSITY; b=0;   break;
        case 3: r=0;   g=INTENSITY; b=0;   break;
        case 4: r=0;   g=INTENSITY; b=INTENSITY; break;
        case 5: r=0;   g=0;   b=INTENSITY; break;
        case 6: r=INTENSITY; g=0;   b=INTENSITY; break;
      }
    }
  

  // Set the last button state to the old state.
//  oldState = newState;


  lightpos = (lightpos + 1) % lightposmax;
  int subpos = lightpos % 5;
  
  /*
  
  0  40
  1  80
  2  80
  3  40
  4  0
  
  */
       //strip.setPixelColor(lightpos / 5, r, g, b);
       
  if (subpos == 4) {
    strip.setPixelColor(lightpos / 5, 0);
  } else {

    
    if (subpos == 1 || subpos == 2) {
      strip.setPixelColor(lightpos / 5, r * 0.8, g * 0.8, b * 0.8);
    } else {
      strip.setPixelColor(lightpos / 5, r * 0.4, g * 0.4, b * 0.4);
    }
  }
  
  strip.show();
  delay(5);

}

