#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

#define BREITE 8
#define HOEHE  8

unsigned char lines[HOEHE];
unsigned char color = 0;

void setup() {
  matrix.begin(0x70);
  matrix.clear();
  matrix.setRotation(3);
  randomLines();
}

void loop() {
  // put your main code here, to run repeatedly:
  updateDisplay();
  if (updateLines() == 0) {
    color = (color + 1) % 3;
    randomLines();
    delay(1000);
  }
  delay(333);
}


unsigned char countNeighbours(unsigned char line, unsigned char column) {
  unsigned char neighbours = 0;
  /*
      1 2 3
    4   5
    6 7 8
  */
  // 1
  if (line > 0 && column > 0) {
    neighbours += bitRead(lines[line - 1], column - 1);
  }
  // 2
  if (line > 0) {
    neighbours += bitRead(lines[line - 1], column);
  }
  // 3
  if (line > 0 && column < BREITE - 1) {
    neighbours += bitRead(lines[line - 1], column + 1);
  }
  // 4
  if (column > 0) {
    neighbours += bitRead(lines[line], column - 1);
  }
  // 5
  if (column < BREITE - 1) {
    neighbours += bitRead(lines[line], column + 1);
  }
  // 6
  if (line < HOEHE - 1 && column > 0) {
    neighbours += bitRead(lines[line + 1], column - 1);
  }
  // 7
  if (line < HOEHE - 1) {
    neighbours += bitRead(lines[line + 1], column);
  }
  // 8
  if (line < HOEHE - 1 && column < BREITE - 1) {
    neighbours += bitRead(lines[line + 1], column + 1);
  }
  
  return neighbours;
}

/* return 1 if an update was made */
unsigned char updateLines() {
  unsigned char line, column, neighbours;
  unsigned char newlines[HOEHE];
  unsigned char deadOrAlive;
  unsigned char hasUpdate = 0;
  for (line = 0; line < HOEHE; line++) {
    newlines[line] = 0;
    for (column = 0; column < BREITE; column++) {
      neighbours = countNeighbours(line, column);
      deadOrAlive = bitRead(lines[line], column);
      bitWrite(newlines[line], column, 
        // dead cell with 3 neighbours gets born
        (deadOrAlive == 0 && neighbours == 3) ||
        // living cell with 2 or 3 neighbours lives on
        (deadOrAlive == 1 && neighbours > 1 && neighbours < 4));
    }
  }
  for (line = 0; line < HOEHE; line++) {
    if (lines[line] != newlines[line]) {
      lines[line] = newlines[line];
      hasUpdate = 1;
    }
  }
  
  return hasUpdate;
}

char checkLines() {
  unsigned char line;
  for (line = 0; line < HOEHE; line++) {
    if (lines[line] > 0) return 1;
  }

  return 0;
}

void randomLines() {
  randomSeed(analogRead(0));

  unsigned char line;
  for (line = 0; line < HOEHE; line++) {
    lines[line] = random(256);
  }
}


void updateDisplay() {
  unsigned char line;
  unsigned int red = (color > 0) ? 256 : 0;
  unsigned int green = (color != 1) ? 1: 0;
  for (line = 0; line < HOEHE; line++) {
    matrix.displaybuffer[line] = lines[line] * green + lines[line] * red;
  }
  matrix.writeDisplay();
}


