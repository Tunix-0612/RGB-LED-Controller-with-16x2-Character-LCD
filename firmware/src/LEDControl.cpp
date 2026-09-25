#include "LEDControl.h"

#include "TMemoryManager.h"
#include "Display.h"

LEDControllerSystem::LEDControllerSystem() { }

void LEDControllerSystem::RGBColorApply(uint8_t targetRed, uint8_t targetGreen, uint8_t targetBlue, uint8_t brightness)
{
  if(ledOff == true)
  {
    analogWrite(Pins::RGB_R, 0);
    analogWrite(Pins::RGB_G, 0);
    analogWrite(Pins::RGB_B, 0);
    return;
  }

  ledBrightness = brightness;

  uint16_t scaledRed = ((uint16_t)targetRed * ledBrightness) / 255;
  uint16_t scaledGreen = ((uint16_t)targetGreen * ledBrightness) / 255;
  uint16_t scaledBlue = ((uint16_t)targetBlue * ledBrightness) / 255;
  analogWrite(Pins::RGB_R, scaledRed);
  analogWrite(Pins::RGB_G, scaledGreen);
  analogWrite(Pins::RGB_B, scaledBlue);
  return;
}

void LEDControllerSystem::fadeAnimationEngine()
{
  bool colorChanged = false;
  if (currentRed < memory.activeConfig.R) 
  {
    currentRed++;
    colorChanged = true;
  } 
  else if (currentRed > memory.activeConfig.R) 
  {
    currentRed--;
    colorChanged = true;
  }
  if (currentGreen < memory.activeConfig.G) 
  {
    currentGreen++;
    colorChanged = true;
  } 
  else if (currentGreen > memory.activeConfig.G) 
  {
    currentGreen--;
    colorChanged = true;
  }
  if (currentBlue < memory.activeConfig.B) 
  {
    currentBlue++;
    colorChanged = true;
  } 
  else if (currentBlue > memory.activeConfig.B) 
  {
    currentBlue--;
    colorChanged = true;
  }
  lcd.setCursor(15, 0);
  if (colorChanged) 
  {
    if (!wasAnimating)
    {
      lcd.setCursor(15, 0);
      lcd.write(CLOCK_CHAR);
      wasAnimating = true;
    }
    RGBColorApply(currentRed, currentGreen, currentBlue, memory.settings.brightnessModeValue[memory.settings.selectedBrightnessIndex]);
  }
  else if (wasAnimating)
  {
    lcd.setCursor(15, 0);
    lcd.write(32); 
    wasAnimating = false;
  }
  return;
}

void LEDControllerSystem::ledChange()
{
  ledOff = !ledOff;
  RGBColorApply(currentRed, currentGreen, currentBlue, memory.settings.brightnessModeValue[memory.settings.selectedBrightnessIndex]);
  lcd.display();
  analogWrite(Pins::LCD_BACKLIGHT, memory.settings.lcdBacklight);
  lcd.clear();
  lcd.setCursor(3, 0);
  if(ledOff) lcd.print(F("LEDs OFF"));
  else lcd.print(F("LEDs ON"));
  delay(750);
  lcd.clear();
  return;
}

bool LEDControllerSystem::getLEDState()
{
  if (ledOff) return true;
  else return false;
}