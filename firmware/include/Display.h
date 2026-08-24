#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal.h>

// (16 - L) / 2 --- Center text function

constexpr uint8_t ARROW_UP_CHAR    = 0;
constexpr uint8_t ARROW_DOWN_CHAR  = 1;
constexpr uint8_t ARROW_LEFT_CHAR  = 2;
constexpr uint8_t ARROW_RIGHT_CHAR = 3;
constexpr uint8_t CLOCK_CHAR       = 4;
constexpr uint8_t TICK_CHAR        = 5;
constexpr uint8_t LDR_CHAR         = 6;
constexpr uint8_t GEAR_CHAR        = 7;

class DisplayManagerSystem
{
  private:
    void loadCustomCharacters();
    
  public:
    DisplayManagerSystem();
    void initDisplay();
    
    void printMenuLine(uint8_t menuIndex);
    const __FlashStringHelper* getMenuString(uint8_t menuIndex);
};

extern LiquidCrystal lcd;
extern DisplayManagerSystem display;

#endif