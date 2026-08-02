#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal.h>

constexpr byte ARROW_UP_CHAR    = 0;
constexpr byte ARROW_DOWN_CHAR  = 1;
constexpr byte ARROW_LEFT_CHAR  = 2;
constexpr byte ARROW_RIGHT_CHAR = 3;
constexpr byte CLOCK_CHAR       = 4;
constexpr byte TICK_CHAR        = 5;
constexpr byte LDR_CHAR         = 6;
constexpr byte GEAR_CHAR        = 7;

class DisplayManagerSystem
{
  private:
    void loadCustomCharacters();
    
  public:
    DisplayManagerSystem();
    void initDisplay();
    
    void printMenuLine(byte menuIndex);
    const __FlashStringHelper* getMenuString(byte menuIndex);
};

extern LiquidCrystal lcd;
extern DisplayManagerSystem display;

#endif