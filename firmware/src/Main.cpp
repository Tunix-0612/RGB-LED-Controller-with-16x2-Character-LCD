/*
    Tunix - Tunahan Bayraktar
    RGB LED and 2x16LCD Controller
    Arduino Nano
    Version: v1.6.0-beta.2+build.2609E

---------------------------------------------------------
# CHANGELOG
---------------------------------------------------------
### Added
- 

### Changed
- Brightness menu UI Draw methodes and variables has been tweaked for the new brightness mode type.
- LED Change function has been tweaked tweaked for the new brightness mode type.
- Info Screen's UI has been updated to show phase+iteration numbers more user friendly.

### Removed
- 

### Fixed
- The bug caused the LED Brightness settings cannot be changed compeletly fixed.
- 

### Notes
- 

### SOME CHANGES MAY NOT BE LISTED HERE.

*/

#include "Constants.h"
#include "Display.h"
#include "LEDControl.h"
#include "Menu.h"

// v1.0.0
#include "InputManager.h" 

// v4.0.5
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
  StatusCode selfTestValue = deviceTest.selfTest();
  if (selfTestValue != StatusCode::NONE) errorManager.errorHandler(selfTestValue);
  
  inputManager.begin();

  pinMode(Pins::RGB_R, OUTPUT);
  pinMode(Pins::RGB_G, OUTPUT);
  pinMode(Pins::RGB_B, OUTPUT);
  pinMode(Pins::LDR, INPUT);
  pinMode(Pins::LCD_BACKLIGHT, OUTPUT);

  display.initDisplay();
  introductionDisplay();

  memory.begin();

  StatusCode firmwareErrorCode = memory.firmwareValidate();
  if (firmwareErrorCode != StatusCode::NONE && firmwareErrorCode != StatusCode::OPERATION_OK) errorManager.errorHandler(firmwareErrorCode);

  if (firmwareErrorCode == StatusCode::OPERATION_OK)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Firmware Updated"));
    delay(1000);
  }

  memory.loadRGBConfig(memory.settings.selectedConfig);

  menu.setClockMenu(true);
  lcd.clear();
  menu.infoDisplay();
}

void loop() 
{
  errorManager.errorHandler(StatusCode::PROGRAM_LOOP_FAILURE);
  while(true) delay(2000);
}