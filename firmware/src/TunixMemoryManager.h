#ifndef TUNIXMEMORYMANAGER_H
#define TUNIXMEMORYMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Constants.h"

// --- EEPROM ADRESSES ---

#define VERSION_ADRESS    	0
#define SETTINGS_ADRESS   	(VERSION_ADRESS + sizeof(TunixMemoryManager::versionStruct))
#define BASE_CONFIG_ADRESS  (SETTINGS_ADRESS + sizeof(TunixMemoryManager::settingsStruct))

class TunixMemoryManager
{
	private:
		int _EEPROMSize;
		long getVersionValue(byte major, byte minor, byte patch);
	
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

		struct versionStruct
		{
			byte major;
			byte minor;
			byte patch;
		};

		struct RGBConfig 
		{
			byte R;
			byte G;
			byte B;
		};
		
		settingsStruct settings;
		settingsStruct defaultSettings;
		versionStruct version;		
		RGBConfig activeConfig;
		
		TunixMemoryManager(int EEPROMSize);
	
		void getBasicMemory();
		void saveBasicMemory();
		void factoryReset();
		byte firmwareValidate();

		void loadRGBConfig(byte index);
		void saveRGBConfig(byte index);
		void createNewRGBConfig();
		void deleteCurrentRGBConfig();
		byte readBrightnessForMode(byte mode);
		void saveBrightnessForMode(byte mode, byte brightness);
};

extern TunixMemoryManager memory;

#endif