/*
    BYTunix - Tunahan Bayraktar
    RGB LED and 2x16LCD Controller
    12/11/2025 - x
    Arduino Nano
    Version 1.4.3 Stable - Build.26.7.F
    Update Name --- OBJECT ORIENTED REFACTORING & DIAGNOSTICS

    ---------------------------------------------------------
    CHANGELOG
    ---------------------------------------------------------
    [Added]
    - 

    [Changed]
    - Setting Menu's main text data has been migrated to Display.cpp
    - Setting Menu's text processing has been modified with F() macros for better RAM optimization
      This resulted in +626 bytes of RAM
    - Setting Menu's main text is now read with new method using pointers and AVR functions
      This resulted in ~+100 bytes of FLASH MEM
    - Setting Menu's detection of next menu is now controlled with switch-case instead of indivudual if blocks
      This resulted in +380 bytes of FLASH MEM
    - Optimized the internal clock

    [Removed]
    - digitalWrite commands has been removed and used analogWrite instead

    [Fixed]
    - Special Character's visual bug fixed with new method of loading them into LCD's memory
    - Fixed the visual bug on Lighting Fade Effect
    - Possible clock overflow resulting in internal clock stops after 47 days has been fixed

    [v1.4.3] - SYSTEM PURIFICATION & RE-OPTIMIZATION
    --------------------------------------------------------------------------------
    * Hotfix deployment dedicated to the v1.4.2 OOP structural transformation.
    * Deep refactoring of scope: Localization of global variables to diminish SRAM footprints.
    * Advanced optimization of data types (Byte conversion pass to minimize structural footprints).
    ---------------------------------------------------------

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
  menu.infoDisplay();
}

void loop() 
{
  errorManager.errorHandler(PROGRAM_LOOP_FAILURE_CODE);
  while(true) delay(2000);
}