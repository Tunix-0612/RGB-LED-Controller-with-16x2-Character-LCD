#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

class MenuSystem
{
  private:
    static unsigned long uptime, oldUptime, uptimeFlag;
    unsigned long lastMinuteMillis = 0;
    unsigned long lastControlMillis = 0;
    bool lastIntervalState = false;

    byte settingsMenuCursor = 1;
    byte RGBConfigMenuCursor = 1;

    int currentTime = 720;
    int LDRValue;
    byte LDRLimitPerc;
    byte lcdBacklightPerc;
    int timeOut;
    bool LDRActivated;
    bool LDRDisabled = false;

    void updateClockAndControl();
    void infoDisplayWrite();
    void RGBConfigMenuWrite();
    void brightnessMenuWrite();
    void screenBrightnessMenuWrite();
    void screenOffStateMenuWrite();
    void LDRManagementMenuText();
    void timerSettingsMenuWrite(bool focusOnStart);
    void setClockMenuWrite();
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
    void setClockMenu();
    void createNewRGBConfigMenu();
    void settingsMenu();

  public:
    MenuSystem();

    void infoDisplay();
    void idleScreen();
};

extern MenuSystem menu;

#endif