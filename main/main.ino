#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
#include "sprites.h"

// Global Objects & Constants
TinyScreen display = TinyScreen(TinyScreenDefault);
const unsigned int* sprites[3] = {playerFaceSouthBMP, playerWalkSouthLeftBMP, playerWalkSouthRightBMP};

// Animation Screen Variables
int animationCounter = 0;
bool isAnimating = true;
bool firstSpriteShown = false;
const unsigned long DEBOUNCE_DELAY = 200; // in milliseconds
unsigned long lastButtonPressTime = 0;

void setup() {
  initializeDisplay();
  drawInitialScreen();
}

void loop() {
  if (isAnimating) {
    displayAnimationScreen();
  }
  
  handleButtonPresses();
}

void handleButtonPresses() {
  unsigned long currentTime = millis();
  if ((currentTime - lastButtonPressTime) > DEBOUNCE_DELAY) {
    if (isAnimating && (display.getButtons() & TSButtonLowerRight)) {
      transitionToMenuScreen();
      lastButtonPressTime = currentTime;
    } else if (!isAnimating && (display.getButtons() & TSButtonUpperLeft)) {
      transitionToAnimationScreen();
      lastButtonPressTime = currentTime;
    }
  }
}

void transitionToMenuScreen() {
  isAnimating = false;
  drawMenu();
}

void transitionToAnimationScreen() {
  isAnimating = true;
  firstSpriteShown = false;
  drawInitialScreen();
}





// First Screen
void initializeDisplay() {
  Wire.begin();
  display.begin();
  display.setBrightness(10);
  display.setFlip(true);
  display.setFont(liberationSans_8ptFontInfo);
}

void drawInitialScreen() {
  display.drawRect(0, 0, 96, 64, TSRectangleFilled, TS_8b_White);
  drawRightArrow(88, 54); // Bottom-right arrow
  drawTextBesideArrow("Go", 70, 53);
}

void displayAnimationScreen() {
  if (!firstSpriteShown) {
    drawBMPImage(40, 24, sprites[0], 16, 16);
    firstSpriteShown = true;
    delay(1000);
    return;
  }

  clearCharacterArea();
  drawBMPImage(40, 24, sprites[animationCounter], 16, 16);
  drawAnimationNavigation();

  animationCounter = (animationCounter == 1 ? 2 : 1);
  delay(1000);
}

void clearCharacterArea() {
  display.drawRect(39, 23, 18, 18, TSRectangleFilled, TS_8b_White);
}

void drawAnimationNavigation() {
  const int arrowBaseX = 88;
  const int arrowBaseY = 54;

  drawRightArrow(arrowBaseX, arrowBaseY);
  drawTextBesideArrow("Go", arrowBaseX - 18, 53);
}

void drawBMPImage(int x, int y, const unsigned int* image, int width, int height) {
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      unsigned int color = image[j * width + i];
      if (color != ALPHA) {
        display.drawPixel(x + i, y + j, color);
      }
    }
  }
}

void drawRightArrow(int baseX, int baseY) {
  display.drawLine(baseX, baseY, baseX + 6, baseY + 4, TS_8b_Green);
  display.drawLine(baseX, baseY + 8, baseX + 6, baseY + 4, TS_8b_Green);
  for (int i = 0; i < 6; i++) {
    display.drawLine(baseX + i, baseY + i, baseX + i, baseY + 8 - i, TS_8b_Green);
  }
}

void drawLeftArrow(int baseX, int baseY) {
    // Adjusted to make the arrow point to the left
    display.drawLine(baseX + 5, baseY, baseX, baseY + 4, TS_8b_Green);
    display.drawLine(baseX + 5, baseY + 8, baseX, baseY + 4, TS_8b_Green);
    for (int i = 0; i < 6; i++) {
        display.drawLine(baseX + 5 - i, baseY + i, baseX + 5 - i, baseY + 8 - i, TS_8b_Green);
    }
}

void drawTextBesideArrow(const char* text, int x, int y) {
  display.fontColor(TS_8b_Green, TS_8b_Black);
  display.setCursor(x, y);
  display.print(text);
}






// Second screen
void drawMenu() {
  display.clearScreen();

  // This can be expanded for more menu items later on
  drawLeftArrow(4, 4); // Top-left arrow pointing left
  drawTextBesideArrow("Log", 14, 3);

  drawRightArrow(88, 54); // Bottom-right arrow
  drawTextBesideArrow("Go", 70, 53);
}





// Third screen




