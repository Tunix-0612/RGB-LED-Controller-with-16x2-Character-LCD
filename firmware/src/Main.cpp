/*
    Tunix - Tunahan Bayraktar
    RGB LED and 2x16LCD Controller
    Arduino Nano
    Version: v1.4.6-stable+build.26.8.A
    USER INTERACTION & INPUT ABSTRACT LAYER
    --------------------------------------------------------------------------------
    * Complete overhaul of button handling routines and hardware debouncing algorithms.
    * General UI aesthetics upgrade, transition smoothing, and template-driven menu modules.

    ---------------------------------------------------------
    CHANGELOG
    ---------------------------------------------------------
    [Added]
    - LDR Check now has it's own separate function.
    - Two new files: InputManager.h/.cpp has been added for complete overhaul of button event manage.
    - VersionInfo struct has been added to Constants.h for better version management.
    - Added new function for getting next valid ASCII character in Menu.h for better character selection in menus.
    - Internal Factory Reset function is now also available in Settings Menu.
    - Set Clock Menu has been updated to have dynamic acceleration for faster time setting.
    - Added a visual settings menu bar for better user experience.

    [Changed]
    - LDR Check procedure has been changed.
    - All menu functions has been updated to be in match with new button logics.
    - #define statements has been replaced with constexpr for better type safety and scope control.
    - Some old comments has been updated for better understanding.

    [Removed]
    - Some old commands deleted for cleanup.
    - Unrequired delay() commands removed for better responsiveness.
    - Some unrequired commands has been removed.
    - Some old comments has been removed.
    - Some old variables has been removed.
    - Some old functions has been removed.
    - Some old structs has been removed.
    - Some old #define statements has been removed.

    [Fixed]
    - Percentage icon at the LDR Limit menu has been aligned before the number.
    - Gibberish character set for the idle screen has been fixed.


    (16 - L) / 2 --- Center text function
*/

#include "Constants.h"
#include "Display.h"
#include "LEDControl.h"
#include "Menu.h"
#include "TunixErrorManager.h"
#include "TunixMemoryManager.h"
#include "TunixSelfTest.h"
#include "InputManager.h"

TunixSelfTest deviceTest;
TunixMemoryManager memory(1024);
TunixErrorManager errorManager;

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
  return;
}

void setup() 
{
  inputManager.begin();
  pinMode(Pins::RGB_R, OUTPUT);
  pinMode(Pins::RGB_G, OUTPUT);
  pinMode(Pins::RGB_B, OUTPUT);
  pinMode(Pins::LDR, INPUT);
  pinMode(Pins::LCD_BACKLIGHT, OUTPUT);
  memory.getBasicMemory();
  display.initDisplay();
  introductionDisplay();
  memory.firmwareValidate();
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