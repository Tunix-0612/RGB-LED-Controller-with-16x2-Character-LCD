#ifndef TMEMORYMANAGER_H
#define TMEMORYMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

#include "Constants.h"
#include "TErrorManager.h"

class TMemoryManager
{
  public:
    enum class PartitionID : uint8_t
    {
      VERSION       = 0,
      SETTINGS      = 1,
      DISPLAY_TEXT  = 2,
      RGB_CONFIGS   = 3,
      USER_DATA_3   = 4,
      USER_DATA_4   = 5,
      USER_DATA_5   = 6,
      USER_DATA_6   = 7,
      USER_DATA_7   = 8,
      USER_DATA_8   = 9,
    };

    #pragma pack(push, 1)
    struct settingsStruct
    {
      uint8_t selectedConfig;
      uint8_t selectedBrightness;
      uint8_t totalConfig;
      uint8_t brightnessMode2;
      uint8_t brightnessMode3;
      uint8_t brightnessMode4;
      uint8_t lcdBacklight;
      bool screenOffState;
      uint8_t LDRLimit;
      uint16_t startTime;
      uint16_t endTime;
      bool timerActive;
      bool LDRActive;
    };
    #pragma pack(pop)
    
    #pragma pack(push, 1)
    struct displayTextStruct
    {
      char idleTextUp[17];
			char idleTextBottom[17];
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct RGBConfig 
		{
			uint8_t R;
			uint8_t G;
			uint8_t B;
		};
    #pragma pack(pop)

    // --- Add additional memory structs here if needed

    VersionInfo eepromVersion;

    settingsStruct settings;

    displayTextStruct displayText;
    RGBConfig activeConfig;

    // --- Create objects here

    // ---------------------------------------------

    TMemoryManager(uint16_t EEPROMSize);

    void begin();

    void factoryReset();
    ErrorCode firmwareValidate();

    // Write/Read Data to/from Partitions
    template <typename T>
    void writeData(PartitionID id, const T& data, uint16_t itemIndex = 0) 
    { 
      uint8_t idIndex = static_cast<uint8_t>(id);
      if (idIndex >= MAX_PARTITIONS) errorManager.errorHandler(ErrorCode::ACCESS_VIOLATION);
      if (sizeof(T) > partitionTable[idIndex].size) errorManager.errorHandler(ErrorCode::BUFFER_OVERFLOW);
      EEPROM.put(getAddress(id, itemIndex), data);
    }

    template <typename T>
    void readData(PartitionID id, T& data, uint16_t itemIndex = 0) 
    { 
      uint8_t idIndex = static_cast<uint8_t>(id);
      if (idIndex >= MAX_PARTITIONS) errorManager.errorHandler(ErrorCode::ACCESS_VIOLATION);
      if (sizeof(T) > partitionTable[idIndex].size) errorManager.errorHandler(ErrorCode::BUFFER_OVERFLOW);
      EEPROM.get(getAddress(id, itemIndex), data);
    }
  
    void loadRGBConfig(uint8_t index);
    void saveRGBConfig(uint8_t index);
    void createNewRGBConfig();
    void deleteCurrentRGBConfig();
    uint8_t readBrightnessForMode(uint8_t mode);
    void saveBrightnessForMode(uint8_t mode, uint8_t brightness);

  private:
    // --- These constants holds the required information about partition table data ---
    static constexpr uint8_t MAX_PARTITIONS           = 10;
    static constexpr uint8_t HEADER_START_ADDRESS     = 0;
    static constexpr uint8_t HEADER_RESERVED_SIZE     = 64;
    static constexpr uint8_t DEVICE_SIGNATURE_ADRESS  = 60;
    static constexpr uint8_t TRANSACTION_FLAG_ADRESS  = 62;
    static constexpr uint8_t DATA_START_ADDRESS       = HEADER_START_ADDRESS + HEADER_RESERVED_SIZE;

    static inline constexpr uint16_t DEVICE_SIGNATURE      = 0x55AA;
    static inline constexpr uint16_t TRANSACTION_PROGRESS  = 0xBBAA;
    static inline constexpr uint16_t TRANSACTION_COMMITTED = 0xAABB;

    // --- This Struct holds both standard and custom struct's data ---
    struct PartitionEntry
    {
      uint16_t startAddress;
      uint16_t size;
      uint16_t count;
    };

    uint16_t _EEPROMSize;
    PartitionEntry partitionTable[MAX_PARTITIONS];

    ErrorCode resizePartition(PartitionID id, uint16_t newSize, uint16_t newCount = 1);
    void initPartitionTable();
    uint16_t getAddress(PartitionID id, uint16_t itemIndex = 0);
    void loadPartitionTable();
};

extern TMemoryManager memory;

#endif