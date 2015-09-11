#include "Charliplexing.h"      //initializes the LoL Sheild library

#define LCD_PORT_TX 7
#define LCD_PORT_RX 6 // not really used 
#include <SoftwareSerial.h>
#include <inttypes.h>
class mySerial : public SoftwareSerial {

  public:
    mySerial(uint8_t receivePin, uint8_t transmitPin,
             bool inverse_logic = false) :
      SoftwareSerial(receivePin, transmitPin, inverse_logic) {}

    virtual size_t write(uint8_t byte) {
      return SoftwareSerial::write(byte);
    }

    int printf(char* fmt, ...) {
      char buff[256];
      va_list args;
      va_start(args, fmt);
      int return_status = vsnprintf(buff, sizeof(buff), fmt, args);
      va_end(args);
      uint8_t *s = (uint8_t *)&buff;
      while (*s) write(*s++);
      return return_status;
    }

};

byte brightness = 7; //Brightness goes from 0-7

mySerial lcdserial(LCD_PORT_RX, LCD_PORT_TX);

static const uint8_t PROGMEM
e8_bmp[] =
{ B10010010,
  B01001001,
  B00000000,
  B00111100,
  B01000110,
  B01011000,
  B01000011,
  B00111100
};

byte paddlePosition = 0;
byte opponentPaddlePosition = 0;
#define paddlePin 0

unsigned int   ballTime = 0, ballStartX = 8, ballStartY = 4;
byte
gameTicks = 0,
ballSpeed = 1,
ballDirection = 100,
ballX = 0,
ballY = 0;

#define BUFFERSIZE 120
char buffer[BUFFERSIZE + 1];
String parserString;

#include <stdarg.h>
void p(char *fmt, ... ) {
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  Serial.print(buf);
}

void setup() {
  Serial.begin(115200);

  lcdserial.begin(9600);
  lcdserial.write(254); // move cursor to beginning of first line
  lcdserial.write(128);

  lcdserial.print("                "); // clear display
  lcdserial.print("                ");

  lcdserial.write(254); // move cursor to beginning of first line
  lcdserial.write(128);

  LedSign::Init(SINGLE_BUFFER);  //Initializes the screen

  //intro();
  randomSeed(analogRead(2));
  ballDirection = random(88, 168);

  Serial.println(F("Ready"));
  Serial.println(F("Waiting for commands. Terminate with ';'."));
}

void loop() {
  // put your main code here, to run repeatedly:
  //tennis();
  processSerial();
  gameTicks = (gameTicks + 1) % 25;
}

int c;
int i = 0;
int len;
bool loopdelay;

void processSerial() {
  loopdelay = true;
  //if (Serial.available()) {
    int c = Serial.read();
    if (c != -1) {
      loopdelay = false;
      if (c == ';' || i + 2 > BUFFERSIZE) {
        buffer[i] = '\0';
        len = i;
        i = 0;
      } else {
        buffer[i++] = c;
      }
    }
    
    if (len > 0) {
      len = 0;
      
      parserString = String(buffer);
      if (parserString.startsWith("R")) {
        processMatrix(parserString.substring(1));
      }

    }
  //}
  //else {
  if (loopdelay) {
    Serial.println("Delay");
    delay(500);
  }
}

void processMatrix(String s) {
  unsigned long data;

  if (s.equalsIgnoreCase(F("CLEAR"))) {

    return;
  }

  /*  if (s.startsWith(F("ROT"))) {
      matrix.setRotation(s.substring(3,4).toInt());
      Serial.print(s.substring(3,4));
    }*/

  if (s.substring(0, 4).equalsIgnoreCase(F("DATA"))) {
    byte lastSep = 4, nextSep, line = 0;
    while (line < 9) {
      nextSep = s.indexOf(",", lastSep);
      if (nextSep == -1) nextSep = s.length();
      data = s.substring(lastSep, nextSep).toInt();

      for (byte led = 0; led < 14; ++led) {
        if (data & (1 << led)) {
          LedSign::Set(led, line, brightness);
        }
        else {
          LedSign::Set(led, line, 0);
        }
      }
      lastSep = nextSep + 1;
      line++;
    }
  }
}
//
//void intro() {
//  matrix.drawLine(0, 0, 1, 1, LED_GREEN);
//  matrix.drawLine(3, 0, 4, 1, LED_GREEN);
//  matrix.drawLine(6, 0, 7, 1, LED_GREEN);
//  matrix.writeDisplay();
//  delay(500);
//
//  matrix.drawLine(2, 7, 5, 7, LED_GREEN);
//  matrix.writeDisplay();
//  delay(250);
//
//  matrix.drawLine(2, 3, 5, 3, LED_GREEN);
//  matrix.writeDisplay();
//  delay(250);
//
//  matrix.drawLine(3, 5, 4, 5, LED_GREEN);
//  matrix.writeDisplay();
//  delay(250);
//
//  matrix.drawLine(1, 4, 1, 6, LED_GREEN);
//  matrix.writeDisplay();
//  delay(250);
//
//  matrix.drawLine(5, 4, 6, 4, LED_GREEN);
//  matrix.drawLine(6, 6, 7, 6, LED_GREEN);
//  matrix.writeDisplay();
//  delay(250);
//
//  for (byte b = 0; b < 9; b++) {
//    matrix.drawBitmap(0, 0, e8_bmp, 8, 8, 1 + b % 3);
//    matrix.writeDisplay();
//    delay(250);
//  }
//
//  matrix.clear();
//}
//
//
//void tennis() {
//  matrix.clear();
//  paddlePosition = map(analogRead(paddlePin), 80, 940, 0, 6);
//  moveBall();
//  moveOpponentPaddle();
//  checkPaddle();
//  checkBall();
//  //printDebug();
//
//  // paint ball
//  matrix.drawPixel(ballX - 1, ballY - 1, LED_GREEN);
//
//  // paint paddle
//  matrix.drawLine(0, paddlePosition, 0, paddlePosition + 1, LED_YELLOW);
//  matrix.drawLine(7, opponentPaddlePosition, 7, opponentPaddlePosition + 1, LED_RED);
//  matrix.writeDisplay();
//}
//
//void moveOpponentPaddle() {
//  if (gameTicks % 8 == 0) {
//    if (opponentPaddlePosition < ballY - 1) opponentPaddlePosition++;
//    if (opponentPaddlePosition > ballY - 1) opponentPaddlePosition--;
//    opponentPaddlePosition = constrain(opponentPaddlePosition, 0, 6);
//  }
//}
//
//void checkPaddle() {
//  if (ballX < 1) {
//    Serial.println("player lost");
//    // user has lost, ball is outside the game field
//    ballX = ballStartX = 7;
//    ballY = ballStartY = opponentPaddlePosition;
//    ballTime = 0;
//    ballDirection = random(88, 168);
//    ballSpeed = 1;
//  } else if (ballX < 2) { // 1 or 0: ball is at left border (or outside)
//    Serial.println("check player");
//    // check if paddle has ball contact
//    if (ballY - 1 == paddlePosition || ballY - 2 == paddlePosition) {
//      Serial.println("reflect player");
//      // reflect the ball
//      ballX = ballStartX = 2;
//      ballStartY = constrain(ballY, 1, 8);
//      ballTime = 0;
//      ballDirection = 128 - ballDirection + random(17) - 8;
//      ballSpeed = min(ballSpeed + 1, 4);
//    }
//  }
//
//  if (ballX > 8) {
//    Serial.println("arduino lost");
//    // arduino has lost, ball is outside the game field
//    ballX = ballStartX = 2;
//    ballY = ballStartY = paddlePosition;
//    ballTime = 0;
//    ballDirection = 172 + random(168);
//    ballSpeed = 1;
//  } else if (ballX > 7) { // 8 or 9: ball is at right border (or outside)
//    Serial.println("check arduino");
//    // check if paddle has ball contact
//    if (ballY - 1 == opponentPaddlePosition || ballY - 2 == opponentPaddlePosition)  {
//      Serial.println("reflect opponent");
//      // reflect the ball
//      ballX = ballStartX = 7;
//      ballStartY = constrain(ballY, 1, 8);
//      ballTime = 0;
//      ballDirection = 128 - ballDirection + random(17) - 8;
//    }
//  }
//}
//
//void printDebug() {
//  Serial.print(F("X: "));
//  Serial.print(ballX);
//  Serial.print(F(" Y: "));
//  Serial.print(ballY);
//  Serial.print(F(" D: "));
//  Serial.print(ballDirection);
//  Serial.println(F(""));
//}
//
//void moveBall() {
//  ballTime += 1;
//  ballX = ballStartX + (ballTime * ballSpeed / 3.0 * cos(ballDirection * 3.14 / 128.0)) / 10.0;
//  ballY = ballStartY + (ballTime * ballSpeed / 3.0 * sin(ballDirection * 3.14 / 128.0)) / 10.0;
//  delay(25);
//}
//
//void checkBall() {
//  // check boundaries
//  /*
//   * the ball can have a position between 1,1 (left top)
//   * and 8,8 (right, bottom)
//   *
//   *           90°/64
//   *              |
//   *              |
//   * 180°/128 ----+-----  0°/0
//   *              |
//   *              |
//   *          270°/192
//   *
//   */
//
//  // neuerWinkel = 360  - alterWinkel
//  if (ballY < 1) {
//    // upper border
//    ballStartX = constrain(ballX, 1, 8);
//    ballY = ballStartY = 1;
//    ballTime = 0;
//    ballDirection = 256 - ballDirection + random(7) - 3;
//  }
//  if (ballY > 8) {
//    // bottom border
//    ballStartX = constrain(ballX, 1, 8);
//    ballY = ballStartY = 8;
//    ballTime = 0;
//    ballDirection = 256 - ballDirection + random(7) - 3;
//  }
//
//  /*  if (ballX < 1) {
//      // left border (paddle side)
//      ballX = ballStartX = 1;
//      ballStartY = constrain(ballY, 1, 8);
//      ballTime = 0;
//      ballDirection = 128 - ballDirection + random(7) - 3;
//    }
//
//    if (ballX > 8) {
//      ballX = ballStartX = 8;
//      ballStartY = constrain(ballY, 1, 8);
//      ballTime = 0;
//      ballDirection = 128 - ballDirection + random(7) - 3;
//    }*/
//}

