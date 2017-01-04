#define FIRE_HEAT_WIDTH           66
#define FIRE_HEAT_HEIGHT          33

// Decrease bottom row by this value
#define FIRE_COOLING_RATE          6
// Increase new fire points in the bottom row by this value
#define FIRE_HEATING_RATE         25
// Increase if random number between 0 and 10 is larger than threshold
#define FIRE_HEATING_THRESHOLD     3
// Number of colors for the fire
#define FIRE_PALETTE_COLORS      100

// heat map
uint8_t fireHeat[FIRE_HEAT_WIDTH][FIRE_HEAT_HEIGHT];
// palette
static const uint16_t PROGMEM firePalette[FIRE_PALETTE_COLORS] = {
0x00, 0x00, 0x800, 0x800, 0x1000, 0x1000, 0x1800, 0x2000, 0x2000, 0x2800, 0x2800, 0x3000, 0x3800, 0x3800, 0x4000, 0x4000, 0x4800, 0x4800, 0x5000, 0x5800, 0x5800, 0x6000, 0x6000, 0x6800, 0x7000, 0x7000, 0x7800, 0x7800, 0x8000, 0x8800, 0x8800, 0x8800, 0x8820, 0x8840, 0x8860, 0x8860, 0x8880, 0x88A0, 0x88C0, 0x88C0, 0x88E0, 0x8900, 0x8920, 0x8940, 0x8940, 0x8960, 0x8980, 0x89A0, 0x89A0, 0x89C0, 0x89E0, 0x8A00, 0x8A20, 0x8A20, 0x8A40, 0x8A60, 0x8A80, 0x8A80, 0x8AA0, 0x8AC0, 0x8AE0, 0x8AE0, 0x8B00, 0x8B20, 0x8B40, 0x8B60, 0x8B60, 0x8B80, 0x8BA0, 0x8BC0, 0x8BC0, 0x8BE0, 0x8C00, 0x8C20, 0x8C40, 0x8C40, 0x8C40, 0x8C41, 0x8C42, 0x8C42, 0x8C43, 0x8C44, 0x8C44, 0x8C45, 0x8C46, 0x8C47, 0x8C47, 0x8C48, 0x8C49, 0x8C49, 0x8C4A, 0x8C4B, 0x8C4C, 0x8C4C, 0x8C4D, 0x8C4E, 0x8C4E, 0x8C4F, 0x8C50, 0x8C51
};


void updateFire() {
  // Counter variables
  uint8_t cx, cy;

  // Add heat to some points, decrease heat on the rest
  for (cx = 0; cx < FIRE_HEAT_WIDTH; cx++) {
    if (random(10) > 7) {
      fireHeat[cx][FIRE_HEAT_HEIGHT - 1] = min(FIRE_PALETTE_COLORS - 1, fireHeat[cx][FIRE_HEAT_HEIGHT - 1] + FIRE_HEATING_RATE);
    } else {
      fireHeat[cx][FIRE_HEAT_HEIGHT - 1] = max(0, fireHeat[cx][FIRE_HEAT_HEIGHT - 1] - FIRE_COOLING_RATE);
    }
  }

  /*
     X_new = (x + A + B + C)/4

              X
            / | \
           A  B  C
  */

  // Recalc rows above
  for (cy = FIRE_HEAT_HEIGHT - 2; cy > 0; cy--) {
    for (cx = 1; cx < FIRE_HEAT_WIDTH - 1; cx++) {
      fireHeat[cx][cy] =
        // old value
        (fireHeat[cx][cy] +
         // left
         fireHeat[cx - 1][cy + 1] +
         // underneath
         fireHeat[cx][cy + 1] +
         // right
         fireHeat[cx + 1][cy + 1]
        ) / 4;
    }
  }

  // Draw fire
  for (cy = 0; cy < PANELHEIGHT; cy++) {
    for (cx = 0; cx < PANELWIDTH; cx++) {
      matrix.drawPixel(cx, cy, firePalette[fireHeat[cx+1][cy]]);
    }
  }

  matrix.swapBuffers(false);
}

