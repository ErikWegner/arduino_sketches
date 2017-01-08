#define starsCount 60
#define starsSize 150

float 
  star_x[starsCount],
  star_y[starsCount],
  star_z[starsCount];

uint8_t starsOffscreen(int8_t star_screenx, uint8_t star_screeny) {
  if (star_screenx < 0) return 1;
  if (star_screenx >= PANELWIDTH) return 1;
  if (star_screeny < 0) return 1;
  if (star_screeny >= PANELHEIGHT) return 1;
  return 0;
}

void resetStar(uint8_t i) {
  star_x[i]  = (rand() % (starsSize * 2)) - starsSize;
  star_y[i]  = (rand() % (starsSize * 2)) - starsSize;
  star_z[i]  = 1000;
}

void setupStarfield() {
  for (uint8_t i = 0; i < starsCount; i++) {
    resetStar(i);
  }
}

void starfield() {
  int8_t star_screenx, star_screeny;
  
  for (uint8_t i = 0; i < starsCount; i++) {
    star_z[i] = star_z[i] - 12;
    star_screenx = 100 * star_x[i] / star_z[i] + PANELWIDTH / 2;
    star_screeny = 100 * star_y[i] / star_z[i] + PANELHEIGHT / 2;

    if (starsOffscreen(star_screenx, star_screeny) || star_z[i] < 0) {
      resetStar(i);
    } else {    
      matrix.drawPixel(star_screenx, star_screeny, 0xffff);
    }
  }
  
  matrix.swapBuffers(true);
}
