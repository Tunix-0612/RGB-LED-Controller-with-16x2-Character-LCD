#include "Menu.h"

#include "Display.h"
#include "TMemoryManager.h"
#include "LEDControl.h"
#include "InputManager.h"

using PartID = TMemoryManager::PartitionID;

uint32_t MenuSystem::uptime = 0;
uint32_t MenuSystem::oldUptime = 0;
uint32_t MenuSystem::uptimeFlag = 0;

MenuSystem::MenuSystem() { }

bool MenuSystem::updateClockAndControl()
{
  uint32_t currentMillis = millis();
  constexpr uint32_t CALIBRATED_MINUTE_MS = 59944UL; 
  // This value is calibrated for the internal clock drift of the Arduino Nano ATmega328P. It is not exactly 60000 ms due to the clock's inaccuracy.
  // The calibrated value is determined through empirical testing and may vary slightly between different units.

  if (currentMillis - lastMinuteMillis >= CALIBRATED_MINUTE_MS) 
  {
    lastMinuteMillis += CALIBRATED_MINUTE_MS;
    currentTime++;
    if (currentTime >= 1440) currentTime = 0;
  }

  if (currentMillis - lastControlMillis < 10000UL) return false;
  lastControlMillis = currentMillis;

  bool inInterval = (memory.settings.startTime < memory.settings.endTime)
    ? (currentTime >= memory.settings.startTime && currentTime < memory.settings.endTime)
    : (currentTime >= memory.settings.startTime || currentTime < memory.settings.endTime);

  if (inInterval != lastIntervalState) 
  {
    lastIntervalState = inInterval;
    
    if (inInterval == LEDController.getLEDState()) LEDController.ledChange();
    return true;
  }
  return false;
}

bool MenuSystem::checkLDR()
{
  if (LDRActivated || LDRDisabled) return false; 

  LDRValue = analogRead(Pins::LDR);

  LDRValue = map(LDRValue, 0, 1023, 100, 0);
  
  if (LDRValue > memory.settings.LDRLimit && LEDController.getLEDState()) 
  {
    LEDController.ledChange();
    LDRActivated = true;
    LDRDisabled = true;

    return true;
  }
  return false;
}

void MenuSystem::infoDisplayWrite()
{
  lcd.setCursor(0, 0);
  lcd.print(F("Config: "));
  lcd.setCursor(8, 0);
  lcd.print(memory.settings.selectedConfig + 1);
  lcd.setCursor(0, 1);
  lcd.print(F("Bright: "));
  lcd.setCursor(8, 1);
  lcd.print(memory.settings.selectedBrightnessIndex + 1);
  if (LDRActivated == true) 
  {
    lcd.setCursor(15, 1);
    lcd.write(LDR_CHAR);
  }
}

void MenuSystem::infoDisplay() 
{
  infoDisplayWrite();
  timeOut = 0;
  while (true) 
  {
    inputManager.update();
    ButtonEvent eventUp    = inputManager.getEvent(BTN_UP);
    ButtonEvent eventDown  = inputManager.getEvent(BTN_DOWN);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    if(eventUp == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      if (memory.settings.selectedConfig == 0) memory.settings.selectedConfig = memory.settings.totalConfig - 1;
      else memory.settings.selectedConfig--;

      memory.loadRGBConfig(memory.settings.selectedConfig);
      infoDisplayWrite();
    }

    if(eventDown == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      memory.settings.selectedConfig++;
      if (memory.settings.selectedConfig >= memory.settings.totalConfig) memory.settings.selectedConfig = 0;

      memory.loadRGBConfig(memory.settings.selectedConfig);
      infoDisplayWrite();
    }

    if(eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;

      uint8_t brightness;
      uint8_t &index = memory.settings.selectedBrightnessIndex;

      index++;
      if (index > 3) index = 0;

      if (index == 0) brightness = 255;
      else            brightness = memory.settings.brightnessModeValue[index - 1];

      LEDController.RGBColorApply(memory.activeConfig.R, 
        memory.activeConfig.G, 
        memory.activeConfig.B, 
        brightness);

      infoDisplayWrite();
    }

    if (eventLeft == BTN_EVENT_LONG_PRESS)
    {
      timeOut = 0;
      LEDController.ledChange();
      infoDisplayWrite();
    }

    if (eventRight == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      settingsMenu();
    }

    LEDController.fadeAnimationEngine();
    
    if(memory.settings.LDRActive) checkLDR();

    timeOut++;
    delay(2);
    if (timeOut > 1500) idleScreen();
  }
}

void MenuSystem::idleTextPrint()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(memory.displayText.idleTextUp);
  lcd.setCursor(0, 1);
  lcd.print(memory.displayText.idleTextBottom);
}

void MenuSystem::idleScreen() 
{
  timeOut = 0;
  uint32_t lastClockBlinkMillis = 0;
  bool clockBlinkState = false;
  memory.writeData(PartID::SETTINGS, memory.settings);
  idleTextPrint();
  while (true) 
  {
    inputManager.update();
    ButtonEvent eventUp    = inputManager.getEvent(BTN_UP);
    ButtonEvent eventDown  = inputManager.getEvent(BTN_DOWN);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);
    
    if (eventUp == BTN_EVENT_CLICK || eventDown == BTN_EVENT_CLICK || eventRight == BTN_EVENT_CLICK) break;
    
    if (eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;

      uint8_t brightness;
      uint8_t &index = memory.settings.selectedBrightnessIndex;

      index++;
      if (index > 3) index = 0;

      if (index == 0) brightness = 255;
      else            brightness = memory.settings.brightnessModeValue[index - 1];

      LEDController.RGBColorApply(memory.activeConfig.R, 
        memory.activeConfig.G, 
        memory.activeConfig.B, 
        brightness);
    }

    if (eventLeft == BTN_EVENT_LONG_PRESS)
    {
      timeOut = 0;
      LEDController.ledChange();
      idleTextPrint();
    }

    if ((timeOut >= 1500) && (memory.settings.screenOffState == true)) 
    {
      analogWrite(Pins::LCD_BACKLIGHT, 0);
      lcd.noDisplay();
    }
    if ((timeOut >= 1500) && (memory.settings.screenOffState == false)) analogWrite(Pins::LCD_BACKLIGHT, 20);

    oldUptime = uptime;
    
    if (memory.settings.LDRActive && checkLDR()) idleTextPrint();

    if (memory.settings.timerActive && clockIsSet && updateClockAndControl()) idleTextPrint();

    uptime = millis();
    if (uptime < oldUptime) uptimeFlag++;

    uint32_t currentMillis = millis();
    if (currentMillis - lastClockBlinkMillis >= 750 && !coldBootClockSet && memory.settings.timerActive) 
    {
      lastClockBlinkMillis = currentMillis;
      clockBlinkState = !clockBlinkState;

      lcd.setCursor(15, 0); 
      if (clockBlinkState) lcd.write(CLOCK_CHAR);
      else lcd.print(F(" "));
    }
    delay(10);
    timeOut++;
  }
  lcd.display();
  lcd.clear();
  timeOut = 0;
  analogWrite(Pins::LCD_BACKLIGHT, memory.settings.lcdBacklight);
  infoDisplayWrite();
  inputManager.update();
}

void MenuSystem::RGBConfigMenuWrite() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F("  R   G   B"));
  lcd.setCursor(2, 1);
  lcd.print(memory.activeConfig.R);
  lcd.setCursor(6, 1);
  lcd.print(memory.activeConfig.G);
  lcd.setCursor(10, 1);
  lcd.print(memory.activeConfig.B);
  if (RGBConfigMenuCursor == 1) lcd.setCursor(2, 0);
  if (RGBConfigMenuCursor == 2) lcd.setCursor(6, 0);
  if (RGBConfigMenuCursor == 3) lcd.setCursor(10, 0);
}

void MenuSystem::RGBConfigMenu() 
{
  uint32_t lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  lcd.clear();
  lcd.cursor();
  lcd.blink();
  RGBConfigMenuWrite();
  while (true) 
  {
    inputManager.update();
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    uint32_t currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
        if (inputManager.isPressed(BTN_UP))
        {
          timeOut = 0;
          if (RGBConfigMenuCursor == 1) memory.activeConfig.R = constrain((int)memory.activeConfig.R + 5, 0, 255);
          if (RGBConfigMenuCursor == 2) memory.activeConfig.G = constrain((int)memory.activeConfig.G + 5, 0, 255);
          if (RGBConfigMenuCursor == 3) memory.activeConfig.B = constrain((int)memory.activeConfig.B + 5, 0, 255);
          uint8_t redValue = memory.activeConfig.R;
          uint8_t greenValue = memory.activeConfig.G;
          uint8_t blueValue = memory.activeConfig.B;
          LEDController.RGBColorApply(redValue, greenValue, blueValue, memory.settings.brightnessModeValue[memory.settings.selectedBrightnessIndex]);
          RGBConfigMenuWrite();
          lastFastChangeMillis = currentMillis;
        }

        if (inputManager.isPressed(BTN_DOWN))
        {
          timeOut = 0;
          if (RGBConfigMenuCursor == 1) memory.activeConfig.R = constrain((int)memory.activeConfig.R - 5, 0, 255);
          if (RGBConfigMenuCursor == 2) memory.activeConfig.G = constrain((int)memory.activeConfig.G - 5, 0, 255);
          if (RGBConfigMenuCursor == 3) memory.activeConfig.B = constrain((int)memory.activeConfig.B - 5, 0, 255);
          uint8_t redValue = memory.activeConfig.R;
          uint8_t greenValue = memory.activeConfig.G;
          uint8_t blueValue = memory.activeConfig.B;
          LEDController.RGBColorApply(redValue, greenValue, blueValue, memory.settings.brightnessModeValue[memory.settings.selectedBrightnessIndex]);
          RGBConfigMenuWrite();
          lastFastChangeMillis = currentMillis;
        }
    }

    if(eventRight == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      RGBConfigMenuCursor++;
      if (RGBConfigMenuCursor == 4) RGBConfigMenuCursor = 1;
      RGBConfigMenuWrite();
    }

    if(eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > 2000) break;
  }
  lcd.noCursor();
  lcd.noBlink();
  memory.saveRGBConfig(memory.settings.selectedConfig);
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

uint8_t MenuSystem::getNextValidAscii(uint8_t currentVal, bool increment)
{
  if (currentVal < 32 || currentVal > 122) return 32;

  if (increment) 
  {
    if (currentVal == 32) return 39;
    if (currentVal == 39) return 48;
    if (currentVal == 57) return 65;
    if (currentVal == 90) return 97;
    if (currentVal == 122) return 32;
    return currentVal + 1;
  } 
  else 
  {
    if (currentVal == 32) return 122;
    if (currentVal == 39) return 32;
    if (currentVal == 48) return 39;
    if (currentVal == 65) return 57;
    if (currentVal == 97) return 90;
    return currentVal - 1;
  }
}

void MenuSystem::idleScreenTextMenu() 
{
  uint8_t cursorLine = 0;
  uint8_t cursorPose = 0;
  char charCurrentByte;

  uint32_t lastFastChangeMillis = 0;
  uint32_t FAST_CHANGE_INTERVAL = 150;

  lcd.clear();
  lcd.setCursor(0, 0);

  for (uint8_t i = 0; i < 16; i++) 
  {
    char c = memory.displayText.idleTextUp[i];
    if (c < 32 || c > 122) c = ' ';
    memory.displayText.idleTextUp[i] = c;
    lcd.print(c);
  }

  lcd.setCursor(0, 1);

  for (uint8_t i = 0; i < 16; i++) 
  {
    char c = memory.displayText.idleTextBottom[i];
    if (c < 32 || c > 122) c = ' ';
    memory.displayText.idleTextBottom[i] = c;
    lcd.print(c);
  }

  lcd.cursor();

  while(true)
  {
    inputManager.update();

    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);

    lcd.setCursor(cursorPose, cursorLine);
    if (cursorLine == 0) charCurrentByte = memory.displayText.idleTextUp[cursorPose];
    else                 charCurrentByte = memory.displayText.idleTextBottom[cursorPose];

    uint32_t currentMillis = millis();

    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP))
      {
        timeOut = 0;
        charCurrentByte = getNextValidAscii(charCurrentByte, true);

        if (cursorLine == 0) memory.displayText.idleTextUp[cursorPose] = charCurrentByte;
        else                 memory.displayText.idleTextBottom[cursorPose] = charCurrentByte;

        lcd.print((char)charCurrentByte);
        lcd.setCursor(cursorPose, cursorLine);
        lastFastChangeMillis = currentMillis;
      }
      else if (inputManager.isPressed(BTN_DOWN))
      {
        timeOut = 0;
        charCurrentByte = getNextValidAscii(charCurrentByte, false);

        if (cursorLine == 0) memory.displayText.idleTextUp[cursorPose] = charCurrentByte;
        else                 memory.displayText.idleTextBottom[cursorPose] = charCurrentByte;

        lcd.print((char)charCurrentByte);
        lcd.setCursor(cursorPose, cursorLine);
        lastFastChangeMillis = currentMillis;
      }
    }

    if (eventRight == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      cursorPose++;
      if (cursorPose > 15) 
      {
        cursorPose = 0;
        cursorLine++;
        if (cursorLine > 1) cursorLine = 0;
      }
      lcd.setCursor(cursorPose, cursorLine);
    }

    if (eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > 6000) break;
  }

  lcd.noCursor();
  lcd.noBlink();

  memory.displayText.idleTextUp[16] = '\0';
  memory.displayText.idleTextBottom[16] = '\0';

  memory.writeData(TMemoryManager::PartitionID::DISPLAY_TEXT, memory.displayText);

  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);

  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::brightnessMenuWrite(uint8_t ledBrightness) 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F("Mode "));
  lcd.print(memory.settings.selectedBrightnessIndex + 1);
  lcd.print(F(" Brightness"));
  lcd.setCursor(0, 1);

  lcd.print(ledBrightness);
  lcd.print(F(" (0-255)"));
}

void MenuSystem::brightnessMenu() 
{
  if (memory.settings.selectedBrightnessIndex == 0) 
  {
    lcd.clear();
    lcd.write(GEAR_CHAR);
    lcd.setCursor(4, 0);
    lcd.print(F("Mode 1"));
    lcd.setCursor(3, 1);
    lcd.print(F("No Control"));
    delay(1500);
    settingsMenuWrite();
    return;
  }
      
  uint8_t index = memory.settings.selectedBrightnessIndex - 1;

  uint32_t lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  uint8_t &ledBrightness = memory.settings.brightnessModeValue[index];

  brightnessMenuWrite(ledBrightness);
  while(true) 
  {
    inputManager.update();
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    uint32_t currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP))
      {
        ledBrightness += 5;
        timeOut = 0;
        ledBrightness = constrain(ledBrightness, 0, 255);
        LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B, ledBrightness);
        brightnessMenuWrite(ledBrightness);
        lastFastChangeMillis = currentMillis;
      }

      if (inputManager.isPressed(BTN_DOWN))
      {
        ledBrightness -= 5;
        timeOut = 0;
        ledBrightness = constrain(ledBrightness, 0, 255);
        LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B, ledBrightness);
        brightnessMenuWrite(ledBrightness);
        lastFastChangeMillis = currentMillis;
      }
    }

    if (eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }

  memory.writeData(PartID::SETTINGS, memory.settings);
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::screenBrightnessMenuWrite() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F(" Backlight"));
  lcd.setCursor(4, 1);
  lcd.print(lcdBacklightPerc);
  lcd.print(F("%"));
}

void MenuSystem::screenBrightnessMenu() 
{
  uint32_t lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  lcdBacklightPerc = map(memory.settings.lcdBacklight, 0, 255, 0, 100);
  screenBrightnessMenuWrite();
  while(true) 
  {
    inputManager.update();
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    uint32_t currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP))
      {
        lcdBacklightPerc += 5;
        timeOut = 0;
        lcdBacklightPerc = constrain(lcdBacklightPerc, 0, 100);
        memory.settings.lcdBacklight = map(lcdBacklightPerc, 0, 100, 0, 255);
        analogWrite(Pins::LCD_BACKLIGHT, memory.settings.lcdBacklight);
        screenBrightnessMenuWrite();
        lastFastChangeMillis = currentMillis;
      }
      if (inputManager.isPressed(BTN_DOWN))
      {
        lcdBacklightPerc -= 5;
        timeOut = 0;
        lcdBacklightPerc = constrain(lcdBacklightPerc, 0, 100);
        memory.settings.lcdBacklight = map(lcdBacklightPerc, 0, 100, 0, 255);
        analogWrite(Pins::LCD_BACKLIGHT, memory.settings.lcdBacklight);
        screenBrightnessMenuWrite();
        lastFastChangeMillis = currentMillis;
      }
    }

    if (eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  memory.writeData(PartID::SETTINGS, memory.settings);
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::screenOffStateMenuWrite() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F(" Dim/Turn Off"));
  lcd.setCursor(5, 1);

  if (memory.settings.screenOffState)   lcd.print(F("Turn Off"));
  else                                  lcd.print(F("Dim"));
}

void MenuSystem::screenOffStateMenu() 
{
  screenOffStateMenuWrite();
  while(true)
  {
    inputManager.update();
    ButtonEvent eventUp    = inputManager.getEvent(BTN_UP);
    ButtonEvent eventDown  = inputManager.getEvent(BTN_DOWN);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    if (eventUp == BTN_EVENT_CLICK || eventDown == BTN_EVENT_CLICK || eventRight == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      memory.settings.screenOffState = !memory.settings.screenOffState;
      screenOffStateMenuWrite(); 
      delay(100); 
    }
    
    if(eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }
    
    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  
  memory.writeData(PartID::SETTINGS, memory.settings);
  
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::LDRManagementMenuText(bool focusOnLDRActive) 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F(" Limit"));
  lcd.setCursor(2, 1);
  if(!focusOnLDRActive)  lcd.print(F(">"));
  else                  lcd.print(F(" "));
  lcd.print(memory.settings.LDRLimit);
  lcd.print(F("%"));

  lcd.setCursor(10, 0);
  lcd.print("Enable");
  lcd.setCursor(11, 1);
  if(focusOnLDRActive)  lcd.print(F(">"));
  else                   lcd.print(F(" "));
  if (memory.settings.LDRActive) lcd.print(" ON");
  else                           lcd.print("OFF");
  
}

void MenuSystem::LDRManagementMenu() 
{
  uint32_t lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  bool focusOnLDRActive = false;

  LDRManagementMenuText(focusOnLDRActive);
  while(true) 
  {
    inputManager.update();
    
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventUp    = inputManager.getEvent(BTN_UP);
    ButtonEvent eventDown  = inputManager.getEvent(BTN_DOWN);

    bool isUpPressed   = inputManager.isPressed(BTN_UP);
    bool isDownPressed = inputManager.isPressed(BTN_DOWN);

    uint32_t currentMillis = millis();
    if (!focusOnLDRActive)
    {
      if (isUpPressed || isDownPressed)
      {
        if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
        {
          timeOut = 0;
          
          if (isUpPressed)
          {
            if (memory.settings.LDRLimit <= 95) memory.settings.LDRLimit += 5;
            else memory.settings.LDRLimit = 100;
          }
          
          if (isDownPressed)
          {
            if (memory.settings.LDRLimit >= 5) memory.settings.LDRLimit -= 5;
            else memory.settings.LDRLimit = 0;
          }

          LDRManagementMenuText(focusOnLDRActive);
          lastFastChangeMillis = currentMillis;
        }
      }
    }
    else if (eventUp == BTN_EVENT_CLICK || eventDown == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      memory.settings.LDRActive = !memory.settings.LDRActive;
      LDRManagementMenuText(focusOnLDRActive);
    }

    if (eventRight == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      focusOnLDRActive = !focusOnLDRActive;
      LDRManagementMenuText(focusOnLDRActive);
    }

    if (eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }
      
    delay(10);
    timeOut++;
    if (timeOut > 1500) break;
  }

  memory.writeData(PartID::SETTINGS, memory.settings);
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::timerSettingsMenuWrite(bool focusOnStart) 
{
  lcd.clear();
  if (focusOnStart) lcd.print(F(">Start: "));
  else              lcd.print(F(" Start: "));
  
  uint16_t startH = memory.settings.startTime / 60;
  uint16_t startM = memory.settings.startTime % 60;

  lcd.setCursor(8, 0);
  if (startH < 10) lcd.print(F("0"));
  lcd.print(startH);
  lcd.print(F(":"));
  if (startM < 10) lcd.print(F("0"));
  lcd.print(startM);

  lcd.setCursor(0, 1);
  if (!focusOnStart) lcd.print(F(">End:   "));
  else               lcd.print(F(" End:   "));
  
  uint16_t endH = memory.settings.endTime / 60;
  uint16_t endM = memory.settings.endTime % 60;

  lcd.setCursor(8, 1);
  if (endH < 10) lcd.print(F("0"));
  lcd.print(endH);
  lcd.print(F(":"));
  if (endM < 10) lcd.print(F("0"));
  lcd.print(endM);
  
  lcd.setCursor(15, 1);
  lcd.write(ARROW_RIGHT_CHAR);
}

void MenuSystem::timerSettingsMenu() 
{  
  bool focusOnStart = true;
  uint32_t lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;
  timerSettingsMenuWrite(focusOnStart);

  while(true)
  {
    uint16_t &targetTime = focusOnStart ? memory.settings.startTime : memory.settings.endTime;

    inputManager.update();
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    uint32_t currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP)) 
      {
        timeOut = 0;
        targetTime += 15;
        if (targetTime >= 1440) targetTime = 0;
        timerSettingsMenuWrite(focusOnStart);
        lastFastChangeMillis = currentMillis;
      }

      if (inputManager.isPressed(BTN_DOWN)) 
      {
        timeOut = 0;
        targetTime -= 15;
        if (targetTime < 0) targetTime = 1425;
        timerSettingsMenuWrite(focusOnStart);
        lastFastChangeMillis = currentMillis;
      }
    }

    if (eventRight == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      focusOnStart = !focusOnStart;
      timerSettingsMenuWrite(focusOnStart);
    }

    if (eventLeft == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  memory.writeData(PartID::SETTINGS, memory.settings);
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::setClockMenuWrite(bool focusOnActivation) 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F(" Clock"));
  lcd.setCursor(1, 1);

  if(!focusOnActivation) lcd.print(">");
  else                  lcd.setCursor(2, 1);
  
  int hours = currentTime / 60;
  int minutes = currentTime % 60;
  
  if (hours < 10) lcd.print(F("0"));
  lcd.print(hours);
  lcd.print(F(":"));
  if (minutes < 10) lcd.print(F("0"));
  lcd.print(minutes);

  lcd.setCursor(10, 0);
  lcd.print(F("Enable"));

  lcd.setCursor(11, 1);
  if(focusOnActivation) lcd.print(">");
  else                  lcd.setCursor(12, 1);

  if(memory.settings.timerActive) lcd.print(F("ON"));
  else                            lcd.print(F("OFF"));
}

void MenuSystem::setClockMenu(bool coldBoot)
{
  uint32_t lastFastChangeMillis = 0;
  
  bool focusOnClockEnable = false;
  
  uint16_t dynamicInterval = 150;
  uint8_t holdCounter = 0;

  setClockMenuWrite(focusOnClockEnable);
  uint16_t maxTimeout = coldBoot ? 6000 : 1000;
  timeOut = 0;

  while(true)
  {
    inputManager.update();

    ButtonEvent eventLeft   = inputManager.getEvent(BTN_LEFT);
    ButtonEvent eventRight  = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventUp     = inputManager.getEvent(BTN_UP);
    ButtonEvent eventDown   = inputManager.getEvent(BTN_DOWN);

    uint32_t currentMillis = millis();

    bool isUpPressed   = inputManager.isPressed(BTN_UP);
    bool isDownPressed = inputManager.isPressed(BTN_DOWN);

    if (!focusOnClockEnable)
    {
      if (isUpPressed || isDownPressed)
      {
        if (currentMillis - lastFastChangeMillis >= dynamicInterval)
        {
          timeOut = 0;
          holdCounter++;

          if (holdCounter > 25)      dynamicInterval = 30;
          else if (holdCounter > 10) dynamicInterval = 75;
          else                       dynamicInterval = 150;

          if (isUpPressed)
          {
            currentTime += 1;
            if (currentTime >= 1440) currentTime = 0;
          }
          else if (isDownPressed)
          {
            currentTime -= 1;
            if (currentTime < 0) currentTime = 1439;
          }

          setClockMenuWrite(focusOnClockEnable);
          lastFastChangeMillis = currentMillis;
        }
      }
      else
      {
        holdCounter = 0;
        dynamicInterval = 150;
      }
    }
    else if (eventUp == BTN_EVENT_CLICK || eventDown == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      memory.settings.timerActive = !memory.settings.timerActive;
      setClockMenuWrite(focusOnClockEnable);
    }

    if (eventRight == BTN_EVENT_CLICK)
    {
      timeOut = 0;
      focusOnClockEnable = !focusOnClockEnable;
      setClockMenuWrite(focusOnClockEnable);
    }

    if (eventLeft == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      clockIsSet = true;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > maxTimeout) break;
  }
  
  if (!coldBootClockSet && timeOut < maxTimeout) coldBootClockSet = true;
  
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  
  inputManager.update();
  if (!coldBoot) settingsMenuWrite();
}

void MenuSystem::createNewRGBConfigMenu() 
{
  if (memory.settings.totalConfig < 20) 
  {
    memory.createNewRGBConfig();
    LEDController.RGBColorApply
    (
      memory.activeConfig.R, 
      memory.activeConfig.G, 
      memory.activeConfig.B, 
      memory.settings.brightnessModeValue[memory.settings.selectedBrightnessIndex]
    );

    lcd.clear();
    lcd.write(GEAR_CHAR);
    lcd.print(F(" Config Created!"));
    lcd.setCursor(15, 1);
    lcd.write(TICK_CHAR);
    delay(1500);
  } 
  else 
  {
    lcd.clear();
    lcd.print(F(" Memory Full!"));
    lcd.setCursor(0, 1);
    lcd.print(F(" Max: 20"));
    delay(2000);
  }
  settingsMenuWrite();
}

void MenuSystem::deleteRGBConfigMenu() 
{
  if (memory.settings.totalConfig > 1) 
  {
    memory.deleteCurrentRGBConfig();
    LEDController.RGBColorApply
    (
      memory.activeConfig.R, 
      memory.activeConfig.G, 
      memory.activeConfig.B, 
      memory.settings.brightnessModeValue[memory.settings.selectedBrightnessIndex]
    );

    lcd.clear();
    lcd.write(GEAR_CHAR);
    lcd.print(F(" Config Deleted!"));
    lcd.setCursor(15, 1);
    lcd.write(TICK_CHAR);
    delay(1500);
  } 
  else 
  {
    lcd.clear();
    lcd.print(F(" Cannot Delete!"));
    lcd.setCursor(0, 1);
    lcd.print(F(" Min 1 Required"));
    delay(2000);
  }
  settingsMenuWrite();
}

void MenuSystem::infoScreen()
{
  lcd.clear();
  lcd.print(F("V")); 
  lcd.print(memory.eepromVersion.major); 
  lcd.print(F(".")); 
  lcd.print(memory.eepromVersion.minor);
  lcd.print(F(".")); 
  lcd.print(memory.eepromVersion.patch);
  lcd.setCursor(0, 1);
  lcd.print(memory.eepromVersion.phase);
  lcd.print(F(".")); 
  lcd.print(memory.eepromVersion.iteration);
  lcd.setCursor(9, 0);
  lcd.print(F("Uptime:")); 
  while(true) 
  {
    inputManager.update();

    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    uint32_t flagSeconds = (uint32_t)uptimeFlag * 4294967UL;
    uint32_t totalSeconds = millis() / 1000 + flagSeconds;
    uint32_t hours = totalSeconds / 3600;
    uint8_t minutes = (totalSeconds / 60) % 60;
    uint8_t seconds = totalSeconds % 60;
    uint8_t xPos = 8;
    if(hours >= 100) xPos = 7;
    if (hours >= 1000) xPos = 6;
    lcd.setCursor(xPos, 1);
    if(hours < 10) lcd.print(F("0"));
    lcd.print(hours);
    lcd.print(F(":"));
    if(minutes < 10) lcd.print(F("0"));
    lcd.print(minutes);
    lcd.print(F(":"));
    if(seconds < 10) lcd.print(F("0"));
    lcd.print(seconds);

    if(eventLeft == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      break;
    }

    delay(100);
  }
  inputManager.update();
  settingsMenuWrite(); 
}

void MenuSystem::settingsMenuWrite() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);

  display.printMenuLine(settingsMenuCursor - 1);

  if(!(settingsMenuCursor == 1))
  {
    lcd.setCursor(0, 1);
    lcd.write(ARROW_LEFT_CHAR);
  }
  if(!(settingsMenuCursor == 12))
  {
    lcd.setCursor(15, 1);
    lcd.write(ARROW_RIGHT_CHAR);
  }
  lcd.setCursor(2, 1);
  for (int i = 0; i < 12; i++) lcd.print(F("-"));
  lcd.blink();
  lcd.setCursor(settingsMenuCursor + 1, 1);
}

void MenuSystem::settingsMenu() 
{
  lcd.clear();
  settingsMenuWrite();
  while (true) 
  {
    inputManager.update();
    ButtonEvent eventUp    = inputManager.getEvent(BTN_UP);
    ButtonEvent eventDown  = inputManager.getEvent(BTN_DOWN);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    if (eventUp == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      settingsMenuCursor--;
      settingsMenuCursor = constrain(settingsMenuCursor, 1, 12);
      settingsMenuWrite();
    }
    if (eventDown == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      settingsMenuCursor++;
      settingsMenuCursor = constrain(settingsMenuCursor, 1, 12);
      settingsMenuWrite();
    }
    if (eventRight == BTN_EVENT_CLICK) 
    {
      lcd.noBlink();
      timeOut = 0;
      switch (settingsMenuCursor) 
      {
        case 1:  RGBConfigMenu();           break;
        case 2:  setClockMenu(false);       break;
        case 3:  idleScreenTextMenu();      break;
        case 4:  brightnessMenu();          break;
        case 5:  LDRManagementMenu();       break;
        case 6:  screenBrightnessMenu();    break;
        case 7:  screenOffStateMenu();      break;
        case 8:  timerSettingsMenu();       break;
        case 9:  createNewRGBConfigMenu();  break;
        case 10: deleteRGBConfigMenu();     break;
        case 11: infoScreen();              break;
        case 12: memory.factoryReset();     break;
      }
    }

    if (eventLeft == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
      break;
    }

    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  lcd.clear();
  lcd.noBlink();
  inputManager.update();
  infoDisplayWrite();
}