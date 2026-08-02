#ifndef TUNIXMEMORYMANAGER_H
#define TUNIXMEMORYMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Constants.h"

class TunixMemoryManager
{
	private:
		int _EEPROMSize;
	
	public:
		struct settingsStruct
		{
			byte lcdBacklight;
			bool screenOffState;
			byte LDRLimit;
			byte selectedConfig;
			byte selectedBrightness;
			byte brightnessMode2;
			byte brightnessMode3;
			byte brightnessMode4;
			byte totalConfig;
			char idleTextUp[17];
			char idleTextBottom[17];
			int startTime;
			int endTime;
		};

		struct RGBConfig 
		{
			byte R;
			byte G;
			byte B;
		};
		
		settingsStruct settings;
		settingsStruct defaultSettings;
		VersionInfo eepromVersion;
		RGBConfig activeConfig;
		
		TunixMemoryManager(int EEPROMSize);
	
		void getBasicMemory();
		void saveBasicMemory();
		void factoryReset();
		ErrorCode firmwareValidate();

		void loadRGBConfig(byte index);
		void saveRGBConfig(byte index);
		void createNewRGBConfig();
		void deleteCurrentRGBConfig();
		byte readBrightnessForMode(byte mode);
		void saveBrightnessForMode(byte mode, byte brightness);
};

// --- EEPROM Memory Addresses ---
constexpr int VERSION_ADRESS     = 0;
constexpr int SETTINGS_ADRESS    = VERSION_ADRESS + sizeof(VersionInfo);
constexpr int BASE_CONFIG_ADRESS = SETTINGS_ADRESS + sizeof(TunixMemoryManager::settingsStruct);

extern TunixMemoryManager memory;

#endif