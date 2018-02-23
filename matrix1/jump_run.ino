/* Constants */

#define JR_HUMP_COUNT 3
#define JR_HUMP_WIDTH 5
#define JR_HUMP_SPEEDSCALE 3
#define JR_HUMP_PIXELS 5
#define JR_AVATAR_RUNNING_FRAMES 2
#define JR_AVATAR_RUNNING_WIDTH 7
#define JR_AVATAR_RUNNING_HEIGHT 18

/* Graphics */

rgb24 jr_palette[] PROGMEM = {
  // 0: transparent
  {0x00, 0x00, 0x00},
  // 1: blue
  {0x09, 0x00, 0xff},
  // 2: skin
  {0xff, 0xb7, 0x83},
  // 3: yellow
  {0xff, 0xfd, 0x00},
  // 4: black
  {0x40, 0x40, 0x40}
};

uint8_t avatar_running[JR_AVATAR_RUNNING_FRAMES][JR_AVATAR_RUNNING_WIDTH * JR_AVATAR_RUNNING_HEIGHT] PROGMEM = {
  // 1 = #0900ff
  // 2 = #ffb783
  // 3 = yellow
  // 4 = black
  {
    0, 0, 0, 3, 3, 0, 3,
    0, 0, 3, 3, 3, 3, 0,
    0, 0, 3, 3, 3, 2, 0,
    0, 0, 3, 0, 2, 2, 0,
    0, 0, 0, 0, 2, 2, 0,
    0, 0, 0, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1,
    1, 0, 1, 1, 1, 1, 2,
    2, 0, 0, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 0, 1, 0,
    0, 1, 1, 1, 0, 1, 0,
    0, 1, 1, 0, 1, 0, 0,
    0, 4, 1, 0, 1, 4, 0,
    0, 4, 4, 0, 4, 0, 0
  }, {
    0, 0, 3, 3, 0, 3, 0,
    0, 3, 3, 3, 3, 0, 0,
    0, 3, 3, 3, 2, 0, 0,
    0, 3, 0, 2, 2, 0, 0,
    0, 0, 0, 2, 2, 0, 0,
    0, 0, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0,
    0, 1, 2, 1, 1, 1, 0,
    0, 0, 1, 1, 0, 0, 2,
    0, 0, 1, 1, 1, 0, 0,
    0, 0, 1, 1, 1, 1, 0,
    0, 1, 0, 1, 1, 0, 0,
    0, 1, 0, 1, 1, 1, 0,
    0, 0, 1, 0, 1, 1, 0,
    0, 4, 1, 0, 4, 1, 0,
    0, 0, 4, 0, 4, 4, 0
  }
};

/* Variables */

// each pixel is a y-offset
uint8_t jr_hump_pixels[JR_HUMP_PIXELS * 2] PROGMEM = { 1,  0,  0,  1,  1};
// line of each hump
uint8_t jr_hump_rows[JR_HUMP_PIXELS] = { 20, 24, 30};

volatile uint8_t jr_run = 0;

// horizontal position for humps (multiplied with 4)
int16_t jr_humps_x[JR_HUMP_COUNT] = { 12, 36, 41};

// decremented on each tick, move humps when reached zero
uint8_t jr_hump_move_delay = 0;
uint8_t jr_avatar_anim_delay = 0;

// when jr_hump_move_delay is zero, restart with this value
uint8_t jr_hump_move_delay_init = 1;
uint8_t jr_avatar_anim_delay_init = 5;

uint8_t jr_avatar_frame = 0;


/* Implementation */

void jump_and_run_tick_enabled() {
  jr_run = 1;
}

inline void jump_and_run_tick() {
  /*if (jr_run != 1) {
    return;
    }
    jr_run = 0;*/
#if DEBUG == 1
  Serial.println("jump_and_run_tick");
#endif
  jr_updateScene();
  jr_drawScene();
}

void jr_drawScene() {
  uint8_t c1, c2;
  int8_t hump_x, hump_y;
  uint8_t paletteindex;
  backgroundLayer.fillScreen({0, 0, 0});
  // draw base line
  backgroundLayer.drawLine(0, 28, MATRIX_WIDTH - 1, 28, {255, 255, 255});

  // draw humps
  for (c1 = 0; c1 < JR_HUMP_COUNT; c1++) {
    hump_x = jr_humps_x[c1] / JR_HUMP_SPEEDSCALE;
    hump_y = jr_hump_rows[c1];
    for (c2 = 0; c2 < JR_HUMP_PIXELS; c2++) {
      backgroundLayer.drawPixel(hump_x + c2, hump_y + jr_hump_pixels[c2], {139, 255, 155});
    }
  }

  // draw avatar
  for (c1 = 0; c1 < JR_AVATAR_RUNNING_WIDTH; c1++) {
    for (c2 = 0; c2 < JR_AVATAR_RUNNING_HEIGHT; c2++) {
      paletteindex = avatar_running[jr_avatar_frame][c1 + c2 * JR_AVATAR_RUNNING_WIDTH];
      if (paletteindex > 0) {
        backgroundLayer.drawPixel(5 + c1, 10 + c2, jr_palette[paletteindex]);
      }
    }
  }

  backgroundLayer.swapBuffers();
}

void jr_updateScene() {
  jr_update_avatar();
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
    jr_humps_x[humpcounter] = jr_humps_x[humpcounter] - humpcounter - 1;
    if (jr_humps_x[humpcounter] < -JR_HUMP_WIDTH * JR_HUMP_SPEEDSCALE) {
      // hump has moved left out of visible area
      jr_humps_x[humpcounter] = (MATRIX_WIDTH + JR_HUMP_WIDTH + random(20)) * JR_HUMP_SPEEDSCALE;
    }
  }
}

void jr_update_avatar() {
  if (jr_avatar_anim_delay > 0) {
    jr_avatar_anim_delay--;
    return;
  }

  jr_avatar_anim_delay = jr_avatar_anim_delay_init;
  
  jr_avatar_frame = jr_avatar_frame + 1;
  if (jr_avatar_frame >= JR_AVATAR_RUNNING_FRAMES) {
    jr_avatar_frame = 0;
  }
}

