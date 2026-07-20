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
	EEPROM.get(VERSION_ADRESS, version);
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
	version.major = SOFTWARE_MAJOR_VERSION;
	version.minor = SOFTWARE_MINOR_VERSION;
	version.patch = SOFTWARE_PATCH_VERSION;
	EEPROM.put(VERSION_ADRESS, version);
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

// Convert version numbers to scalar ones such as 10302 for easier version check
long TunixMemoryManager::getVersionValue(byte major, byte minor, byte patch) { return ((long)major * 10000) + ((long)minor * 100) + patch; }

// --- Validate the firmware versions
byte TunixMemoryManager::firmwareValidate() 
{
  long currentVer = getVersionValue(version.major, version.minor, version.patch);
  long softwareVer = getVersionValue(SOFTWARE_MAJOR_VERSION, SOFTWARE_MINOR_VERSION, SOFTWARE_PATCH_VERSION);
  long minAllowedVer = getVersionValue(MINIMUM_SOFTWARE_MAJOR_VERSION, MINIMUM_SOFTWARE_MINOR_VERSION, MINIMUM_SOFTWARE_PATCH_VERSION);

  // If the version check is the same, return immediately
  if (currentVer == softwareVer) return 0;

  // If the software version is newer than saved version then proceed to update
  if (currentVer < softwareVer)
  {
    // Minimum valid update version check
    if (currentVer < minAllowedVer) return VERSION_NOT_SUPPORTED_CODE; 

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(F("Device is"));
    lcd.setCursor(1, 1);
    lcd.print(F("being updated"));
    delay(500);

    // --- 1.4.0 MIGRATION ---
    if (currentVer < 10400)
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
    if (currentVer == 10401)
    {
      settings.startTime = 1095;
      settings.endTime = 1395;

      EEPROM.put(SETTINGS_ADRESS, settings);
    }

    // Version Data Save
    version.major = SOFTWARE_MAJOR_VERSION;
    version.minor = SOFTWARE_MINOR_VERSION;
    version.patch = SOFTWARE_PATCH_VERSION;
    EEPROM.put(VERSION_ADRESS, version);

    lcd.clear();
    lcd.print(F("Updated Firmware"));
    lcd.setCursor(0, 1);
    lcd.print(F("PLS RESET"));
    while(true); // Lock the device
  }
  // Downgraded version check
  else return DOWNGRADED_FIRMWARE_CODE;
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