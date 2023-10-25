// Add libraries
#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
#include <STBLE.h>
#include "BMA250.h"
#include "sprites.h"

// Initialise bluetooth
#ifndef BLE_DEBUG
#define BLE_DEBUG true
#endif

#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif

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
int stepGoal = 1000;
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

// Screens
enum ScreenType {
  ANIMATION_SCREEN,
  MENU_SCREEN,
  TRACKER_SCREEN
};

ScreenType currentScreen = ANIMATION_SCREEN;

// Setup and Loop
void setup() {
  initializeDisplay();
  initializeAccel();
  drawInitialScreen();
  SerialMonitorInterface.begin(9600);
  while (!SerialMonitorInterface); //This line will block until a serial monitor is opened with TinyScreen+!
  BLEsetup();
}

void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  if (ble_rx_buffer_len) {//Check if data is available
    SerialMonitorInterface.print(ble_rx_buffer_len);
    SerialMonitorInterface.print(" : ");
    SerialMonitorInterface.println((char*)ble_rx_buffer);
    ble_rx_buffer_len = 0;//clear afer reading
  }
  if (SerialMonitorInterface.available()) {//Check if serial input is available to send
    delay(10);//should catch input
    uint8_t sendBuffer[21];
    uint8_t sendLength = 0;
    while (SerialMonitorInterface.available() && sendLength < 19) {
      sendBuffer[sendLength] = SerialMonitorInterface.read();
      sendLength++;
    }
    if (SerialMonitorInterface.available()) {
      SerialMonitorInterface.print(F("Input truncated, dropped: "));
      if (SerialMonitorInterface.available()) {
        SerialMonitorInterface.write(SerialMonitorInterface.read());
      }
    }
    sendBuffer[sendLength] = '\0'; //Terminate string
    sendLength++;
    if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)sendBuffer, sendLength))
    {
      SerialMonitorInterface.println(F("TX dropped!"));
    }
  }



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
    }
  }
}

void transitionToMenuScreen() {
  resetStopwatch();
  currentScreen = MENU_SCREEN;
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
  display.clearScreen();

  // This can be expanded for more menu items later on
  drawLeftArrow(4, 4); // Top-left arrow pointing left
  drawTextBesideArrow("Log", 14, 3, TS_8b_Black);

  drawRightArrow(88, 54); // Bottom-right arrow
  drawTextBesideArrow("Go", 70, 53, TS_8b_Black);
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

// Displays main menu
void displayMenu() {
  readSteps();

  display.setCursor(5, 25);
  display.print("Alarm: ");

  display.setCursor(5, 35);
  display.print("Goal: ");
  display.print(stepGoal);

  display.setCursor(5, 45);
  display.print("Prog: ");
  display.print(totalSteps);
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



