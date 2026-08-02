#include "TunixMemoryManager.h"
#include "Display.h"

TunixMemoryManager::TunixMemoryManager(int EEPROMSize)
{
	_EEPROMSize = EEPROMSize - 1;
	// Assign defaultSettings values here
  defaultSettings.lcdBacklight = 255;
  defaultSettings.screenOffState = false;
  defaultSettings.LDRLimit = 100;
  defaultSettings.selectedConfig = 0;
  defaultSettings.selectedBrightness = 1;
  defaultSettings.brightnessMode2 = 150;
  defaultSettings.brightnessMode3 = 80;
  defaultSettings.brightnessMode4 = 40;
  defaultSettings.totalConfig = 1;
  strncpy(defaultSettings.idleTextUp, "      TUNIX     ", 16);
  defaultSettings.idleTextUp[16] = '\0';
  strncpy(defaultSettings.idleTextBottom, "   ELECTRONICS  ", 16);
  defaultSettings.idleTextBottom[16] = '\0';
  defaultSettings.startTime = 1095;
  defaultSettings.endTime = 1395;
}

void TunixMemoryManager::getBasicMemory()
{
	EEPROM.get(VERSION_ADRESS, eepromVersion);
	EEPROM.get(SETTINGS_ADRESS, settings);
	// --- Add Additional Device Memory Recalls Here
	return;
}

void TunixMemoryManager::saveBasicMemory()
{
	EEPROM.put(SETTINGS_ADRESS, settings);
	// --- Add Additional Device Memory Saves Here
	return;
}

void TunixMemoryManager::factoryReset()
{
  lcd.clear();
  lcd.print(F("INITIALIZING..."));
	// --- Add Pre-Reset Commands
	// --- Add Deafult settings here if you want to
	settings = defaultSettings;
	EEPROM.put(SETTINGS_ADRESS, settings);

	eepromVersion = SystemVersion::FIRMWARE;
	EEPROM.put(VERSION_ADRESS, eepromVersion);

	// --- Processes After Reset
  activeConfig.R = 255;
  activeConfig.G = 0;
  activeConfig.B = 0;
  EEPROM.put(BASE_CONFIG_ADRESS, activeConfig);

  delay(500);
  lcd.clear();
  lcd.print(F("RESETED!"));
	while(true);
}

// --- Validate the firmware versions
ErrorCode TunixMemoryManager::firmwareValidate() 
{
  // If no version difference, return no error
  if (eepromVersion == SystemVersion::FIRMWARE) return ErrorCode::NONE;

  // If the firmware version is greater than the EEPROM version, it means the firmware has been updated.
  if (eepromVersion < SystemVersion::FIRMWARE)
  {
    // If the EEPROM version is less than the minimum supported version, return an error.
    if (eepromVersion < SystemVersion::MIN_SUPPORTED) return ErrorCode::VERSION_NOT_SUPPORTED;

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(F("Device is"));
    lcd.setCursor(1, 1);
    lcd.print(F("being updated"));
    delay(500);

    // --- 1.4.0 MIGRATION ---
    if (eepromVersion < VersionInfo{1, 4, 0})
    {
      delay(500);

      int offset = 4;

      for (int i = settings.totalConfig - 1; i >= 0; i--)
      {
        RGBConfig tempConfig;

        int oldAddress = (BASE_CONFIG_ADRESS - offset) + (i * sizeof(RGBConfig));
        EEPROM.get(oldAddress, tempConfig);

        int newAddress = BASE_CONFIG_ADRESS + (i * sizeof(RGBConfig));
        EEPROM.put(newAddress, tempConfig);
      }

      settings.startTime = 1095;
      settings.endTime = 1395;

      EEPROM.put(SETTINGS_ADRESS, settings);
    }

    // --- 1.4.1 START AND END TIMER FIX ---
    if (eepromVersion == VersionInfo{1, 4, 1})
    {
      settings.startTime = 1095;
      settings.endTime = 1395;

      EEPROM.put(SETTINGS_ADRESS, settings);
    }

    // Save the new firmware version to EEPROM
    eepromVersion = SystemVersion::FIRMWARE;
    EEPROM.put(VERSION_ADRESS, eepromVersion);

    lcd.clear();
    lcd.print(F("Updated Firmware"));
    lcd.setCursor(0, 1);
    lcd.print(F("PLS RESET"));
    
    while (true); // Lock the Device.
  }

  return ErrorCode::DOWNGRADED_FIRMWARE;
}

void TunixMemoryManager::loadRGBConfig(byte index)
{
  if(index >= settings.totalConfig) index = 0; 
  int baseAddress = BASE_CONFIG_ADRESS + (index * sizeof(RGBConfig));
  EEPROM.get(baseAddress, activeConfig);
  if(activeConfig.R == 255 && activeConfig.G == 255 && activeConfig.B == 255) 
  {
      activeConfig.R = 0; 
      activeConfig.G = 0; 
      activeConfig.B = 0;
  }
  settings.selectedConfig = index;
}

void TunixMemoryManager::saveRGBConfig(byte index)
{
  int baseAddress = BASE_CONFIG_ADRESS + (index * sizeof(RGBConfig));
  EEPROM.put(baseAddress, activeConfig);
}

void TunixMemoryManager::createNewRGBConfig() 
{
  if(settings.totalConfig < 20)
  {
    settings.totalConfig++;
    settings.selectedConfig = settings.totalConfig - 1;
    saveBasicMemory();
    saveRGBConfig(settings.selectedConfig); 
  }
}

void TunixMemoryManager::deleteCurrentRGBConfig() 
{
  if (settings.totalConfig <= 1) return;
  for (byte i = settings.selectedConfig; i < settings.totalConfig - 1; i++) 
  {
    int currentAddress = BASE_CONFIG_ADRESS + (i * sizeof(RGBConfig));
    int nextAddress = BASE_CONFIG_ADRESS + ((i + 1) * sizeof(RGBConfig));
    
    RGBConfig nextGroup;
    EEPROM.get(nextAddress, nextGroup);
    EEPROM.put(currentAddress, nextGroup);
  }
  settings.totalConfig--;
  if (settings.selectedConfig >= settings.totalConfig) settings.selectedConfig = settings.totalConfig - 1;
  EEPROM.put(SETTINGS_ADRESS, settings);
  loadRGBConfig(settings.selectedConfig);
}

byte TunixMemoryManager::readBrightnessForMode(byte mode) 
{
  if(mode == 1) return 255;
  byte val = 255;
  if(mode == 2) val = settings.brightnessMode2;
  if(mode == 3) val = settings.brightnessMode3;
  if(mode == 4) val = settings.brightnessMode4;
  if(val == 255 && mode != 1) return 125; 
  return val;
}

void TunixMemoryManager::saveBrightnessForMode(byte mode, byte brightness)
{
  if(mode == 2) settings.brightnessMode2 = brightness;
  if(mode == 3) settings.brightnessMode3 = brightness;
  if(mode == 4) settings.brightnessMode4 = brightness;
  EEPROM.put(SETTINGS_ADRESS, settings);
}