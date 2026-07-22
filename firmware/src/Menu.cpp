#include "Arduino.h"
#include "Menu.h"
#include "Display.h"
#include "TunixMemoryManager.h"
#include "LEDControl.h"

unsigned long MenuSystem::uptime = 0;
unsigned long MenuSystem::oldUptime = 0;
unsigned long MenuSystem::uptimeFlag = 0;

MenuSystem::MenuSystem() { }

bool MenuSystem::updateClockAndControl() 
{
  unsigned long currentMillis = millis();

  if (currentMillis - lastMinuteMillis >= 60000UL) 
  {
    lastMinuteMillis = currentMillis;
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
    lcd.write(6);
  }
}

void MenuSystem::infoDisplay() 
{
  while (digitalRead(BUTTON_LEFT) == LOW);
  infoDisplayWrite();
  timeOut = 0;
  while (1) 
  {
    delay(5);
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      while (digitalRead(BUTTON_UP) == LOW);
      timeOut = 0;
      if (memory.settings.selectedConfig == 0) memory.settings.selectedConfig = memory.settings.totalConfig - 1;
      else memory.settings.selectedConfig--;

      memory.loadRGBConfig(memory.settings.selectedConfig);
      infoDisplayWrite();
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      while (digitalRead(BUTTON_DOWN) == LOW);
      timeOut = 0;
      memory.settings.selectedConfig++;
      if (memory.settings.selectedConfig >= memory.settings.totalConfig) memory.settings.selectedConfig = 0;

      memory.loadRGBConfig(memory.settings.selectedConfig);
      infoDisplayWrite();
    }

    if (digitalRead(BUTTON_LEFT) == LOW) 
    {
      timeOut = 0;
      short longPressCounter = 0;
      bool longPress = false;
      delay(50);
      while (digitalRead(BUTTON_LEFT) == LOW) 
      {
        longPressCounter++;
        if (longPressCounter >= 100) 
        {
          longPress = true;
          LEDController.ledChange();
          while (digitalRead(BUTTON_LEFT) == LOW);
          lcd.clear();
          infoDisplayWrite();
          break;
        }
        delay(5);
      }
      if (longPress == false) 
      {
        memory.settings.selectedBrightness++;
        memory.settings.selectedBrightness = constrain(memory.settings.selectedBrightness, 1, 5);
        if (memory.settings.selectedBrightness == 5) memory.settings.selectedBrightness = 1;
        memory.saveBasicMemory();
        LEDController.RGBBrigthnessRead();
        LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
        infoDisplayWrite();
      }
    }
    if (digitalRead(BUTTON_RIGHT) == LOW) 
    {
      while (digitalRead(BUTTON_RIGHT) == LOW);
      timeOut = 0;
      settingsMenu();
    }
    LEDController.fadeAnimationEngine();
    LDRValue = analogRead(LDR_PIN);
    bool LEDState = LEDController.getLEDState();
    if ((LDRValue > memory.settings.LDRLimit) && (LEDState == true) && (LDRActivated == false) && (LDRDisabled == false)) 
    {
      LEDController.ledChange();
      LDRActivated = true;
      LDRDisabled = true;
      infoDisplayWrite();
    }
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
  short longPressCounter = 0;
  idleTextPrint();
  while (true) 
  {
    if (digitalRead(BUTTON_UP) == LOW || digitalRead(BUTTON_DOWN) == LOW || digitalRead(BUTTON_RIGHT) == LOW) break;
    if (digitalRead(BUTTON_LEFT) == LOW) 
    {
      longPressCounter = 0;
      while (digitalRead(BUTTON_LEFT) == LOW) 
      {
        longPressCounter++;
        if (longPressCounter >= 100) 
        {
          LEDController.ledChange();
          lcd.print(memory.settings.idleTextUp);
          lcd.setCursor(0, 1);
          lcd.print(memory.settings.idleTextBottom);
          while (digitalRead(BUTTON_LEFT) == LOW);
          longPressCounter = 0;
          timeOut = 0;
        }
        delay(10);
      }
      if (longPressCounter < 100 && longPressCounter > 0) 
      {
        memory.settings.selectedBrightness++;
        memory.settings.selectedBrightness = constrain(memory.settings.selectedBrightness, 1, 5);
        if (memory.settings.selectedBrightness == 5) memory.settings.selectedBrightness = 1;
        memory.saveBasicMemory();
        LEDController.RGBBrigthnessRead();
        LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
        timeOut = 0;
      }
    }
    if ((timeOut >= 1500) && (memory.settings.screenOffState == true)) 
    {
      analogWrite(LCD_BACKLIGHT_PIN, 0);
      lcd.noDisplay();
    }
    if ((timeOut >= 1500) && (memory.settings.screenOffState == false)) analogWrite(LCD_BACKLIGHT_PIN, 20);
    LDRValue = analogRead(LDR_PIN);
    bool LEDState = LEDController.getLEDState();
    if ((LDRValue > memory.settings.LDRLimit) && (LEDState == true) && (LDRActivated == false)) 
    {
      LEDController.ledChange();
      lcd.print(memory.settings.idleTextUp);
      lcd.setCursor(0, 1);
      lcd.print(memory.settings.idleTextBottom);
      LDRActivated = true;
    }
    oldUptime = uptime;
    delay(10);

    bool LEDChanged = updateClockAndControl();
    if (LEDChanged) idleTextPrint();

    uptime = millis();
    if (uptime < oldUptime) uptimeFlag++;

    unsigned long currentMillis = millis();
    if (currentMillis - lastClockBlinkMillis >= 750 && !coldBootClockSet) 
    {
      lastClockBlinkMillis = currentMillis;
      clockBlinkState = !clockBlinkState;

      lcd.setCursor(15, 0); 
      if (clockBlinkState) lcd.write(4);
      else lcd.print(F(" "));
    }
    timeOut++;
  }
  while (digitalRead(BUTTON_UP) == LOW || digitalRead(BUTTON_DOWN) == LOW || digitalRead(BUTTON_RIGHT) == LOW || digitalRead(BUTTON_LEFT) == LOW);
  lcd.display();
  lcd.clear();
  timeOut = 0;
  analogWrite(LCD_BACKLIGHT_PIN, memory.settings.lcdBacklight);
  infoDisplayWrite();
}

void MenuSystem::RGBConfigMenuWrite() 
{
  lcd.clear();
  lcd.write(7);
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
  lcd.clear();
  lcd.cursor();
  lcd.blink();
  RGBConfigMenuWrite();
  while (digitalRead(BUTTON_LEFT) == HIGH) 
  {
    while (digitalRead(BUTTON_UP) == LOW) 
    {
      timeOut = 0;
      if (RGBConfigMenuCursor == 1) memory.activeConfig.R += 5;
      if (RGBConfigMenuCursor == 2) memory.activeConfig.G += 5;
      if (RGBConfigMenuCursor == 3) memory.activeConfig.B += 5;
      memory.activeConfig.R = constrain(memory.activeConfig.R, 0, 255);
      memory.activeConfig.G = constrain(memory.activeConfig.G, 0, 255);
      memory.activeConfig.B = constrain(memory.activeConfig.B, 0, 255);
      byte redValue = memory.activeConfig.R;
      byte greenValue = memory.activeConfig.G;
      byte blueValue = memory.activeConfig.B;
      LEDController.RGBColorApply(redValue, greenValue, blueValue);
      RGBConfigMenuWrite();
      delay(200);
    }
    while (digitalRead(BUTTON_DOWN) == LOW) 
    {
      timeOut = 0;
      if (RGBConfigMenuCursor == 1) memory.activeConfig.R -= 5;
      if (RGBConfigMenuCursor == 2) memory.activeConfig.G -= 5;
      if (RGBConfigMenuCursor == 3) memory.activeConfig.B -= 5;
      memory.activeConfig.R = constrain(memory.activeConfig.R, 0, 255);
      memory.activeConfig.G = constrain(memory.activeConfig.G, 0, 255);
      memory.activeConfig.B = constrain(memory.activeConfig.B, 0, 255);
      byte redValue = memory.activeConfig.R;
      byte greenValue = memory.activeConfig.G;
      byte blueValue = memory.activeConfig.B;
      LEDController.RGBColorApply(redValue, greenValue, blueValue);
      RGBConfigMenuWrite();
      delay(200);
    }
    if (digitalRead(BUTTON_RIGHT) == LOW) 
    {
      while (digitalRead(BUTTON_RIGHT) == LOW);
      timeOut = 0;
      RGBConfigMenuCursor++;
      if (RGBConfigMenuCursor == 4) RGBConfigMenuCursor = 1;
      RGBConfigMenuWrite();
    }
    delay(10);
    timeOut++;
    if (timeOut > 2000) break;
  }
  lcd.noCursor();
  lcd.noBlink();
  while (digitalRead(BUTTON_LEFT) == LOW);
  memory.saveRGBConfig(memory.settings.selectedConfig);
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::idleScreenTextMenu() 
{
  while (digitalRead(BUTTON_RIGHT) == LOW);
  byte cursorLine = 0;
  byte cursorPose = 0;
  char currentChar;
  lcd.clear();
  lcd.print(memory.settings.idleTextUp);
  lcd.setCursor(0, 1);
  lcd.print(memory.settings.idleTextBottom);
  lcd.cursor();
  lcd.blink();
  while (digitalRead(BUTTON_LEFT) == HIGH) 
  {
    lcd.setCursor(cursorPose, cursorLine);
    if (cursorLine == 0) currentChar = memory.settings.idleTextUp[cursorPose];
    else currentChar = memory.settings.idleTextBottom[cursorPose];
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      timeOut = 0;
      currentChar--;
      if (currentChar < 32) currentChar = 126;
      lcd.write(currentChar);
      if (cursorLine == 0) memory.settings.idleTextUp[cursorPose] = currentChar;
      else memory.settings.idleTextBottom[cursorPose] = currentChar;
      delay(200);
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      timeOut = 0;
      currentChar++;
      if (currentChar > 126) currentChar = 32;
      lcd.write(currentChar);
      if (cursorLine == 0) memory.settings.idleTextUp[cursorPose] = currentChar;
      else memory.settings.idleTextBottom[cursorPose] = currentChar;
      delay(200);
    }
    if (digitalRead(BUTTON_RIGHT) == LOW) 
    {
      while (digitalRead(BUTTON_RIGHT) == LOW);
      timeOut = 0;
      cursorPose++;
      if (cursorPose > 15) 
      {
        cursorPose = 0;
        cursorLine++;
        if (cursorLine > 1) cursorLine = 0;
      }
    }
    delay(10);
    timeOut++;
    if (timeOut > 6000) break;
  }
  while (digitalRead(BUTTON_LEFT) == LOW);
  lcd.noCursor();
  lcd.noBlink();
  memory.saveBasicMemory();
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::brightnessMenuWrite() 
{
  lcd.clear();
  lcd.write(7);
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
  while (digitalRead(BUTTON_RIGHT) == LOW);
  if (memory.settings.selectedBrightness == 1) 
  {
    lcd.clear();
    lcd.write(7);
    lcd.setCursor(4, 0);
    lcd.print(F("Mode 1"));
    lcd.setCursor(3, 1);
    lcd.print(F("No Control"));
    delay(1500);
    while (digitalRead(BUTTON_LEFT) == HIGH) delay(10);
    while (digitalRead(BUTTON_LEFT) == LOW);
    settingsMenuWrite();
    return;
  }
  brightnessMenuWrite();
  byte ledBrightness = LEDController.getLEDBrightness();
  while (digitalRead(BUTTON_LEFT) == HIGH) 
  {
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      ledBrightness += 5;
      timeOut = 0;
      ledBrightness = constrain(ledBrightness, 0, 255);
      LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
      brightnessMenuWrite();
      delay(200);
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      ledBrightness -= 5;
      timeOut = 0;
      ledBrightness = constrain(ledBrightness, 0, 255);
      LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);
      brightnessMenuWrite();
      delay(200);
    }
    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  while(digitalRead(BUTTON_LEFT) == LOW);
  memory.saveBrightnessForMode(memory.settings.selectedBrightness, ledBrightness);
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::screenBrightnessMenuWrite() 
{
  lcd.clear();
  lcd.write(7);
  lcd.print(F(" Backlight"));
  lcd.setCursor(4, 1);
  lcd.print(lcdBacklightPerc);
  lcd.print(F("%"));
}

void MenuSystem::screenBrightnessMenu() 
{
  while (digitalRead(BUTTON_RIGHT) == LOW);
  lcdBacklightPerc = map(memory.settings.lcdBacklight, 0, 255, 0, 100);
  screenBrightnessMenuWrite();
  while (digitalRead(BUTTON_LEFT) == HIGH) 
  {
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      lcdBacklightPerc += 5;
      timeOut = 0;
      lcdBacklightPerc = constrain(lcdBacklightPerc, 0, 100);
      memory.settings.lcdBacklight = map(lcdBacklightPerc, 0, 100, 0, 255);
      analogWrite(LCD_BACKLIGHT_PIN, memory.settings.lcdBacklight);
      screenBrightnessMenuWrite();
      delay(200);
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      lcdBacklightPerc -= 5;
      timeOut = 0;
      lcdBacklightPerc = constrain(lcdBacklightPerc, 0, 100);
      memory.settings.lcdBacklight = map(lcdBacklightPerc, 0, 100, 0, 255);
      analogWrite(LCD_BACKLIGHT_PIN, memory.settings.lcdBacklight);
      screenBrightnessMenuWrite();
      delay(200);
    }
    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  while (digitalRead(BUTTON_LEFT) == LOW);
  memory.saveBasicMemory();
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::screenOffStateMenuWrite() 
{
  lcd.clear();
  lcd.write(7);
  lcd.print(F(" Dim/Turn Off"));
  lcd.setCursor(5, 1);

  if (memory.settings.screenOffState)   lcd.print(F("Turn Off"));
  else                                  lcd.print(F("Dim"));
}

void MenuSystem::screenOffStateMenu() 
{
  while (digitalRead(BUTTON_RIGHT) == LOW);
  screenOffStateMenuWrite();
  while (digitalRead(BUTTON_LEFT) == HIGH)
  {
    if (digitalRead(BUTTON_UP) == LOW || digitalRead(BUTTON_DOWN) == LOW || digitalRead(BUTTON_RIGHT) == LOW) 
    {
      timeOut = 0;
      memory.settings.screenOffState = !memory.settings.screenOffState;
      screenOffStateMenuWrite(); 
      delay(300); 
    }
    
    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  
  while (digitalRead(BUTTON_LEFT) == LOW);
  
  memory.saveBasicMemory();
  
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::LDRManagementMenuText() 
{
  lcd.clear();
  lcd.write(7);
  lcd.print(F(" LDR Limit"));
  lcd.setCursor(5, 1);
  lcd.print(LDRLimitPerc);
  lcd.print(F("%"));
  LDRValue = map(analogRead(LDR_PIN), 0, 1023, 0, 100);
  lcd.setCursor(13, 0);
  lcd.print(LDRValue);
}

void MenuSystem::LDRManagementMenu() 
{
  while (digitalRead(BUTTON_RIGHT) == LOW);
  LDRLimitPerc = map(memory.settings.LDRLimit, 0, 1023, 0, 100);
  LDRManagementMenuText();
  while (digitalRead(BUTTON_LEFT) == HIGH) 
  {
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      LDRLimitPerc += 5;
      timeOut = 0;
      LDRLimitPerc = constrain(LDRLimitPerc, 0, 100);
      memory.settings.LDRLimit = map(LDRLimitPerc, 0, 100, 0, 1023);
      LDRManagementMenuText();
      delay(200);
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      LDRLimitPerc -= 5;
      timeOut = 0;
      LDRLimitPerc = constrain(LDRLimitPerc, 0, 100);
      memory.settings.LDRLimit = map(LDRLimitPerc, 0, 100, 0, 1023);
      LDRManagementMenuText();
      delay(200);
    }
    delay(10);
    timeOut++;
    if (timeOut > 1500) break;
  }
  while (digitalRead(BUTTON_LEFT) == LOW);
  memory.saveBasicMemory();
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::timerSettingsMenuWrite(bool focusOnStart) 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  if (focusOnStart) lcd.print(F(">Start: "));
  else lcd.print(F(" Start: "));
  lcd.setCursor(8, 0);

  int startH = memory.settings.startTime / 60;
  int startM = memory.settings.startTime % 60;
  if (startH < 10) lcd.print(F("0"));
  lcd.print(startH);
  lcd.print(F(":"));
  if (startM < 10) lcd.print(F("0"));
  lcd.print(startM);

  lcd.setCursor(0, 1);
  if (!focusOnStart) lcd.print(F(">End:   "));
  else lcd.print(F(" End:   "));
  lcd.setCursor(8, 1);
  
  int endH = memory.settings.endTime / 60;
  int endM = memory.settings.endTime % 60;
  if (endH < 10) lcd.print(F("0"));
  lcd.print(endH);
  lcd.print(F(":"));
  if (endM < 10) lcd.print(F("0"));
  lcd.print(endM);
  
  lcd.setCursor(15, 1);
  lcd.write(3);
}

void MenuSystem::timerSettingsMenu() 
{
  while (digitalRead(BUTTON_RIGHT) == LOW); 
  
  bool focusOnStart = true;
  timerSettingsMenuWrite(focusOnStart);
  
  while (digitalRead(BUTTON_LEFT) == HIGH)
  {
    bool valueChanged = false;
    int &targetTime = focusOnStart ? memory.settings.startTime : memory.settings.endTime;

    if (digitalRead(BUTTON_UP) == LOW) 
    {
      timeOut = 0;
      targetTime += 15;
      if (targetTime >= 1440) targetTime = 0;
      valueChanged = true;
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      timeOut = 0;
      targetTime -= 15;
      if (targetTime < 0) targetTime = 1425;
      valueChanged = true;
    }
    if (digitalRead(BUTTON_RIGHT) == LOW) 
    {
      while (digitalRead(BUTTON_RIGHT) == LOW);
      timeOut = 0;
      focusOnStart = !focusOnStart;
      timerSettingsMenuWrite(focusOnStart);
    }
    if (valueChanged) 
    {
      timerSettingsMenuWrite(focusOnStart);
      delay(250);
    }
    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  while (digitalRead(BUTTON_LEFT) == LOW);
  memory.saveBasicMemory();
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  settingsMenuWrite();
}

void MenuSystem::setClockMenuWrite() 
{
  lcd.clear();
  lcd.write(7);
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
  lcd.write(3);
}

void MenuSystem::setClockMenu(bool coldBoot) 
{
  if (!coldBoot) while (digitalRead(BUTTON_RIGHT) == LOW);
  setClockMenuWrite();
  int16_t maxTimeout = coldBoot ? 6000 : 1000;
  timeOut = 0;

  while (digitalRead(BUTTON_LEFT) == HIGH)
  {
    bool timeChanged = false;
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      timeOut = 0;
      currentTime += 1;
      if (currentTime >= 1440) currentTime = 0;
      timeChanged = true;
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      timeOut = 0;
      currentTime -= 1;
      if (currentTime < 0) currentTime = 1439;
      timeChanged = true;
    }
    if (timeChanged) 
    {
      setClockMenuWrite();
      delay(150);
    }
    delay(10);
    timeOut++;
    if (timeOut > maxTimeout) break;
  }
  if (!coldBootClockSet && timeOut < maxTimeout) coldBootClockSet = true;
  while (digitalRead(BUTTON_LEFT) == LOW); 
  lcd.setCursor(15, 1);
  lcd.write(5);
  delay(1000);
  if (!coldBoot) settingsMenuWrite();
}

void MenuSystem::createNewRGBConfigMenu() 
{
  if (memory.settings.totalConfig < 20) 
  {
    memory.createNewRGBConfig();
    LEDController.RGBColorApply(memory.activeConfig.R, memory.activeConfig.G, memory.activeConfig.B);

    lcd.clear();
    lcd.write(7);
    lcd.print(F(" Config Created!"));
    lcd.setCursor(15, 1);
    lcd.write(5);
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
    lcd.write(7);
    lcd.print(F(" Config Deleted!"));
    lcd.setCursor(15, 1);
    lcd.write(5);
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
  lcd.print(memory.version.major); 
  lcd.print(F(".")); 
  lcd.print(memory.version.minor);
  lcd.print(F(".")); 
  lcd.print(memory.version.patch);
  lcd.setCursor(9, 0);
  lcd.print(F("Uptime:")); 
  while(digitalRead(BUTTON_LEFT) == HIGH) 
  {
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
    delay(100); 
  }
  while(digitalRead(BUTTON_LEFT) == LOW); 
  settingsMenuWrite(); 
}

void MenuSystem::settingsMenuWrite() 
{
  lcd.clear();
  lcd.write(7);

  display.printMenuLine(settingsMenuCursor - 1);

  lcd.setCursor(0, 1);
  lcd.write(2);
  lcd.setCursor(7, 1);
  lcd.write(byte(0));
  lcd.write(1);
  lcd.setCursor(15, 1);
  lcd.write(3);
}

void MenuSystem::settingsMenu() 
{
  lcd.clear();
  settingsMenuWrite();
  while (digitalRead(BUTTON_LEFT) == HIGH) 
  {
    if (digitalRead(BUTTON_UP) == LOW) 
    {
      while (digitalRead(BUTTON_UP) == LOW);
      timeOut = 0;
      settingsMenuCursor--;
      settingsMenuCursor = constrain(settingsMenuCursor, 1, 11);
      settingsMenuWrite();
    }
    if (digitalRead(BUTTON_DOWN) == LOW) 
    {
      while (digitalRead(BUTTON_DOWN) == LOW);
      timeOut = 0;
      settingsMenuCursor++;
      settingsMenuCursor = constrain(settingsMenuCursor, 1, 11);
      settingsMenuWrite();
    }
    if (digitalRead(BUTTON_RIGHT) == LOW) 
    {
      while (digitalRead(BUTTON_RIGHT) == LOW);
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
      }
    }
    delay(10);
    timeOut++;
    if (timeOut > 1000) break;
  }
  while (digitalRead(BUTTON_LEFT) == LOW);
  lcd.clear();
  infoDisplayWrite();
}