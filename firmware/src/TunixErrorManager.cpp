#include "TunixErrorManager.h"
#include "TunixMemoryManager.h"
#include "Display.h"

TunixErrorManager::TunixErrorManager() { }

// --- Add Additional Error Handler Functions Here

void TunixErrorManager::errorHandler(ErrorCode code) // --- Standart Error Handlers
{
  byte numericCode = static_cast<byte>(code);
  char errorCodeStr[4];
  itoa(numericCode, errorCodeStr, 10);

  switch(code) 
  {
    case ErrorCode::FIRMWARE_VERSION_ERROR:
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
      while(digitalRead(Pins::BUTTON_RIGHT) == HIGH);
      memory.factoryReset();
      break;

    case ErrorCode::VERSION_NOT_SUPPORTED:
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
      while(digitalRead(Pins::BUTTON_RIGHT) == HIGH);
      memory.factoryReset();
      break;

    case ErrorCode::DOWNGRADED_FIRMWARE:
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
      while(digitalRead(Pins::BUTTON_RIGHT) == HIGH);
      memory.factoryReset();
      break;

    case ErrorCode::PROGRAM_LOOP_FAILURE:
      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" Program Error"));
      lcd.setCursor(0, 1);
      lcd.print(F("PLS RESET"));
      while(true);
      break;

    case ErrorCode::RAM_FAILURE:
      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" RAM Error"));
      lcd.setCursor(0, 1);
      lcd.print(F("FAULTY DEVICE!"));
      while(true);
      break;

    default:
      lcd.clear();
      lcd.print(F("CRITICAL ERROR"));
      lcd.setCursor(0, 1);
      lcd.print(F("UNKNOWN ERROR!"));
      while(true);
      break;
  }

  return;
}