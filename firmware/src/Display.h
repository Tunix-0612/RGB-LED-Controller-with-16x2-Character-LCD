#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal.h>

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