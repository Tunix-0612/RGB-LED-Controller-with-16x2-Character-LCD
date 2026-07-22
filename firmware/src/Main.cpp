/*
    BYTunix - Tunahan Bayraktar
    RGB LED and 2x16LCD Controller
    12/11/2025 - x
    Arduino Nano
    Version 1.4.5-Build.26.7.H
    Update Name --- HOT-FIX

    ---------------------------------------------------------
    CHANGELOG
    ---------------------------------------------------------
    [Added]
    - 

    [Changed]
    - 

    [Removed]
    - Removed unused code

    [Fixed]
    - Fixed device version miss registered

    [v1.4.5] - HOT-FIX
    --------------------------------------------------------------------------------
    * Fix Errors

    (16 - L) / 2 --- Center text function
*/

#include "Constants.h"
#include "Display.h"
#include "LEDControl.h"
#include "Menu.h"
#include "TunixErrorManager.h"
#include "TunixMemoryManager.h"
#include "TunixSelfTest.h"

TunixSelfTest deviceTest;
TunixMemoryManager memory(1024);
TunixErrorManager errorManager;

DisplayManagerSystem display;
LEDControllerSystem LEDController;
MenuSystem menu;

void introductionDisplay()
{
  lcd.clear();
  analogWrite(LCD_BACKLIGHT_PIN, 255);
  lcd.setCursor(5, 0);
  lcd.print(F("Tunix"));
  lcd.setCursor(3, 1);
  lcd.print(F("Electronics"));
  delay(2000);
  return;
}

void setup() 
{
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  memory.getBasicMemory();
  display.initDisplay();
  introductionDisplay();
  memory.firmwareValidate();
  byte selfTestValue = deviceTest.selfTest();
  if (selfTestValue != 0) errorManager.errorHandler(selfTestValue);
  memory.loadRGBConfig(memory.settings.selectedConfig);
  LEDController.RGBBrigthnessRead();
  menu.setClockMenu(true);
  lcd.clear();
  menu.infoDisplay();
}

void loop() 
{
  errorManager.errorHandler(PROGRAM_LOOP_FAILURE_CODE);
  while(true) delay(2000);
}