/*
 *   Copyright 2015 David Valeri
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#define UNSIGNED_LONG_MAX 4294967295

// Define clear bit macro.  Operates on a Special Registry Flag (SFR) and
// a bit offset in the SRF.
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

// Define set bit macro.  Operates on a Special Registry Flag (SFR) and
// a bit offset in the SRF.
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Config FFT ////////////////////////////////////////

// Configure FFT
// See http://wiki.openmusiclabs.com/wiki/Defines
#define LOG_OUT 1
#define FFT_N 256

#include <FFT.h>

// Application Constants /////////////////////////////

// The time, in milliseconds, for which a sample is averaged into the calculated
// smoothed level when the current sample is higher than the smoothed average.
const unsigned long RISING_LEVEL_W = 2500;

// The time, in milliseconds, for which a sample is averaged into the calculated
// smoothed level when the current sample is lower than the smoothed average.  
const unsigned long FALLING_LEVEL_W = 5000;

// Used internally to detect when a full set of samples have been collected for FFT analysis.
const unsigned int FFT_SAMPLE_COUNTER_THRESHOLD = FFT_N * 2;

// Used internally to detect when a full set of samples have been collected for adjuster evaluation.
const unsigned int ADJUST_SAMPLE_COUNTER_THRESHOLD = 5;

const unsigned int FFT_OUTPUT_BINS = FFT_N / 2;

// The OK display level.
const byte OK_LEVEL = 0;
// The INFO display level.
const byte INFO_LEVEL = 1;
// The WARN display level.
const byte WARN_LEVEL = 2;
// The WARN PLUS display level.
const byte WARN_PLUS_LEVEL = 3;
// The STFU DAVE display level.
const byte EMERG_LEVEL = 4;

const byte INPUT_SOURCE_LEVEL = 0;
const byte INPUT_SOURCE_ADJUSTER = 1;

const byte INPUT_SWITCH_SAMPLES_TO_IGNORE = 2;

const int THRESHOLD_DELTA = 100;
const int THRESHOLD_RELEASE_DELTA = 75;

const unsigned int DEFAULT_THRESHOLD = 10000;

//////////////////////////////////////////////////////

// The moving average of the input level.
volatile float smoothedLevel = 0;

// The current adjuster level reading.
volatile int adjusterLevel = 0;


// Current sample counter index.
unsigned short sampleCounter = 0;

byte fftCounter = 0;

// The last time, in millis since starup, that the input level was evaluated.
unsigned long previousSampleUpdateMillis = 0;

// A flag indicating if the next ADC result should be ignored.  Used to ignore
// indeterminate results since we don't concern ourselves with timing when switching
// the ADC MUX.
volatile boolean ignoreConversion = false;

// The current input source for the ADC.  Used to determine what we are currently
// taking readings for.
volatile byte inputSource = INPUT_SOURCE_ADJUSTER;


// The current display level.
byte displayLevel = OK_LEVEL;

// The last time, in milliseconds since starup, that the display level was evaluated.
unsigned long previousDisplayLevelUpdateMillis = 0;

// The time, in milliseconds, at which the display level will next be evaluated.
unsigned long displayLevelUpdateInterval = 0;

volatile unsigned int infoThreshold = DEFAULT_THRESHOLD;
volatile unsigned int infoReleaseThreshold = DEFAULT_THRESHOLD;

volatile unsigned int warnThreshold = DEFAULT_THRESHOLD;
volatile unsigned int warnReleaseThreshold = DEFAULT_THRESHOLD;

volatile unsigned int warnPlusThreshold = DEFAULT_THRESHOLD;
volatile unsigned int warnPlusReleaseThreshold = DEFAULT_THRESHOLD;

volatile unsigned int emergThreshold = DEFAULT_THRESHOLD;
volatile unsigned int emergReleaseThreshold = DEFAULT_THRESHOLD;

// Adafruit LED ring
#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 9 // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
volatile byte rotoPos = 0;

// 4DLED
#include "Wire.h"
#include "I2C_4DLED.h"
const uint8_t i2cAddressSAA1064 = 0x70;

#define TONE_PIN 11
#define NUM_TONES 3
byte tones = NUM_TONES;

void setup() {
  
  // Initialize the serial port so we can have debug logging.
  Serial.begin(115200);
  
  Serial.println(F(""));

  Wire.begin();

  // 4-Digit-LED-Anzeige mit I2C-Adresse des SAA1064 initialisieren
  FourDigitLedDisplay.begin(i2cAddressSAA1064);
  
  // alle im SAA1064 gespeicherten Digits löschen
  FourDigitLedDisplay.clearDisplay();
  FourDigitLedDisplay.setDisplayOutputCurrent(I2C_4DLED::OUTPUT_CURRENT_3_MA);
  // Segmenttest des Displays starten
  FourDigitLedDisplay.testDisplaySegments(1);
  
  // nach einer Sekunde den Segmenttest beenden
  delay(1000);
  FourDigitLedDisplay.testDisplaySegments(0);

  initAdc();

  cli();
  strip.begin();
  strip.show();
  Serial.println(F("############################ Initialized ############################"));
  sei();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Calculate the new display state //////////////////////////

  byte newDisplayLevel = displayLevel;

  // We must ensure that we are atomically reading in the values that we compare
  // against since they are written in the ISR and read here.  We don't want to
  // read a half written value so we ensure that we don't allow the interrupt to fire
  // while reading the values.
  cli();
  float currentSmoothedLevel = smoothedLevel;
  int currentInfoThreshold = infoThreshold;
  int currentInfoReleaseThreshold = infoReleaseThreshold;
  
  int currentWarnThreshold = warnThreshold;
  int currentWarnReleaseThreshold = warnReleaseThreshold;
  
  int currentWarnPlusThreshold = warnPlusThreshold;
  int currentWarnPlusReleaseThreshold = warnPlusReleaseThreshold;
  
  int currentErmergThreshold = emergThreshold;
  int currentErmergReleaseThreshold = emergReleaseThreshold;
  sei();

  if (currentSmoothedLevel > currentErmergThreshold) {
    newDisplayLevel = EMERG_LEVEL;
  } else if (currentSmoothedLevel > currentWarnPlusThreshold) {
    // Add a little historesis.
    if (displayLevel < WARN_PLUS_LEVEL
        || (displayLevel > WARN_PLUS_LEVEL && currentSmoothedLevel < currentErmergReleaseThreshold)) {
      newDisplayLevel = WARN_PLUS_LEVEL;
    }
  } else if (currentSmoothedLevel > currentWarnThreshold) {
    // Add a little historesis.
    if (displayLevel < WARN_LEVEL 
        || (displayLevel > WARN_LEVEL && currentSmoothedLevel < currentWarnPlusReleaseThreshold)) {
      newDisplayLevel = WARN_LEVEL;
    }
  } else if (currentSmoothedLevel > currentInfoThreshold) {
    // Add a little historesis.
    if (displayLevel < INFO_LEVEL 
        || (displayLevel > INFO_LEVEL && currentSmoothedLevel < currentWarnReleaseThreshold)) {
      newDisplayLevel = INFO_LEVEL;
    }
  } else if (currentSmoothedLevel < currentInfoReleaseThreshold) {
    newDisplayLevel = OK_LEVEL;    
  }

  if (newDisplayLevel != displayLevel) {
    displayLevel = newDisplayLevel;
    // If we are in a blinking state, don't immediately update the display
    // because the user then sometimes sees a flicker or a truncated
    // blink cycle during the transition.
    if (displayLevelUpdateInterval == UNSIGNED_LONG_MAX) {
      displayLevelUpdateInterval = 0;
    }

    cli();
    Serial.print(F("Chose new display level: "));
    Serial.println(displayLevel);
    //Serial.println(led4d);
    sei();

  }

  // Drive the display as needed /////////////////////////////////

  if (currentMillis - previousDisplayLevelUpdateMillis > displayLevelUpdateInterval) {

    previousDisplayLevelUpdateMillis = currentMillis;
    
    // Perhaps no need to check again until the display level changes.
    displayLevelUpdateInterval = UNSIGNED_LONG_MAX;
    
    if (displayLevel != EMERG_LEVEL) {
      tones = NUM_TONES;
    }
    
    switch (displayLevel) {
      default:
      case OK_LEVEL:
        showGreen();
        break;
      case INFO_LEVEL:
        showRoto(0x00, 0x20, 0x00);
        displayLevelUpdateInterval = 333;
        break;
      case WARN_LEVEL:
        showRoto(0x28, 0x20, 0x00);
        displayLevelUpdateInterval = 250;
        break;
      case WARN_PLUS_LEVEL:
        showRoto(0x40, 0x00, 0x00);
        displayLevelUpdateInterval = 150;
        break;
      case EMERG_LEVEL:
        showBlinkRed();
        displayLevelUpdateInterval = 200;
        break;
    }
    
    rotoPos = (rotoPos + 1) % PIXEL_COUNT;

    int16_t led4d = currentSmoothedLevel - 2500;
    FourDigitLedDisplay.writeDecimal(max(0, led4d));
    
    cli();
/*    Serial.print(F("Next display update in: "));
    Serial.println(displayLevelUpdateInterval);
    
    Serial.print(F("Current level: "));
    Serial.println(currentSmoothedLevel);*/
    sei();
  }
}

void showRoto(byte r, byte g, byte b) {
  strip.clear();
  int i;
  for(i=0; i < PIXEL_COUNT; i+=3) {
    strip.setPixelColor((rotoPos + i) % PIXEL_COUNT, r, g, b);
  }
  strip.show();
}

void showBlinkRed() {
  strip.clear();
  if (rotoPos % 2 == 0) {
    // set all pixels to red
    int i;
    for(i=0; i < PIXEL_COUNT; i++) {
      strip.setPixelColor(i, 128, 0, 0);
    }
    if (tones > 0) {
      tone(TONE_PIN, 660);
      tones--;
    }
  } else {
    noTone(TONE_PIN);
  }
  strip.show();
}

void showGreen() {
  // set all pixels to a dark green
  int i;
  for(i=0; i < PIXEL_COUNT; i++) {
    strip.setPixelColor(i, 0, 8, 0);
  }
  strip.show();
}

/**
 * Interrupt handler for ADC sample ready.
 */
ISR(ADC_vect) {
  unsigned long currentMillis = millis();

  // Read low to high per the atomic access control rules for the ADC.  Reading ADCL locks these
  // registers while reading ADCH releases these registers to the ADC again for writing.
  byte sampleLowByte = ADCL; 
  byte sampleHighByte = ADCH;
  int sample = (sampleHighByte << 8) | sampleLowByte;
  
  if (!ignoreConversion) {

    if (inputSource == INPUT_SOURCE_ADJUSTER) {
      // Now massage the sample into a format that we can shove into the FFT input and calculate
      // an arithmatic mean on when we have enough samples.

      fft_input[sampleCounter] = sample;

      sampleCounter += 1;

      if (sampleCounter == ADJUST_SAMPLE_COUNTER_THRESHOLD) {

        int sum = 0;

        for (int i = 0; i < ADJUST_SAMPLE_COUNTER_THRESHOLD; i++) {
          sum += fft_input[i];
        }

        int newAdjusterLevel = sum / ADJUST_SAMPLE_COUNTER_THRESHOLD;

        // Docs on abs say to not do maths inside of the brackets so assign this
        // to an intermediate value.  http://www.arduino.cc/en/Reference/Abs
        int delta = newAdjusterLevel - adjusterLevel;

        if (abs(delta) > 2)
        {
          adjusterLevel = newAdjusterLevel;

          int mappedAdjusterLevel = map(adjusterLevel, 0, 1024, 0, 512);

          infoThreshold = 2000 + mappedAdjusterLevel;
          infoReleaseThreshold = infoThreshold - (THRESHOLD_RELEASE_DELTA * 0.75);

          warnThreshold = infoThreshold + THRESHOLD_DELTA;
          warnReleaseThreshold = warnThreshold - THRESHOLD_RELEASE_DELTA;

          warnPlusThreshold = warnThreshold + THRESHOLD_DELTA;
          warnPlusReleaseThreshold = warnPlusThreshold - THRESHOLD_RELEASE_DELTA;

          emergThreshold = warnPlusThreshold + THRESHOLD_DELTA;
          emergReleaseThreshold = emergThreshold - THRESHOLD_RELEASE_DELTA;

          // Bring the smoothed average up to baseline quickly.
          if (smoothedLevel == 0.0) {
            smoothedLevel = infoThreshold - THRESHOLD_DELTA;
          }

          //Serial.print(F("Set baseline threshold: "));
          //Serial.println(infoThreshold);
        }

        sampleCounter = 0;
        setAdcInput(INPUT_SOURCE_LEVEL);
      }
    } else if (inputSource == INPUT_SOURCE_LEVEL) {    
      // Now massage the sample into the format that FFT wants it in.

      // Shift sample down by DC bias (1024 / 2 in ADC speak).
      sample -= 0x0200;
      // Slide the sample left the remaining 6 bits to fill the 16b int with our
      // 10b sample
      sample <<= 6;
    
      // This bin is the real part bin.  It gets the converted sample value.
      fft_input[sampleCounter] = sample;
    
      // This bin is the imaginary part bin.  It gets 0s, always.
      fft_input[sampleCounter + 1] = 0;
    
      // Don't forget to skip 2 at a time since we fill two indexes in the
      // array per sample.
      sampleCounter += 2;
    
      if (sampleCounter == FFT_SAMPLE_COUNTER_THRESHOLD) {
        // Do the FFT
        fft_window();
        fft_reorder();
        fft_run();
        fft_mag_log();
    
        // CPU_CLOCK / ADC_PRESCALER / CYCLES_PER_SAMPLE / FHT_N
        // 
        // Fs = 16MHz / 32 / 13 = ~38KHz
        // Df = Fs / 256 = 150Hz bandwidth per bin
        //
        // We have 128 bins each 150Hz wide for a total range of ~19KHz.
        //
    
        int magnitude = 0;
    
        for (int i = 1; i < FFT_OUTPUT_BINS; i++) {
          magnitude += fft_log_out[i];
        }
    
        // Uncomment to send binary FFT data out the serial line for use in Pd
        //      Serial.write(255);
        //      Serial.write(fft_log_out, 128);
    
        // Now we are going to adjust the running smoothed value.  We use different smoothing
        // factors for increases and decreases so that we are less sensitive to dropping volume levels
        // than we are to rising levels.
        //
        // Math is happening here...
        // http://en.wikipedia.org/wiki/Low-pass_filter#Simple_infinite_impulse_response_filter
        // http://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
        // http://en.wikipedia.org/wiki/Moving_average#Application_to_measuring_computer_performance
    
        if (smoothedLevel < magnitude) {
          smoothedLevel = smoothedLevel + alpha(currentMillis, previousSampleUpdateMillis, RISING_LEVEL_W)
              * (magnitude - smoothedLevel);
        } else {
          smoothedLevel = smoothedLevel + alpha(currentMillis, previousSampleUpdateMillis, FALLING_LEVEL_W)
              * (magnitude - smoothedLevel);
        }
    
        previousSampleUpdateMillis = currentMillis;
        sampleCounter = 0;
        
        fftCounter += 1;
        
        // 38KHz / 256 samples per FFT ~= 150 FFTs per second
        if (fftCounter == 10) {
          fftCounter = 0;  
          setAdcInput(INPUT_SOURCE_ADJUSTER);
        }
      }
    }
  } else {
    sampleCounter += 1;
    
    // We need to skip more than just the first sample as it seems that two samples
    // can be inaccurate after switching the mux.  Probably related to timing and/or
    // impedence.  We'll live with it.
    if (sampleCounter == INPUT_SWITCH_SAMPLES_TO_IGNORE) {
      ignoreConversion = false;
      sampleCounter = 0;
    }
  }
}

float alpha(unsigned long currentTimeMillis, unsigned long lastSampleTimeMillis, unsigned long readingWindow) {
  // Per http://en.wikipedia.org/wiki/Moving_average#Application_to_measuring_computer_performance
  return 1.00 - pow(2.71828182845904523536, -1.00
      * (((currentTimeMillis - lastSampleTimeMillis)) / (float) readingWindow));
}

/**
 * Initializes the ADC system.
 */
void initAdc() {
  // Turn off global interrupts.
  cli();

  // ADC Setup ///////////////////

  // Disable digital input buffer on ADC pins to reduce noise.
  // See section 24.9.5 of the AVR238P datasheet.
  sbi(DIDR0,ADC0D);
  sbi(DIDR0,ADC1D);
  sbi(DIDR0,ADC2D);
  sbi(DIDR0,ADC3D);
  sbi(DIDR0,ADC4D);
  sbi(DIDR0,ADC5D);



  // ADCSRA /////////////
  // 1 1 1 0 1 1 0 1
  //
  // 7 6 5 4 3 2 1 0
  // A A A A A A A A
  // D D D D D D D D
  // E S A I I P P P
  // N C T F E S S S
  //     E     2 1 0

  // Turn on the ADC.
  // See section 24.9.2 of the AVR238P datasheet.
  sbi(ADCSRA, ADEN);
  sbi(ADCSRA, ADSC);

  // Set the ADC to auto-trigger based on the ADTS bits of the ADCSRV SRF.  This enables
  // us to choose free-run mode rather than single conversion mode for sampling.
  // See sections 24.3 and 24.9.2 of the AVR238P datasheet.
  sbi(ADCSRA, ADATE);

  // Enable the ADC Conversion Complete Interrupt on conversion completion.  Note this is only
  // effective if the I bit of the SREG (global interrupt enable) is also enabled.
  // See sections 24.9.2 of the AVR238P datasheet.
  sbi(ADCSRA, ADIE);

  // Set the ADC prescaler select bits to increase the ADC speed.  A division factor
  // of 32 is chosen to take us to a ADC clock of 500KHz.  We lose a fraction of a bit of
  // accuracy but get a theoretical effective sampling rate of 38.5 KHz.  More than enough
  // to capture the frequency range of sounds that we are interested in.  There is a drop
  // in effective bits in the conversion, although not significant per the datasheet.
  // Emperical data from other sources indicate that the drop in effective bits is on the
  // order of magnitude of < 0.5 bits; however, it crosses the half bit mark between 9 and
  // 10 bits, thus effectively removing an entire effective bit.  We will just ignore this
  // and use all 10 bits as it does seem to cut down on the noise in the lower frequency bands.
  // See sections 24.9.2 of the AVR238P datasheet.
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  // ADCSRB /////////////
  // 0 0 0 0 0 0 0 0
  //
  // 7 6 5 4 3 2 1 0
  //           A A A
  //           D D D
  //           T T T
  //           S S S
  //           2 1 0

  // Set the ADC Auto Trigger Source to be free running mode.  The ADC will perform conversions
  // continuosly while ADSC of ADCSRA is high and ADC is enabled.
  // See section 24.9.4 of the AVR238P datasheet.
  cbi(ADCSRB, ADTS2);
  cbi(ADCSRB, ADTS1);
  cbi(ADCSRB, ADTS0);

  // ADMUX ////////////
  // See section 24.9.1 of the AVR238P datasheet.
  // 0 0 0 0 0 0 0 0
  // R R A   M M M M
  // E E D   U U U U
  // F F L   X X X X
  // S S A   3 2 1 0
  // 1 0 R

  // Set an external reference voltage.
  cbi(ADMUX, REFS1);
  cbi(ADMUX, REFS0);

  // Set the results of a conversion to be left-aligned, MSBs appear in ADCH.
  // See sections 24.2 and 24.9.1 of the AVR238P datasheet.
  cbi(ADMUX, ADLAR);

  // Set the input source for the ADC.
  setAdcInput(INPUT_SOURCE_ADJUSTER);

  // Turn on global interrupts.
  sei();
}

void setAdcInput(byte input) {
  ignoreConversion = true;
  inputSource = input;
  
  switch (input) {
    case INPUT_SOURCE_LEVEL:
      // Set ADC0 as the input channel in the ADC muxer.
      cbi(ADMUX, MUX3);
      cbi(ADMUX, MUX2);
      cbi(ADMUX, MUX1);
      cbi(ADMUX, MUX0);
      break;
    case INPUT_SOURCE_ADJUSTER:
      // Set ADC5 as the input channel in the ADC muxer.
      cbi(ADMUX, MUX3);
      sbi(ADMUX, MUX2);
      cbi(ADMUX, MUX1);
      sbi(ADMUX, MUX0);
      break;
  }
}

