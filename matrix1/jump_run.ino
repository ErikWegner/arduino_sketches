/* Graphics */

/* Constants */

#define JR_HUMP_COUNT 3
#define JR_HUMP_WIDTH 5
#define JR_JUMP_PIXELS 5

// each pixel is a y-offset
uint8_t jr_hump_pixels[JR_JUMP_PIXELS * 2] PROGMEM = { 1,  0,  0,  1,  1};
// line of each hump
uint8_t jr_hump_rows[JR_JUMP_PIXELS] = { 23, 25, 30};

/* Variables */

volatile uint8_t jr_run = 0;

// horizontal position for humps
int8_t jr_humps_x[JR_HUMP_COUNT] = { -JR_HUMP_WIDTH, -JR_HUMP_WIDTH, -JR_HUMP_WIDTH};

// decremented on each tick, move humps when reached zero
uint8_t jr_hump_move_delay = 0;

// when jr_hump_move_delay is zero, restart with this value
uint8_t jr_hump_move_delay_init = 1;


/* Implementation */

void jump_and_run_tick_enabled() {
  jr_run = 1;
}

inline void jump_and_run_tick() {
  if (jr_run != 1) {
    return;
  }
  jr_run = 0;
#if DEBUG == 1
  Serial.println("jump_and_run_tick");
#endif
  jr_updateScene();
  jr_drawScene();
}

void jr_drawScene() {
  uint8_t c1, c2;
  int8_t hump_x, hump_y;
  backgroundLayer.fillScreen({0, 0, 0});
  // draw base line
  backgroundLayer.drawLine(0, 28, MATRIX_WIDTH - 1, 28, {255, 255, 255});

  // draw humps
  for (c1 = 0; c1 < JR_HUMP_COUNT; c1++) {
    hump_x = jr_humps_x[c1];
    hump_y = jr_hump_rows[c1];
    for (c2 = 0; c2 < JR_JUMP_PIXELS; c2++) {
      backgroundLayer.drawPixel(hump_x + c2, hump_y + jr_hump_pixels[c2], {139, 255, 155});
    }
  }

  backgroundLayer.swapBuffers();
}

void jr_updateScene() {
  jr_move_humps();
}

/**
   Decrement jr_hump_move_delay. If its value has
   reached zero, move all humps one pixel left. If
   hump is out of screen, set new random position
   outside matrix.
*/
void jr_move_humps() {
  if (jr_hump_move_delay > 0) {
    jr_hump_move_delay--;
    return;
  }

  jr_hump_move_delay = jr_hump_move_delay_init;
  for (uint8_t humpcounter = 0; humpcounter < JR_HUMP_COUNT; humpcounter++) {
    jr_humps_x[humpcounter] = jr_humps_x[humpcounter] - 1;
    if (jr_humps_x[humpcounter] < -JR_HUMP_WIDTH) {
      // hump has moved left out of visible area
      jr_humps_x[humpcounter] = MATRIX_WIDTH + JR_HUMP_WIDTH + random(20);
    }
  }
}

