#include "Display.h"

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

DisplayManagerSystem::DisplayManagerSystem() {}

// SETTINGS SCREEN TEXT DATA
static const char m1[] PROGMEM = " RGB Config";
static const char m2[] PROGMEM = " Set Clock";
static const char m3[] PROGMEM = " Idle Screen";
static const char m4[] PROGMEM = " LED Brghtnss.";
static const char m5[] PROGMEM = " LDR Management";
static const char m6[] PROGMEM = "Screen Brightns";
static const char m7[] PROGMEM = "Screen Turn Off";
static const char m8[] PROGMEM = " Timer Settings";
static const char m9[] PROGMEM = " New RGB Config";
static const char m10[] PROGMEM = " Delete Config";
static const char m11[] PROGMEM = " Info";

static const char* const menuItems[] PROGMEM = { m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11 };

// SPECIAL CHARACTERS
static const byte arrowUp[8] PROGMEM    = { B00100, B01110, B11111, B00100, B00100, B00000, B00000, B00000 };
static const byte arrowDown[8] PROGMEM  = { B00000, B00000, B00100, B00100, B11111, B01110, B00100, B00000 };
static const byte arrowLeft[8] PROGMEM  = { B00000, B00100, B01100, B11111, B01100, B00100, B00000, B00000 };
static const byte arrowRight[8] PROGMEM = { B00000, B00100, B00110, B11111, B00110, B00100, B00000, B00000 };
static const byte clockSymbol[8] PROGMEM = { B00000, B01110, B10101, B10101, B10111, B10001, B01110, B00000 };
static const byte tickSymbol[8] PROGMEM  = { B00000, B00000, B00001, B00011, B10110, B01100, B00000, B00000 };
static const byte LDRSymbol[8] PROGMEM   = { B00000, B11101, B10101, B10101, B10101, B10111, B00000, B00000 };
static const byte gearSymbol[8] PROGMEM  = { B00100, B01110, B11011, B01010, B11011, B01110, B00100, B00000 };

static const byte* const allCharacters[] PROGMEM = 
{
  arrowUp, arrowDown, arrowLeft, arrowRight,
  clockSymbol, tickSymbol, LDRSymbol, gearSymbol
};

void DisplayManagerSystem::loadCustomCharacters() 
{
  byte buffer[8];

  for (byte charIndex = 0; charIndex < 8; charIndex++) 
  {
    const byte* charAddress = (const byte*)pgm_read_word(&(allCharacters[charIndex]));

    for (byte i = 0; i < 8; i++) buffer[i] = pgm_read_byte_near(charAddress + i);

    lcd.createChar(charIndex, buffer);
  }
  return;
}


void DisplayManagerSystem::initDisplay() 
{
  lcd.begin(16, 2);
  loadCustomCharacters();
  lcd.display();
  return;
}

const __FlashStringHelper* DisplayManagerSystem::getMenuString(byte menuIndex) {

    if (menuIndex >= 11) return F(""); 

    return (const __FlashStringHelper*)pgm_read_word(&(menuItems[menuIndex]));
}

void DisplayManagerSystem::printMenuLine(byte menuIndex) { lcd.print(getMenuString(menuIndex)); }