/*
    BYTunix - Tunahan Bayraktar
    RGB LED and 2x16LCD Controller
    12/11/2025 - x
    Arduino Nano
    Version 1.4.4-Build.26.7.G
    Update Name --- POWER LOSS LOGIC & INTELLIGENT BOOTSTRAPPING

    ---------------------------------------------------------
    CHANGELOG
    ---------------------------------------------------------
    [Added]
    - Cold Boot set clock screen has been added, used the same function with the settings menus' but with variable timeOut.

    [Changed]
    - infoScreen now resets the timeOut at start
    - infoScreen's timeOut check to send the device to idle now checks for ..> 1500 instad of ..== 1500

    [Removed]
    - Trıggering LED Change with internal clock at info display has been removed as info display is waiting for user input.

    [Fixed]
    - Internal clock jump to 23:45 from 00:00 when decreasing has been fixed.
    - Blank LCD after LED Change triggered has been fixed.

    [v1.4.4] - POWER LOSS LOGIC & INTELLIGENT BOOTSTRAPPING
    --------------------------------------------------------------------------------
    * Implementation of volatile time loss detection mechanism (Asynchronous RTC emulator alert).
    * UI Warning Signal: Flashing clock icon on 'idleScreen' and 'infoDisplay' upon sudden cold boots.
    * Smart Boot Architecture: Timed intercept block on startup for manual clock entry; 
      graceful fallback to predefined defaults upon timeout expiration.

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