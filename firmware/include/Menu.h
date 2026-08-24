#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

class MenuSystem
{
  private:
    static uint32_t uptime, oldUptime, uptimeFlag;
    bool coldBootClockSet;
    uint32_t lastMinuteMillis = 0;
    uint32_t lastControlMillis = 0;
    bool lastIntervalState = false;

    uint8_t settingsMenuCursor = 1;
    uint8_t RGBConfigMenuCursor = 1;

    uint16_t currentTime = 720;
    uint16_t LDRValue;
    uint8_t LDRLimitPerc;
    uint8_t lcdBacklightPerc;
    uint16_t timeOut;
    bool LDRActivated;
    bool LDRDisabled = false;

    bool updateClockAndControl();
    bool checkLDR();
    void infoDisplayWrite();
    void RGBConfigMenuWrite();
    uint8_t getNextValidAscii(uint8_t currentVal, bool increment);
    void brightnessMenuWrite();
    void screenBrightnessMenuWrite();
    void screenOffStateMenuWrite();
    void LDRManagementMenuText();
    void timerSettingsMenuWrite(bool focusOnStart);
    void setClockMenuWrite(bool focusOnActivation);
    void settingsMenuWrite();
    void deleteRGBConfigMenu();
    void infoScreen();

    void RGBConfigMenu();
    void idleScreenTextMenu();
    void brightnessMenu();
    void screenBrightnessMenu();
    void screenOffStateMenu();
    void LDRManagementMenu();
    void timerSettingsMenu();
    void createNewRGBConfigMenu();
    void settingsMenu();
    void idleTextPrint();

  public:
    MenuSystem();

    void infoDisplay();
    void idleScreen();
    void setClockMenu(bool coldBoot);
};

extern MenuSystem menu;

#endif