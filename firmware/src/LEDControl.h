#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <Arduino.h>

#include "Constants.h"

class LEDControllerSystem
{
  private:
    byte currentRed = 0;
    byte currentGreen = 0;
    byte currentBlue = 0;
    byte ledBrightness = 255;
    bool wasAnimating = false;

  public:
    LEDControllerSystem();

    bool ledOff = true;

    void RGBColorApply(byte targetRed, byte targetGreen, byte targetBlue);
    void fadeAnimationEngine();
    void RGBBrigthnessRead();
    void ledChange();
    bool getLEDState();
    byte getLEDBrightness();
};

extern LEDControllerSystem LEDController;

#endif