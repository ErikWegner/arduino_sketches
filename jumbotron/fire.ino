#define FIRE_HEAT_WIDTH           32
#define FIRE_HEAT_HEIGHT          32

// Decrease bottom row by this value
#define FIRE_COOLING_RATE          5
// Increase new fire points in the bottom row by this value
#define FIRE_HEATING_RATE         20
// Increase if random number between 0 and 10 is larger than threshold
#define FIRE_HEATING_THRESHOLD     7
// Number of colors for the fire
#define FIRE_PALETTE_COLORS      100

// heat map
byte fireHeat[FIRE_HEAT_WIDTH][FIRE_HEAT_HEIGHT];

void updateFire() {
  // Counter variables
  byte cx, cy;
  // tempoarily save heat value
  byte heat;

  // Add heat to some points, decrease heat on the rest
  for (cx = 0; cx < FIRE_HEAT_WIDTH; cx++) {
    if (random(10) > 7) {
      fireHeat[cx][FIRE_HEAT_HEIGHT - 1] = min(FIRE_PALETTE_COLORS - 1, fireHeat[cx][FIRE_HEAT_HEIGHT - 1] + FIRE_HEATING_RATE);
    } else {
      fireHeat[cx][FIRE_HEAT_HEIGHT - 1] = max(0, fireHeat[cx][FIRE_HEAT_HEIGHT - 1] - FIRE_COOLING_RATE);
    }
  }

  /*
   * X_new = (x + A + B + C)/4
   * 
   *          X
   *        / | \
   *       A  B  C
   */

  // Recalc rows above
  for (cy = FIRE_HEIGHT_HEIGHT - 2; cy >= 0; cy--) {
    for (cx = 1; cx < FIRE_HEAT_WIDTH - 2; cx++) {
      fireHeat[cx][cy] = 
        // old value
        (fireHeat[cx][cy] +
        // left
        fireHeat[cx - 1][cy + 1] +
        // underneath
        fireHeat[cx][cy + 1] +
        // right
        fireHeat[cx + 1][cy + 1]
        )/4;
    }
  }

  // Draw fire
  for (cy = 0; cy < PANELHEIGHT; cy++) {
    for (cy = 0; cx < PANELWIDTH; cx++) {
      
    }
  }
}

