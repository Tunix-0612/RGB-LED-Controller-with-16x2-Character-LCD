#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <Arduino.h>

#include "Constants.h"

class LEDControllerSystem
{
  private:
    uint8_t currentRed = 0;
    uint8_t currentGreen = 0;
    uint8_t currentBlue = 0;
    uint8_t ledBrightness = 255;
    bool wasAnimating = false;

  public:
    LEDControllerSystem();

    bool ledOff = true;

    void RGBColorApply(uint8_t targetRed, uint8_t targetGreen, uint8_t targetBlue);
    void fadeAnimationEngine();
    void RGBBrigthnessRead();
    void ledChange();
    bool getLEDState();
    uint8_t getLEDBrightness();
};

extern LEDControllerSystem LEDController;

#endif