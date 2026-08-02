#include "Arduino.h"
#include "Menu.h"
#include "Display.h"
#include "TunixMemoryManager.h"
#include "LEDControl.h"
#include "InputManager.h"

unsigned long MenuSystem::uptime = 0;
unsigned long MenuSystem::oldUptime = 0;
unsigned long MenuSystem::uptimeFlag = 0;

MenuSystem::MenuSystem() { }

bool MenuSystem::updateClockAndControl() 
{
  unsigned long currentMillis = millis();
  constexpr unsigned long CALIBRATED_MINUTE_MS = 59944UL; 
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
  lcd.print(memory.settings.selectedBrightness);
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
      memory.settings.selectedBrightness++;
      if (memory.settings.selectedBrightness > 4) memory.settings.selectedBrightness = 1;
      LEDController.RGBBrigthnessRead();
      LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
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
    
    checkLDR();

    timeOut++;
    delay(2);
    if (timeOut > 1500) idleScreen();
  }
}

void MenuSystem::idleTextPrint()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(memory.settings.idleTextUp);
  lcd.setCursor(0, 1);
  lcd.print(memory.settings.idleTextBottom);
}

void MenuSystem::idleScreen() 
{
  timeOut = 0;
  unsigned long lastClockBlinkMillis = 0;
  bool clockBlinkState = false;
  memory.saveBasicMemory();
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
      memory.settings.selectedBrightness++;
      if (memory.settings.selectedBrightness > 4) memory.settings.selectedBrightness = 1;
      memory.saveBasicMemory();
      LEDController.RGBBrigthnessRead();
      LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
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
    
    if (checkLDR()) idleTextPrint();

    if (updateClockAndControl()) idleTextPrint();

    uptime = millis();
    if (uptime < oldUptime) uptimeFlag++;

    unsigned long currentMillis = millis();
    if (currentMillis - lastClockBlinkMillis >= 750 && !coldBootClockSet) 
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
  unsigned long lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  lcd.clear();
  lcd.cursor();
  lcd.blink();
  RGBConfigMenuWrite();
  while (true) 
  {
    inputManager.update();
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    unsigned long currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
        if (inputManager.isPressed(BTN_UP))
        {
          timeOut = 0;
          if (RGBConfigMenuCursor == 1) memory.activeConfig.R = constrain((int)memory.activeConfig.R + 5, 0, 255);
          if (RGBConfigMenuCursor == 2) memory.activeConfig.G = constrain((int)memory.activeConfig.G + 5, 0, 255);
          if (RGBConfigMenuCursor == 3) memory.activeConfig.B = constrain((int)memory.activeConfig.B + 5, 0, 255);
          byte redValue = memory.activeConfig.R;
          byte greenValue = memory.activeConfig.G;
          byte blueValue = memory.activeConfig.B;
          LEDController.RGBColorApply(redValue, greenValue, blueValue);
          RGBConfigMenuWrite();
          lastFastChangeMillis = currentMillis;
        }

        if (inputManager.isPressed(BTN_DOWN))
        {
          timeOut = 0;
          if (RGBConfigMenuCursor == 1) memory.activeConfig.R = constrain((int)memory.activeConfig.R - 5, 0, 255);
          if (RGBConfigMenuCursor == 2) memory.activeConfig.G = constrain((int)memory.activeConfig.G - 5, 0, 255);
          if (RGBConfigMenuCursor == 3) memory.activeConfig.B = constrain((int)memory.activeConfig.B - 5, 0, 255);
          byte redValue = memory.activeConfig.R;
          byte greenValue = memory.activeConfig.G;
          byte blueValue = memory.activeConfig.B;
          LEDController.RGBColorApply(redValue, greenValue, blueValue);
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

byte MenuSystem::getNextValidAscii(byte currentVal, bool increment)
{
  if (increment) 
  {
    currentVal++;
    if (currentVal < 32) return 32;
    if (currentVal > 32 && currentVal < 48) return 48;
    if (currentVal > 57 && currentVal < 65) return 65;
    if (currentVal > 90 && currentVal < 97) return 97;
    if (currentVal > 122) return 32;
  } 
  else 
  {
    currentVal--;
    if (currentVal < 32 || currentVal > 122)  return 122;
    if (currentVal > 32 && currentVal < 48)   return 32;
    if (currentVal > 57 && currentVal < 65)   return 57;
    if (currentVal > 90 && currentVal < 97)   return 90;
  }
  
  return currentVal;
}

void MenuSystem::idleScreenTextMenu() 
{
  byte cursorLine = 0;
  byte cursorPose = 0;
  char charCurrentByte;

  unsigned long lastFastChangeMillis = 0;
  unsigned long FAST_CHANGE_INTERVAL = 150;

  lcd.clear();
  lcd.print(memory.settings.idleTextUp);
  lcd.setCursor(0, 1);
  lcd.print(memory.settings.idleTextBottom);
  lcd.cursor();
  lcd.blink();
  while(true)
  {
    inputManager.update();

    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);

    lcd.setCursor(cursorPose, cursorLine);
    if (cursorLine == 0) charCurrentByte = memory.settings.idleTextUp[cursorPose];
    else charCurrentByte = memory.settings.idleTextBottom[cursorPose];

    unsigned long currentMillis = millis();

    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP))
      {
        timeOut = 0;

        charCurrentByte = getNextValidAscii(charCurrentByte, true);
        lcd.print((char)charCurrentByte);

        if (cursorLine == 0) memory.settings.idleTextUp[cursorPose] = charCurrentByte;
        else memory.settings.idleTextBottom[cursorPose] = charCurrentByte;
        
        lastFastChangeMillis = currentMillis;
      }

      if (inputManager.isPressed(BTN_DOWN))
      {
        timeOut = 0;

        charCurrentByte = getNextValidAscii(charCurrentByte, false);
        lcd.print((char)charCurrentByte);

        if (cursorLine == 0) memory.settings.idleTextUp[cursorPose] = charCurrentByte;
        else memory.settings.idleTextBottom[cursorPose] = charCurrentByte;

        lastFastChangeMillis = currentMillis;
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
      }
    }

    if(eventLeft == BTN_EVENT_CLICK)
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
  memory.saveBasicMemory();
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::brightnessMenuWrite() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F("Mode "));
  lcd.print(memory.settings.selectedBrightness);
  lcd.print(F(" Brightness"));
  lcd.setCursor(0, 1);
  byte ledBrightness = LEDController.getLEDBrightness();
  lcd.print(ledBrightness);
  lcd.print(F(" (0-255)"));
}

void MenuSystem::brightnessMenu() 
{
  if (memory.settings.selectedBrightness == 1) 
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

  unsigned long lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  brightnessMenuWrite();
  byte ledBrightness = LEDController.getLEDBrightness();
  while(true) 
  {
    inputManager.update();
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    unsigned long currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP))
      {
        ledBrightness += 5;
        timeOut = 0;
        ledBrightness = constrain(ledBrightness, 0, 255);
        LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
        brightnessMenuWrite();
        lastFastChangeMillis = currentMillis;
      }

      if (inputManager.isPressed(BTN_DOWN))
      {
        ledBrightness -= 5;
        timeOut = 0;
        ledBrightness = constrain(ledBrightness, 0, 255);
        LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
        brightnessMenuWrite();
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
  memory.saveBrightnessForMode(memory.settings.selectedBrightness, ledBrightness);
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
  unsigned long lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;

  lcdBacklightPerc = map(memory.settings.lcdBacklight, 0, 255, 0, 100);
  screenBrightnessMenuWrite();
  while(true) 
  {
    inputManager.update();
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    unsigned long currentMillis = millis();
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
  memory.saveBasicMemory();
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
  
  memory.saveBasicMemory();
  
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::LDRManagementMenuText() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F(" LDR Limit"));
  lcd.setCursor(5, 1);
  lcd.print(LDRLimitPerc);
  lcd.print(F("%"));
  LDRValue = map(analogRead(Pins::LDR), 0, 1023, 0, 100);
  lcd.setCursor(13, 0);
  lcd.print(LDRValue);
}

void MenuSystem::LDRManagementMenu() 
{
  unsigned long lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;
  LDRLimitPerc = map(memory.settings.LDRLimit, 0, 1023, 0, 100);
  LDRManagementMenuText();
  while(true) 
  {
    inputManager.update();
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    unsigned long currentMillis = millis();
    if (currentMillis - lastFastChangeMillis >= FAST_CHANGE_INTERVAL)
    {
      if (inputManager.isPressed(BTN_UP))
      {
        LDRLimitPerc += 5;
        timeOut = 0;
        LDRLimitPerc = constrain(LDRLimitPerc, 0, 100);
        memory.settings.LDRLimit = map(LDRLimitPerc, 0, 100, 0, 1023);
        LDRManagementMenuText();
        lastFastChangeMillis = currentMillis;
      }
      
      if (inputManager.isPressed(BTN_DOWN))
      {
        LDRLimitPerc -= 5;
        timeOut = 0;
        LDRLimitPerc = constrain(LDRLimitPerc, 0, 100);
        memory.settings.LDRLimit = map(LDRLimitPerc, 0, 100, 0, 1023);
        LDRManagementMenuText();
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
    if (timeOut > 1500) break;
  }

  memory.saveBasicMemory();
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
  
  int startH = memory.settings.startTime / 60;
  int startM = memory.settings.startTime % 60;

  lcd.setCursor(8, 0);
  if (startH < 10) lcd.print(F("0"));
  lcd.print(startH);
  lcd.print(F(":"));
  if (startM < 10) lcd.print(F("0"));
  lcd.print(startM);

  lcd.setCursor(0, 1);
  if (!focusOnStart) lcd.print(F(">End:   "));
  else lcd.print(F(" End:   "));
  
  int endH = memory.settings.endTime / 60;
  int endM = memory.settings.endTime % 60;

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
  unsigned long lastFastChangeMillis = 0, FAST_CHANGE_INTERVAL = 150;
  timerSettingsMenuWrite(focusOnStart);

  while(true)
  {
    int &targetTime = focusOnStart ? memory.settings.startTime : memory.settings.endTime;

    inputManager.update();
    ButtonEvent eventRight = inputManager.getEvent(BTN_RIGHT);
    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    unsigned long currentMillis = millis();
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
  memory.saveBasicMemory();
  lcd.setCursor(15, 1);
  lcd.write(TICK_CHAR);
  delay(1000);
  inputManager.update();
  settingsMenuWrite();
}

void MenuSystem::setClockMenuWrite() 
{
  lcd.clear();
  lcd.write(GEAR_CHAR);
  lcd.print(F(" Adjust Clock"));
  lcd.setCursor(5, 1);
  
  int hours = currentTime / 60;
  int minutes = currentTime % 60;
  
  if (hours < 10) lcd.print(F("0"));
  lcd.print(hours);
  lcd.print(F(":"));
  if (minutes < 10) lcd.print(F("0"));
  lcd.print(minutes);
  
  lcd.setCursor(15, 1);
  lcd.write(ARROW_RIGHT_CHAR);
}

void MenuSystem::setClockMenu(bool coldBoot)
{
  unsigned long lastFastChangeMillis = 0;
  
  uint16_t dynamicInterval = 150;
  uint8_t holdCounter = 0;

  setClockMenuWrite();
  int16_t maxTimeout = coldBoot ? 6000 : 1000;
  timeOut = 0;

  while(true)
  {
    inputManager.update();
    ButtonEvent eventLeft = inputManager.getEvent(BTN_LEFT);

    unsigned long currentMillis = millis();

    bool isUpPressed   = inputManager.isPressed(BTN_UP);
    bool isDownPressed = inputManager.isPressed(BTN_DOWN);

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

        setClockMenuWrite();
        lastFastChangeMillis = currentMillis;
      }
    }
    else
    {
      holdCounter = 0;
      dynamicInterval = 150;
    }

    if (eventLeft == BTN_EVENT_CLICK) 
    {
      timeOut = 0;
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
    LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);

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
    LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
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
  lcd.setCursor(9, 0);
  lcd.print(F("Uptime:")); 
  while(true) 
  {
    inputManager.update();

    ButtonEvent eventLeft  = inputManager.getEvent(BTN_LEFT);

    unsigned long flagSeconds = (unsigned long)uptimeFlag * 4294967UL;
    unsigned long totalSeconds = millis() / 1000 + flagSeconds;
    unsigned long hours = totalSeconds / 3600;
    byte minutes = (totalSeconds / 60) % 60;
    byte seconds = totalSeconds % 60;
    byte xPos = 8;
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