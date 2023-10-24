#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>

TinyScreen display = TinyScreen(TinyScreenDefault);

void setup(void) {
  Wire.begin();
  display.begin();
  display.setBrightness(10);
  display.setFlip(true);
  display.setFont(liberationSans_8ptFontInfo);
  drawMenu();
}

void loop() {
  handleButtons();
}

void handleButtons() {
  if (display.getButtons() & TSButtonUpperLeft) {
    // This is where you handle the action when the top-leftmost button is pressed.
    // For now, let's just clear the screen:
    display.clearScreen();
    delay(500); // Simple debounce, so we don't repeatedly trigger the action.
  }

  if (display.getButtons() & TSButtonLowerRight) {
    // Action for the bottom-rightmost button
    // For demonstration purposes, let's just invert the screen colors:
    display.clearScreen();
    display.fontColor(TS_8b_Green, TS_8b_Black);
    display.setCursor(10, 30); 
    display.print("Button Pressed");
    delay(500); // Simple debounce
  }
}


void drawMenu() {
  display.clearScreen();

  // Top-left arrow coordinates
  int topLeftBaseX = 4; 
  int topLeftBaseY = 4;

  // Draw the top-left arrow
  display.drawLine(topLeftBaseX, topLeftBaseY + 4, topLeftBaseX + 5, topLeftBaseY, TS_8b_Green);
  display.drawLine(topLeftBaseX, topLeftBaseY + 4, topLeftBaseX + 5, topLeftBaseY + 8, TS_8b_Green);
  
  // Fill the inside of the top-left arrow
  for (int i = 0; i < 6; i++) {
    int startY = topLeftBaseY + 4 + i;
    int endY = topLeftBaseY + 4 - i;
    display.drawLine(topLeftBaseX + i, startY, topLeftBaseX + i, endY, TS_8b_Green);
  }

  // Draw "Log" text right next to the top-left arrow
  display.fontColor(TS_8b_Green, TS_8b_Black);
  display.setCursor(topLeftBaseX + 10, 3);
  display.print("Log");

  // Bottom-right arrow coordinates
  int bottomRightBaseX = 88; // Adjusted for bottom-right position
  int bottomRightBaseY = 54; 

  // Draw the bottom-right arrow
  display.drawLine(bottomRightBaseX, bottomRightBaseY, bottomRightBaseX + 6, bottomRightBaseY + 4, TS_8b_Green);
  display.drawLine(bottomRightBaseX, bottomRightBaseY + 8, bottomRightBaseX + 6, bottomRightBaseY + 4, TS_8b_Green);
  
  // Fill the inside of the bottom-right arrow
  for (int i = 0; i < 6; i++) {
    int startY = bottomRightBaseY;
    int endY = bottomRightBaseY + 8;
    display.drawLine(bottomRightBaseX + i, startY + i, bottomRightBaseX + i, endY - i, TS_8b_Green);
  }

  // Draw "Go" text to the left of the bottom-right arrow
  display.fontColor(TS_8b_Green, TS_8b_Black);
  display.setCursor(bottomRightBaseX - 18, 53); // Adjusted to position "Go" to the left of the arroww
  display.print("Go");
}







