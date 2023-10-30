// Add libraries
#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
#include <STBLE.h>
#include "BMA250.h"
#include "sprites.h"
#include <RTCZero.h>
RTCZero rtc;

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
bool dataSent = false;
bool prevButtonState = false;

// Stopwatch variables
bool stopwatchRunning = false;
unsigned long stopwatchStartTime = 0;
unsigned long elapsedMillis = 0;
int minutes = 0;
int seconds = 0;

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

// Progress bar variables
unsigned long previousMillis = 0;
const unsigned long progressUpdateInterval = 3000; // Interval for updating the progress bar (3 seconds)
int prevFilledWidth = 0; // Store the previous filled width of the progress bar
bool goalReached = false; // Flag to indicate if the goal has been reached
unsigned long lastUpdateMillis = 0; // Store the last time the progress bar was updated

// Eco
int weight = 70;

//Time
const byte time_hours = 13;
const byte time_minutes = 51;
const byte time_seconds = 0;

// Batt
float getBattPercent();
int batteryPercentage = getBattPercent();

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
  
  rtc.begin();
  rtc.setTime(time_hours, time_minutes, time_seconds);

  BLEsetup();
}

void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  readSteps();

  switch (currentScreen) {
    case ANIMATION_SCREEN:
      displayAnimationScreen();
      drawBatterySymbol(display, 14, 1, batteryPercentage);
      displayTime();
      break;
    case MENU_SCREEN:
      // Displays menu
      displayMenu();
      drawBatterySymbol(display, 14, 1, batteryPercentage);
      displayTime();
      break;
    case TRACKER_SCREEN:
      displayStopwatch(); // Update and display the stopwatch
      checkForSteps();
      break;
    case ECO_SCREEN:
      displayEco();
      drawBatterySymbol(display, 14, 1, batteryPercentage);
      displayTime();
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
  bool currButtonState = display.getButtons() & TSButtonUpperRight;

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
        } else if (currButtonState && !prevButtonState) {
          if (!dataSent){
            char time[20]; // Send stopwatch time when top right button is pressed
            snprintf(time, sizeof(time), "%d:%d", minutes, seconds);
            lib_aci_send_data(0, (uint8_t*)time, strlen(time));
            dataSent = true;
          }
        }
        if (!currButtonState){
          dataSent = false;
        }
        break;
      case ECO_SCREEN:
        if (display.getButtons() & TSButtonUpperLeft) {
          transitionToMenuScreen();
          lastButtonPressTime = currentTime;
        }
    }
    prevButtonState = currButtonState;
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
  drawTextBesideArrow("Vitatrax", 26, 10, TS_8b_White);
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
    // Display Progress Bar
    displayProgressBar(totalSteps, stepGoal);
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
  // drawTracker();
}

void displayStopwatch() {
  if (stopwatchRunning) {
    elapsedMillis += millis() - stopwatchStartTime;
    stopwatchStartTime = millis();
  }

  int elapsedSeconds = elapsedMillis / 1000;
  minutes = elapsedSeconds / 60;
  seconds = elapsedSeconds % 60;

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

// Progress bar Function
void displayProgressBar(int totalSteps, int stepGoal) {
  unsigned long currentMillis = millis();

  // Always draw the progress bar
  int barWidth = 80;
  int barHeight = 10;
  int xPos = (96 - barWidth) / 2;
  int yPos = 28;

  double progress = (double)totalSteps / stepGoal;

  // Limit the filled width to match the stepGoal
  int filledWidth = min(barWidth, static_cast<int>(barWidth * progress));

  // Determine the progress bar color based on goal completion
  uint16_t progressBarColor = (totalSteps >= stepGoal) ? TS_8b_Green : TS_8b_Red;

/*
  // Clear the previous progress
  for (int x = xPos; x < xPos + prevFilledWidth; x++) {
    for (int y = yPos; y < yPos + barHeight; y++) {
      display.drawPixel(x, y, TS_8b_Black);
    }
  }
*/
  // Draw the filled part of the progress bar
  for (int x = xPos; x < xPos + filledWidth; x++) {
    for (int y = yPos; y < yPos + barHeight; y++) {
      display.drawPixel(x, y, progressBarColor);
    }
  }

  // Draw the border of the progress bar in white
  for (int x = xPos - 1; x < xPos + barWidth + 2; x++) {
    display.drawPixel(x, yPos - 1, TS_8b_White);  // Top border
    display.drawPixel(x, yPos + barHeight, TS_8b_White);  // Bottom border
  }

  for (int y = yPos - 1; y < yPos + barHeight + 1; y++) {
    display.drawPixel(xPos - 1, y, TS_8b_White);  // Left border
    display.drawPixel(xPos + barWidth, y, TS_8b_White);  // Right border
  }

  prevFilledWidth = filledWidth;

  // Check if the goal has been reached
  if (totalSteps >= stepGoal) {
    goalReached = true;
  }

  // Update the last update time, but only if the time interval has passed
  if (currentMillis - lastUpdateMillis >= progressUpdateInterval) {
    lastUpdateMillis = currentMillis;
  }
  /* For troubleshooting
  // Update the live text in real-time
  display.fontColor(TS_8b_Green, TS_8b_Black);
  display.setCursor(15, 30);
  display.print("Prog: ");
  display.print(totalSteps);
  display.print(" / ");
  display.print(stepGoal);
  */
}
  
// Function to check for steps and update totalSteps
void checkForSteps() {
  int stepDetected = readSteps(); // This function checks for steps and returns 1 if a step is detected

  if (stepDetected) {
    displayProgressBar(totalSteps, stepGoal); // Update the progress bar with the new totalSteps
  }
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

void display2digits(int number) {
  if (number < 10) {
    display.print("0");
  }
  display.print(number);
}

void displayTime() {
  display.setFont(liberationSans_8ptFontInfo); // Choose a font size
  display.setCursor(50, 0); // Set the cursor position
  display2digits(rtc.getHours());
  display.print(":");
  display2digits(rtc.getMinutes());
}

float getVCC() {
  SYSCTRL->VREF.reg |= SYSCTRL_VREF_BGOUTEN;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SAMPCTRL.bit.SAMPLEN = 0x1;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->INPUTCTRL.bit.MUXPOS = 0x19;         // Internal bandgap input
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SWTRIG.bit.START = 1;  // Start conversion
  ADC->INTFLAG.bit.RESRDY = 1;  // Clear the Data Ready flag
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->SWTRIG.bit.START = 1;  // Start the conversion again to throw out first value
  while ( ADC->INTFLAG.bit.RESRDY == 0 );   // Waiting for conversion to complete
  uint32_t valueRead = ADC->RESULT.reg;
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  while (ADC->STATUS.bit.SYNCBUSY == 1);
  SYSCTRL->VREF.reg &= ~SYSCTRL_VREF_BGOUTEN;
  float vcc = (1.1 * 1023.0) / valueRead;
  return vcc;
}
// Calculate the battery voltage
float getBattVoltage(void) {
  const int VBATTpin = A4;
  float VCC = getVCC();

  // Use resistor division and math to get the voltage
  float resistorDiv = 0.5;
  float ADCres = 1023.0;
  float battVoltageReading = analogRead(VBATTpin);
  battVoltageReading = analogRead(VBATTpin); // Throw out first value
  float battVoltage = VCC * battVoltageReading / ADCres / resistorDiv;

  return battVoltage;
}

// Calculate the battery voltage
float getBattPercent()
{
  float batteryLeft = max((getBattVoltage() - 3.00), 0);
  return min((batteryLeft * 83.333333), 100); // hard upper limit of 100 as it often shows over 100 when charging
}

void drawBatterySymbol(TinyScreen &display, uint8_t x, uint8_t y, int percentage) {
    // Draw battery outline
    // x-axis, y-axis, length, height, fill, color 
    display.drawRect(x + 67, y - 1, 13, 10, 1, TS_8b_Green);
    // Draw battery terminal
    display.drawRect(x + 80, y + 1, 2, 4, 1, TS_8b_Green);
    
    // Calculate filled rectangle width based on percentage
    uint8_t fillWidth = 11 * percentage / 100;

    // Draw filled rectangle using drawRect in a loop
    display.drawRect(x + 68, y , fillWidth, 8, 1, TS_8b_Red);

}




