#include "TunixErrorManager.h"
#include "TunixMemoryManager.h"
#include "Display.h"

TunixErrorManager::TunixErrorManager() { }

// --- Add Additional Error Handler Functions Here

void TunixErrorManager::errorHandler(int errorCode) // --- Standart Error Handlers
{
  char errorCodeStr[4];
  itoa(errorCode, errorCodeStr, 10);

  if(errorCode == FIRMWARE_VERSION_ERROR_CODE)
  {
    lcd.clear();
    lcd.print(errorCodeStr);
    lcd.print(F(" Version Incons. Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("Press right button to fac. reset"));
    delay(2000);
    for (int i = 0; i < 20; i++) 
    {
      delay(350);
      lcd.scrollDisplayLeft();
    }
    while(digitalRead(BUTTON_RIGHT) == HIGH);
    memory.factoryReset();
    return;
  }

  if(errorCode == VERSION_NOT_SUPPORTED_CODE)
  {
    lcd.clear();
    lcd.print(errorCodeStr);
    lcd.print(F(" Update Incons. Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("This update is not supported by previos ver."));
    delay(2000);
    for (int i = 0; i < 30; i++)
    {
      delay(350);
      lcd.scrollDisplayLeft();
    }
    delay(3000);
    lcd.clear();
    lcd.print(errorCodeStr);
    lcd.print(F(" Update Incons. Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("Press right button to fac. reset"));
    delay(2000);
    for (int i = 0; i < 20; i++)
    {
      delay(350);
      lcd.scrollDisplayLeft();
    }
    while(digitalRead(BUTTON_RIGHT) == HIGH);
    memory.factoryReset();
    return;
  }

  if(errorCode == DOWNGRADED_FIRMWARE_CODE)
  {
    lcd.clear();
    lcd.print(errorCodeStr);
    lcd.print(F(" Firmware Incons. Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("Device downgraded fac. reset is recommended"));
    delay(2000);
    for (int i = 0; i < 30; i++) 
    {
      delay(350);
      lcd.scrollDisplayLeft();
    }
    while(digitalRead(BUTTON_RIGHT) == HIGH);
    return;
  }
  
  if(errorCode == PROGRAM_LOOP_FAILURE_CODE)
  {
    lcd.clear();
    lcd.print(errorCodeStr);
    lcd.print(F(" Program Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("PLS RESET"));
    while(true);
  }

  if(errorCode == RAM_FAILURE_CODE)
  {
    lcd.clear();
    lcd.print(errorCodeStr);
    lcd.print(F(" RAM Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("FAULTY DEVICE!"));
    while(true);
  }
}