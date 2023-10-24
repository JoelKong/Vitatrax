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
}


void drawMenu() {
  display.clearScreen();

  // Small arrow coordinates
  int baseX = 4; 
  int baseY = 4;

  // Draw the arrow
  display.drawLine(baseX, baseY + 4, baseX + 7, baseY, TS_8b_Green); // Diagonal upper line
  display.drawLine(baseX, baseY + 4, baseX + 7, baseY + 8, TS_8b_Green); // Diagonal lower line
  
  // Fill the inside of the arrow
  for (int i = 0; i < 7; i++) {
    int startY = baseY + 4 + i;
    int endY = baseY + 4 - i;
    display.drawLine(baseX + i, startY, baseX + i, endY, TS_8b_Green);
  }

  // Draw "Log" text right next to the arrow
  display.fontColor(TS_8b_Green, TS_8b_Black);
  display.setCursor(baseX + 10, 3);
  display.print("Log");
}

