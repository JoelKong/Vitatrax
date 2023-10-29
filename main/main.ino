// Add libraries
#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
#include <STBLE.h>
#include "BMA250.h"
#include "sprites.h"

// Initialise bluetooth
// #ifndef BLE_DEBUG
// #define BLE_DEBUG true
// #endif

// #if defined (ARDUINO_ARCH_AVR)
// #define SerialMonitorInterface Serial
// #elif defined(ARDUINO_ARCH_SAMD)
// #define SerialMonitorInterface SerialUSB
// #endif

uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0


// Initialise accelerometer
BMA250 accel_sensor;
double temp;

// Global Objects & Constants
TinyScreen display = TinyScreen(TinyScreenDefault);
const unsigned int* sprites[3] = {playerFaceSouthBMP, playerWalkSouthLeftBMP, playerWalkSouthRightBMP};

// Screen Variables
int animationCounter = 0;
bool firstSpriteShown = false;
const unsigned long DEBOUNCE_DELAY = 200; // in milliseconds
unsigned long lastButtonPressTime = 0;

// Stopwatch variables
bool stopwatchRunning = false;
unsigned long stopwatchStartTime = 0;
unsigned long elapsedMillis = 0;

// Step count variables
int stepGoal = 500;
bool validStep = false;
int newSample = 0;
int oldSample = 0;
int totalSteps = 0;
int stepIntervalLow = 200;
unsigned long stepIntervalHigh = 2000;
unsigned long lastStepTime = 0;
const int amtSamples = 32;
int aBuff[amtSamples];
int aBuffPos = 0;

// Mood Indicator Display
bool faceDisplayed = false;
String faceType = "h";


//Alarm
String alarmValueStr = "10:00"; 

// Eco
int weight = 70;

// Screens
enum ScreenType {
  ANIMATION_SCREEN,
  MENU_SCREEN,
  TRACKER_SCREEN,
  ECO_SCREEN
};

ScreenType currentScreen = ANIMATION_SCREEN;

// Setup and Loop
void setup() {
  initializeDisplay();
  initializeAccel();
  drawInitialScreen();

  BLEsetup();
}

void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  readSteps();

  switch (currentScreen) {
    case ANIMATION_SCREEN:
      displayAnimationScreen();
      break;
    case MENU_SCREEN:
      // Displays menu
      displayMenu();
      break;
    case TRACKER_SCREEN:
      displayStopwatch(); // Update and display the stopwatch
      break;
    case ECO_SCREEN:
      displayEco();
      break;
  }
  
  handleButtonPresses();
}

// Accelerometer setup
void initializeAccel() {
  Serial.begin(115200);
  Wire.begin();
  accel_sensor.begin(BMA250_range_2g, BMA250_update_time_64ms); 
}

// Buttons
void handleButtonPresses() {
  unsigned long currentTime = millis();
  if ((currentTime - lastButtonPressTime) > DEBOUNCE_DELAY) {
    switch (currentScreen) {
      case ANIMATION_SCREEN:
        if (display.getButtons() & TSButtonLowerRight) {
          transitionToMenuScreen();
          lastButtonPressTime = currentTime;
        }
        break;
      case MENU_SCREEN:
        if (display.getButtons() & TSButtonLowerRight) {
          transitionToTrackerScreen();
          lastButtonPressTime = currentTime;
        } else if (display.getButtons() & TSButtonUpperLeft) {
          transitionToAnimationScreen();
          lastButtonPressTime = currentTime;
        } else if (display.getButtons() & TSButtonLowerLeft){
          transitionToEcoScreen();
        }
        break;
      case TRACKER_SCREEN:
        if (display.getButtons() & TSButtonUpperLeft) {
          transitionToMenuScreen();
          lastButtonPressTime = currentTime;
        } else if (display.getButtons() & TSButtonLowerRight) {
          toggleStopwatch();  // Toggle the stopwatch when the bottom right button is pressed
          lastButtonPressTime = currentTime;
        } else if (display.getButtons() & TSButtonLowerLeft) {
            resetStopwatch();  // Reset the stopwatch when the bottom left button is pressed
            lastButtonPressTime = currentTime;
        } else if (display.getButtons() & TSButtonUpperRight) {
          // Add bluetooth track here
        }
        break;
      case ECO_SCREEN:
        if (display.getButtons() & TSButtonUpperLeft) {
          transitionToMenuScreen();
          lastButtonPressTime = currentTime;
        }
    }
  }
}

void transitionToMenuScreen() {
  resetStopwatch();
  currentScreen = MENU_SCREEN;
  faceDisplayed = false;
  drawMenu();
}

void transitionToAnimationScreen() {
  currentScreen = ANIMATION_SCREEN;
  firstSpriteShown = false;
  drawInitialScreen();
}

void transitionToTrackerScreen() {
  currentScreen = TRACKER_SCREEN;
  drawTracker();
}

void transitionToEcoScreen() {
  currentScreen = ECO_SCREEN;
  drawEco();
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
  drawTextBesideArrow("Go", 70, 53, TS_8b_White);
  drawTextBesideArrow("Vitatrax", 26, 8, TS_8b_White);
}

void displayAnimationScreen() {
  if (!firstSpriteShown) {
    drawBMPImage(40, 24, sprites[0], 16, 16);
    firstSpriteShown = true;
    delay(600);
    return;
  }

  clearCharacterArea();
  drawBMPImage(40, 24, sprites[animationCounter], 16, 16);
  drawAnimationNavigation();

  animationCounter = (animationCounter == 1 ? 2 : 1);
  delay(600);
}

void clearCharacterArea() {
  display.drawRect(39, 23, 18, 18, TSRectangleFilled, TS_8b_White);
}

void drawAnimationNavigation() {
  const int arrowBaseX = 88;
  const int arrowBaseY = 54;

  drawRightArrow(arrowBaseX, arrowBaseY);
  drawTextBesideArrow("Go", arrowBaseX - 18, 53, TS_8b_White);
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

void drawTextBesideArrow(const char* text, int x, int y, uint8_t bg) {
  display.fontColor(TS_8b_Green, bg);
  display.setCursor(x, y);
  display.print(text);
}






// Second screen
void drawMenu() {
  if (currentScreen == MENU_SCREEN) {
    display.clearScreen();

    // This can be expanded for more menu items later on
    drawLeftArrow(4, 4); // Top-left arrow pointing left
    drawTextBesideArrow("Log", 14, 3, TS_8b_Black);

    drawLeftArrow(4, 53); // Bottom-left arrow
    drawTextBesideArrow("Eco", 14, 52, TS_8b_Black);

    drawRightArrow(88, 54); // Bottom-right arrow
    drawTextBesideArrow("Go", 70, 53, TS_8b_Black);
  }
}

// Steps functions
int readSteps() {
  accel_sensor.read();

  int sum = pow(accel_sensor.X, 2);
  sum += pow(accel_sensor.Y, 2);
  sum += pow(accel_sensor.Z, 2);
  int magnitude = sqrt(sum);

  int difference = abs(magnitude - oldSample);

  if (difference < 10) {
    difference = 0;
  }
  if (difference > 30) {
    difference = 30;
  }

  if (magnitude - oldSample > 0) {
    difference = oldSample + difference;
  } else {
    difference = oldSample - difference;
  }

  aBuff[aBuffPos] = difference;

  int Amin = 1000;
  int Amax = 0;
  for (int i = 0; i < amtSamples; i++) {
    int sampleVal = aBuff[i];
    if (sampleVal < Amin)Amin = sampleVal;
    if (sampleVal > Amax)Amax = sampleVal;
  }

  int peakToPeak = Amax - Amin;
  int threshold = (peakToPeak / 2) + Amin;

  oldSample = newSample;

  bool newStep = false;
  if (abs(newSample - difference) > 10) {
    newSample = difference;
    if (peakToPeak > 70 && oldSample > threshold && newSample < threshold) {
      if (validStep) {
        if (millis() > lastStepTime + stepIntervalLow && millis() < lastStepTime + stepIntervalHigh) {
          newStep = true;
        } else {
          validStep = false;
        }
      } else if (millis() > lastStepTime + stepIntervalLow && millis() < lastStepTime + stepIntervalHigh) {
        newStep = true;
        validStep = true;
      }
      lastStepTime = millis();
      if (newStep) {
        totalSteps++;
        char data[] = "1";
        lib_aci_send_data(0, (uint8_t*)data, strlen(data));
      }
    }
  }
  aBuffPos++;
  if (aBuffPos >= amtSamples) {
    aBuffPos = 0;
  }
  if (newStep)
    return 1;
  return 0;
}

// Mood Indicator functions
// Draw circle for the face
void drawFilledCircle(int x0, int y0, int radius, uint16_t color) {
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      if (x * x + y * y <= radius * radius) {
        display.drawPixel(x0 + x, y0 + y, color);
      }
    }
  }
}

// Smiley/Neutral/Sad Face
void drawFace(int x, int y, uint16_t color) {
  // Face
  drawFilledCircle(x, y, 10, color);

  // Eyes
  drawFilledCircle(x - 5, y - 3, 1, 0xFFFF); // Left eye
  drawFilledCircle(x + 5, y - 3, 1, 0xFFFF); // Right eye

  // Smiley Mouth (Default)
  if (faceType == "h") {
    display.drawPixel(x - 3, y + 3, 0xFFFF);
    display.drawPixel(x - 2, y + 4, 0xFFFF);
    display.drawPixel(x - 1, y + 4, 0xFFFF);
    display.drawPixel(x, y + 4, 0xFFFF);
    display.drawPixel(x + 1, y + 4, 0xFFFF);
    display.drawPixel(x + 2, y + 4, 0xFFFF);
    display.drawPixel(x + 3, y + 3, 0xFFFF);
  } else if (faceType == "n") {
    // Neutral Mouth
    display.drawLine(x - 2, y + 2, x + 2, y + 2, 0xFFFF);
  } else if (faceType == "s") {
    // Sad Mouth 
    display.drawLine(x - 2, y + 4, x, y + 2, 0xFFFF);
    display.drawLine(x, y + 2, x + 2, y + 4, 0xFFFF);
  }

}


// Displays main menu
void displayMenu() {
  display.setCursor(5, 15);
  display.print("Alarm: ");
  display.print(alarmValueStr);

  display.setCursor(5, 25);
  display.print("Goal: ");
  display.print(stepGoal);

  display.setCursor(5, 35);
  display.print("Prog: ");
  display.print(totalSteps);

  // Draw the Face (Default is Smiley)
  if (!faceDisplayed) {
    drawFace(80, 35, TS_8b_Green); 
    faceDisplayed = true;
  }
}

// Alarm
// void updateAlarmValue(String alarmValueStr) {
  // Replace the following with the correct coordinates and text elements
  // int alarmX = 5;
  // int alarmY = 15;

  // Clear the section where "Alarm: " is displayed
  // clearSection(alarmX, alarmY, 70, 10, 0x0000);

  // Extract hours and minutes
  // int hours = alarmValueStr.substring(0, 2).toInt();
  // int minutes = alarmValueStr.substring(2, 4).toInt();

  // Format and display the new alarm value as "Alarm: HH:MM"
  // display.setCursor(alarmX, alarmY);
  // display.print("Alarm: ");
  // if (hours < 10) {
  //   display.print("0");
  // }
  // display.print(hours);
  // display.print(":");
  // if (minutes < 10) {
  //   display.print("0");
  // }
  // display.print(minutes);
// }

void refreshMenuScreen(String alarmValueStr, int stepGoal, String faceType) {
  display.clearScreen();
  // To redraw face
  faceDisplayed = false;
  displayMenu();
}

void clearSection(int x, int y, int width, int height, uint16_t backgroundColor) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      display.drawPixel(x + j, y + i, backgroundColor);
    }
  }
}

void displayPopup(String message) {
  display.clearScreen();
  display.setFont(liberationSans_8ptFontInfo);
  display.setCursor(20, 20);
  display.print(message);
  delay(2000);  // Display the message for 2 seconds (adjust as needed)
}





// Third screen
void drawTracker() {
  if (currentScreen == TRACKER_SCREEN) {  // Guard the drawing with a condition to prevent unnecessary redrawing
    display.clearScreen();
    drawLeftArrow(4, 4); // Top-left arrow pointing left
    drawTextBesideArrow("Back", 14, 3, TS_8b_Black);

    drawLeftArrow(4, 53); // Top-left arrow pointing left
    drawTextBesideArrow("Reset", 14, 52, TS_8b_Black);

    drawRightArrow(88, 4); // Adjusted to move the arrow a bit to the right
    drawTextBesideArrow("Track", 56, 3, TS_8b_Black);

    // drawRightArrow(88, 53); // Adjusted to move the arrow a bit to the right
    // drawTextBesideArrow("Start", 58, 52, TS_8b_Black);
    // drawTextBesideArrow("Stop", 60, 52, TS_8b_Black);
    if (stopwatchRunning) {
      drawRightArrow(88, 53); // Adjusted to move the arrow a bit to the right
      drawTextBesideArrow("Stop", 60, 52, TS_8b_Black);
    } else {
      drawRightArrow(88, 53); // Adjusted to move the arrow a bit to the right
      drawTextBesideArrow("Start", 58, 52, TS_8b_Black);
    }
  }
}

// Stopwatch Functions
void startStopwatch() {
  stopwatchStartTime = millis();
  stopwatchRunning = true;
}

void stopStopwatch() {
  elapsedMillis += millis() - stopwatchStartTime;
  stopwatchRunning = false;
}

void resetStopwatch() {
  stopwatchRunning = false;
  elapsedMillis = 0;
  if (stopwatchRunning) {
    stopwatchStartTime = millis();
  }
  drawTracker();
}

void displayStopwatch() {
  if (stopwatchRunning) {
    elapsedMillis += millis() - stopwatchStartTime;
    stopwatchStartTime = millis();
  }

  int elapsedSeconds = elapsedMillis / 1000;
  int minutes = elapsedSeconds / 60;
  int seconds = elapsedSeconds % 60;

  display.setCursor(40, 16); // Adjust these values to position the stopwatch in the middle of the screen
  display.print(minutes);
  display.print(":");
  if (seconds < 10) display.print("0");
  display.print(seconds);

  double temperature = readTemperature();
  display.setCursor(40, 40);  // Adjust x and y positions accordingly
  display.print(temperature, 1);  // Print temperature with one decimal place
  display.print("°C");
}

void toggleStopwatch() {
  if (stopwatchRunning) {
    // Stop the stopwatch
    elapsedMillis += millis() - stopwatchStartTime;
    stopwatchRunning = false;
  } else {
    // Start the stopwatch
    stopwatchStartTime = millis();
    stopwatchRunning = true;
  }
  drawTracker();  // Refresh the tracker screen to update the UI
}

// Accelerometer Functions
double readTemperature() {
  accel_sensor.read();
  temp = ((accel_sensor.rawTemp * 0.5) + 24.0);
  return temp;
}


// Fourth screen
void drawEco() {
  if (currentScreen == ECO_SCREEN) {  // Guard the drawing with a condition to prevent unnecessary redrawing
    display.clearScreen();
    drawLeftArrow(4, 4); // Top-left arrow pointing left
    drawTextBesideArrow("Back", 14, 3, TS_8b_Black);
  }
}


void displayEco() {
  display.setCursor(5, 15);
  display.print("Weight: ");
  display.print(weight);
  display.print("kg");

  // Calories of user
  double calories = 3.9 * weight * 3.5 / 200;
  // CO2 emission per step
  double co2_per_step = (calories / 100) * 2.2;

  // Total CO2 emission
  double total_co2 = totalSteps * co2_per_step;
  // Trees saved
  int trees = round((totalSteps * co2_per_step) / 21.77);
  // Equivalent car miles
  double car_miles = total_co2 / 2.3;

  display.setCursor(5, 27);
  display.print(trees);
  display.print(" trees saved");
  display.setCursor(5, 37);
  display.print(total_co2, 1);
  display.print(" CO2 emissions");
  display.setCursor(5, 47);
  display.print("=");
  display.print(car_miles, 1);
  display.print("car miles");
}

void refreshEcoScreen(int weight){
  display.clearScreen();
  displayEco();
}





