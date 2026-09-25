#include "TErrorManager.h"

#include "TMemoryManager.h"
#include "Display.h"

TErrorManager::TErrorManager() { }

// --- Add Additional Error Handler Functions Here

void TErrorManager::errorHandler(StatusCode code) // --- Standart Error Handlers
{
  uint8_t numericCode = static_cast<uint8_t>(code);
  char errorCodeStr[4]; // Can be used to display in screens etc.
  itoa(numericCode, errorCodeStr, 10);

  switch(code) 
  {
    case StatusCode::NONE: break; // Continue normally

    case StatusCode::FIRMWARE_VERSION_ERROR:

      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" Version Incons. Error"));
      delay(2000);
      lcd.clear();
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

    case StatusCode::VERSION_NOT_SUPPORTED:
      
      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" Update Incons. Error"));
      delay(2000);
      lcd.clear();
      lcd.print(F("This update is not supported by previos ver."));
      delay(2000);
      for (int i = 0; i < 30; i++)
      {
        delay(350);
        lcd.scrollDisplayLeft();
      }
      delay(3000);
      lcd.clear();
      lcd.print(F("Press right button to fac. reset"));
      delay(2000);
      for (int i = 0; i < 18; i++)
      {
        delay(350);
        lcd.scrollDisplayLeft();
      }
      while(digitalRead(Pins::BUTTON_RIGHT) == HIGH);
      memory.factoryReset();

      break;

    case StatusCode::DOWNGRADED_FIRMWARE:
    
      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" Firmware Incons. Error"));
      delay(2000);
      lcd.clear();
      lcd.print(F("Device downgraded fac. reset is recommended"));
      delay(2000);
      for (int i = 0; i < 29; i++) 
      {
        delay(350);
        lcd.scrollDisplayLeft();
      }
      while(digitalRead(Pins::BUTTON_RIGHT) == HIGH);

      break;

    case StatusCode::EEPROM_FULL:

      while(true); // EEPROM is full, halt device and patch the update
      break;

    case StatusCode::ACCESS_VIOLATION:

      while(true); // Software related issue, halt device and patch the update
      break;

    case StatusCode::BUFFER_OVERFLOW:

      while(true); // Software related issue, halt device and patch the update
      break;

    case StatusCode::INTEGRITY_ERROR:

      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" Integrity err"));
      lcd.setCursor(0, 1);
      lcd.print(F("Pls factory res."));
      while(digitalRead(Pins::BUTTON_RIGHT) == HIGH);
      break;

    
    case StatusCode::PROGRAM_LOOP_FAILURE:

      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" Program Error"));
      lcd.setCursor(0, 1);
      lcd.print(F("PLS RESET"));
      while(true);

      break;

    case StatusCode::RAM_FAILURE:

      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" RAM Error"));
      lcd.setCursor(0, 1);
      lcd.print(F("FAULTY DEVICE!"));
      while(true);

      break;

    case StatusCode::RAM_LOW:

      while(true); // Low RAM Amount
      break;

    default:

      lcd.clear();
      lcd.print(errorCodeStr);
      lcd.print(F(" CRITICAL ERR"));
      lcd.setCursor(0, 1);
      lcd.print(F("UNKNOWN ERROR!"));
      while(true);

      break;
  }

  return;
}
