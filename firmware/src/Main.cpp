/*
    Tunix - Tunahan Bayraktar
    RGB LED and 2x16LCD Controller
    Arduino Nano
    Version: v1.5.1+build.2609B

---------------------------------------------------------
# CHANGELOG
---------------------------------------------------------
### Added
- 

### Changed
- Brightness menu's screen handler now uses parameter to display the current brightness value.

### Removed
-

### Fixed
- Slight Screen Optimization for the LED Brightness Menu
- Now the LED Brightness Menu displays the current brightness value in real-time as the user adjusts it, providing immediate feedback on the changes being made.

### SOME CHANGES MAY NOT BE LISTED HERE.

*/

#include "Constants.h"
#include "Display.h"
#include "LEDControl.h"
#include "Menu.h"

#include "InputManager.h" // v1.0.0

// v4.0.0
#include "TErrorManager.h"
#include "TMemoryManager.h"
#include "TSelfTest.h"

TSelfTest deviceTest;
TMemoryManager memory(1024);
TErrorManager errorManager;

DisplayManagerSystem display;
LEDControllerSystem LEDController;
MenuSystem menu;
InputManager inputManager(Pins::BUTTON_UP, Pins::BUTTON_DOWN, Pins::BUTTON_LEFT, Pins::BUTTON_RIGHT);

void introductionDisplay()
{
  lcd.clear();
  analogWrite(Pins::LCD_BACKLIGHT, 255);
  lcd.setCursor(5, 0);
  lcd.print(F("Tunix"));
  lcd.setCursor(3, 1);
  lcd.print(F("Electronics"));
  delay(2000);
}

void setup() 
{
  inputManager.begin();

  pinMode(Pins::RGB_R, OUTPUT);
  pinMode(Pins::RGB_G, OUTPUT);
  pinMode(Pins::RGB_B, OUTPUT);
  pinMode(Pins::LDR, INPUT);
  pinMode(Pins::LCD_BACKLIGHT, OUTPUT);

  display.initDisplay();
  introductionDisplay();

  memory.begin();
  ErrorCode firmwareErrorCode = memory.firmwareValidate();
  if (firmwareErrorCode != ErrorCode::NONE) errorManager.errorHandler(firmwareErrorCode);

  ErrorCode selfTestValue = deviceTest.selfTest();
  if (selfTestValue != ErrorCode::NONE) errorManager.errorHandler(selfTestValue);
  memory.loadRGBConfig(memory.settings.selectedConfig);
  LEDController.RGBBrigthnessRead();
  menu.setClockMenu(true);
  lcd.clear();
  menu.infoDisplay();
}

void loop() 
{
  errorManager.errorHandler(ErrorCode::PROGRAM_LOOP_FAILURE);
  while(true) delay(2000);
}