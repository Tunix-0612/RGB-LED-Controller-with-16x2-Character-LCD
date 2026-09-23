#include "TMemoryManager.h"

#include "TErrorManager.h"

TMemoryManager::TMemoryManager(uint16_t EEPROMSize)
{
  _EEPROMSize = EEPROMSize;

  // --- Assign Default values here - OR - in the factory reset with local defaultSettings Struct.
}

void TMemoryManager::begin()
{
  // Load / Init partition table.
  loadPartitionTable();

  readData(PartitionID::VERSION, eepromVersion);
  readData(PartitionID::SETTINGS, settings);
  readData(PartitionID::DISPLAY_TEXT, displayText);
}

ErrorCode TMemoryManager::resizePartition(PartitionID id, uint16_t newSize, uint16_t newCount)
{
  // Get the ID.
  uint8_t index = static_cast<uint8_t>(id);

  // If ID exceedes the maximimum return ACCESS_VIOLATION.
  if (index >= MAX_PARTITIONS) return ErrorCode::ACCESS_VIOLATION;

  // In case of a system shutdown while Partitioning.
  EEPROM.put(TRANSACTION_FLAG_ADRESS, TRANSACTION_PROGRESS);

  uint16_t oldTotalSize = partitionTable[index].size * partitionTable[index].count;
  uint16_t newTotalSize = newSize * newCount;

  if (oldTotalSize == newTotalSize) // If no change in size return.
  {
    partitionTable[index].size = newSize;
    partitionTable[index].count = newCount;
    EEPROM.put(HEADER_START_ADDRESS, partitionTable);
    EEPROM.put(TRANSACTION_FLAG_ADRESS, TRANSACTION_COMMITTED); // Mark as Transaction finished succseffuly.
    return ErrorCode::NONE;
  }

  // If partition is a new partition.
  if (partitionTable[index].startAddress == 0 && newTotalSize > 0)
  {
    if (index == 0) partitionTable[index].startAddress = DATA_START_ADDRESS;
    else
    {
      // Find the last active Partition's End Address
      for (int i = index - 1; i >= 0; i--)
      {
        if (partitionTable[i].size > 0 && partitionTable[i].count > 0)
        {
          partitionTable[index].startAddress = partitionTable[i].startAddress + (partitionTable[i].size * partitionTable[i].count);
          break;
        }
      }

      // If no active Partitions before, start from DATA_START_ADDRESS
      if (partitionTable[index].startAddress == 0) partitionTable[index].startAddress = DATA_START_ADDRESS;
    }
  }

  int shiftAmount = static_cast<int>(newTotalSize) - static_cast<int>(oldTotalSize);
  uint8_t nextIndex = index + 1;

  // Calculate the addresses of data that has to be migrated
  uint16_t moveStartAddress = 0;
  uint16_t moveEndAddress = 0;
  bool hasSubsequentData = false;

  for (uint8_t i = nextIndex; i < MAX_PARTITIONS; i++)
  {
    if (partitionTable[i].size > 0 && partitionTable[i].count > 0)
    {
      if (!hasSubsequentData)
      {
        moveStartAddress = partitionTable[i].startAddress;
        hasSubsequentData = true;
      }
      moveEndAddress = partitionTable[i].startAddress + (partitionTable[i].size * partitionTable[i].count);
    }
  }

  // If no data has to migrated update Partition table and return
  if (!hasSubsequentData)
  {
    // EEPROM Limit check
    if (partitionTable[index].startAddress + newTotalSize > _EEPROMSize) return ErrorCode::EEPROM_FULL;

    // If Partition is deleted by 0 as size, delete the start address as well
    if (newTotalSize == 0) partitionTable[index].startAddress = 0;
    partitionTable[index].size = newSize;
    partitionTable[index].count = newCount;
    EEPROM.put(HEADER_START_ADDRESS, partitionTable);
    EEPROM.put(TRANSACTION_FLAG_ADRESS, TRANSACTION_COMMITTED); // Mark as Transaction finished succseffuly.
    return ErrorCode::NONE;
  }

  // EEPROM Limit check
  if (shiftAmount > 0 && (moveEndAddress + shiftAmount) > _EEPROMSize) return ErrorCode::EEPROM_FULL;

  // Shift data on the EEPROM
  if (shiftAmount > 0) // Shift forward (From End to Start)
  {
    for (int32_t i = moveEndAddress - 1; i >= moveStartAddress; i--)
    {
      uint8_t data = EEPROM.read(i);
      EEPROM.update(i + shiftAmount, data);
    }
  }
  else // Shift backwards (From Start to End)
  {
    for (uint32_t i = moveStartAddress; i < moveEndAddress; i++)
    {
      uint8_t data = EEPROM.read(i);
      EEPROM.update(i + shiftAmount, data);
    }
  }

  // Update Partitions' start addresses
  for (uint8_t i = nextIndex; i < MAX_PARTITIONS; i++) 
    if (partitionTable[i].size > 0 && partitionTable[i].count > 0) partitionTable[i].startAddress += shiftAmount;

  // Update Target Partition's entry
  partitionTable[index].size = newSize;
  partitionTable[index].count = newCount;

  // If Partition is deleted by 0 as size, delete the start address as well
  if (newTotalSize == 0) partitionTable[index].startAddress = 0;

  EEPROM.put(HEADER_START_ADDRESS, partitionTable);

  EEPROM.put(TRANSACTION_FLAG_ADRESS, TRANSACTION_COMMITTED); // Mark as Transaction finished succseffuly.
  return ErrorCode::NONE;
}

void TMemoryManager::initPartitionTable()
{
  EEPROM.put(TRANSACTION_FLAG_ADRESS, TRANSACTION_PROGRESS);
  memset(partitionTable, 0, sizeof(partitionTable));

  uint8_t versionSize     = 5;
  uint8_t settingsSize    = 25;
  uint8_t displayTextSize = 35;
  uint8_t RGBConfigSize   = 3;
  
  // YOU CAN allocate more bytes to any partition including version and settings.
  // TPT (Tunix Partition Table) allows user to write/read data smaller than the partition size.
  // If you try to write +20 bytes to a 20 byte partition it will CALL "BUFFER_OVERFLOW" Error.

  // --- You can Allocate more memory than needed here.
  partitionTable[static_cast<uint8_t>(PartitionID::VERSION)].startAddress = DATA_START_ADDRESS;
  partitionTable[static_cast<uint8_t>(PartitionID::VERSION)].size = versionSize;
  partitionTable[static_cast<uint8_t>(PartitionID::VERSION)].count = 1;

  uint16_t settingsStart = DATA_START_ADDRESS + versionSize;

  partitionTable[static_cast<uint8_t>(PartitionID::SETTINGS)].startAddress = settingsStart;
  partitionTable[static_cast<uint8_t>(PartitionID::SETTINGS)].size = settingsSize;
  partitionTable[static_cast<uint8_t>(PartitionID::SETTINGS)].count = 1;

  // --- Custom structs' partitioning should be here

  uint16_t displayTextStart = settingsStart + settingsSize;

  partitionTable[static_cast<uint8_t>(PartitionID::DISPLAY_TEXT)].startAddress = displayTextStart;
  partitionTable[static_cast<uint8_t>(PartitionID::DISPLAY_TEXT)].size = displayTextSize;
  partitionTable[static_cast<uint8_t>(PartitionID::DISPLAY_TEXT)].count = 1;

  uint16_t RGBConfigStart = displayTextStart + displayTextSize;

  partitionTable[static_cast<uint8_t>(PartitionID::RGB_CONFIGS)].startAddress = RGBConfigStart;
  partitionTable[static_cast<uint8_t>(PartitionID::RGB_CONFIGS)].size = RGBConfigSize;
  partitionTable[static_cast<uint8_t>(PartitionID::RGB_CONFIGS)].count = 20;

  EEPROM.put(HEADER_START_ADDRESS, partitionTable);

  factoryReset();

  EEPROM.put(DEVICE_SIGNATURE_ADRESS, DEVICE_SIGNATURE);
  EEPROM.put(TRANSACTION_FLAG_ADRESS, TRANSACTION_COMMITTED);
}

uint16_t TMemoryManager::getAddress(PartitionID id, uint16_t itemIndex) 
{
    uint8_t idIndex = static_cast<uint8_t>(id);
    PartitionEntry& entry = partitionTable[idIndex];

    if (itemIndex >= entry.count) errorManager.errorHandler(ErrorCode::ACCESS_VIOLATION);

    return entry.startAddress + (itemIndex * entry.size);
}

void TMemoryManager::loadPartitionTable()
{
  uint16_t signature;
  EEPROM.get(DEVICE_SIGNATURE_ADRESS, signature);

  uint16_t transactionFlag;
  EEPROM.get(TRANSACTION_FLAG_ADRESS, transactionFlag);

  // If there is a signature error or failed update attempt re-initialize Partition Table.
  if (signature != DEVICE_SIGNATURE || transactionFlag == TRANSACTION_PROGRESS)
  {
    errorManager.errorHandler(ErrorCode::INTEGRITY_ERROR);

    initPartitionTable();
  }
  else EEPROM.get(HEADER_START_ADDRESS, partitionTable);
}

void TMemoryManager::factoryReset()
{
  for (uint16_t i = 64; i < EEPROM.length(); i++) EEPROM.update(i, 255);

  // --- Assign Default Values Here if no global defaultVariables is present.

  settingsStruct defaultSettings;
  displayTextStruct defaultDisplayText;
  RGBConfig defaultRGBConfig;

  defaultSettings.brightnessMode2 = 135;
  defaultSettings.brightnessMode3 = 60;
  defaultSettings.brightnessMode4 = 15;
  defaultSettings.selectedBrightness = 1;
  defaultSettings.selectedConfig = 0;
  defaultSettings.totalConfig = 1;
  defaultSettings.startTime = 1095;
  defaultSettings.endTime = 1395;
  defaultSettings.lcdBacklight = 90;
  defaultSettings.screenOffState = false;
  defaultSettings.LDRLimit = 100;
  defaultSettings.timerActive = true;
  defaultSettings.LDRActive = false;

  strncpy(defaultDisplayText.idleTextUp, "      TUNIX     ", 16);
  defaultDisplayText.idleTextUp[16] = '\0';
  strncpy(defaultDisplayText.idleTextBottom, "   ELECTRONICS  ", 16);
  defaultDisplayText.idleTextBottom[16] = '\0';

  defaultRGBConfig.R = 255;
  defaultRGBConfig.G = 125;
  defaultRGBConfig.B = 55;

  writeData(PartitionID::SETTINGS, defaultSettings);

  eepromVersion = SystemVersion::FIRMWARE;
  writeData(PartitionID::VERSION, eepromVersion);

  writeData(PartitionID::DISPLAY_TEXT, defaultDisplayText);
  writeData(PartitionID::RGB_CONFIGS, defaultRGBConfig, 0);

  delay(500);
}

// Validate the firmware versions
ErrorCode TMemoryManager::firmwareValidate()
{
  // If no version difference, return no error
  if (eepromVersion == SystemVersion::FIRMWARE) return ErrorCode::NONE;

  // If the firmware version is greater than the EEPROM version, it means the firmware has been updated.
  if (eepromVersion < SystemVersion::FIRMWARE)
  {
    // If the EEPROM version is less than the minimum supported version, return an error.
    if (eepromVersion < SystemVersion::MIN_SUPPORTED) return ErrorCode::VERSION_NOT_SUPPORTED;

    delay(2000);

    if (eepromVersion == VersionInfo{1, 4, 2}) // --- Example version declare.
    {
      // --- Version Specific Update/Fix Example
    }

    // --- Generic update path
    
    // Save the new firmware version to EEPROM
    
    
    while (true); // Lock the Device.
  }

  eepromVersion = SystemVersion::FIRMWARE;
  writeData(PartitionID::SETTINGS, settings);
  writeData(PartitionID::VERSION, eepromVersion);
  
  // If device is downgraded return the error/warning
  return ErrorCode::DOWNGRADED_FIRMWARE;
}

// ---------------------------------------------
// DEVICE RELATED MEMORY FUNCTIONS
// ---------------------------------------------
void TMemoryManager::loadRGBConfig(uint8_t index)
{
  if(index >= settings.totalConfig) index = 0; 
  readData(PartitionID::RGB_CONFIGS, activeConfig, index);
  if(activeConfig.R == 255 && activeConfig.G == 255 && activeConfig.B == 255) 
  {
      activeConfig.R = 0; 
      activeConfig.G = 0; 
      activeConfig.B = 0;
  }
  settings.selectedConfig = index;
}

void TMemoryManager::saveRGBConfig(uint8_t index)
{
  writeData(PartitionID::RGB_CONFIGS, activeConfig, index);
}

void TMemoryManager::createNewRGBConfig() 
{
  if(settings.totalConfig < 20)
  {
    settings.totalConfig++;
    settings.selectedConfig = settings.totalConfig - 1;

    writeData(PartitionID::SETTINGS, settings);
    saveRGBConfig(settings.selectedConfig); 
  }
}

void TMemoryManager::deleteCurrentRGBConfig() 
{
  if (settings.totalConfig <= 1) return;
  for (uint8_t i = settings.selectedConfig; i < settings.totalConfig - 1; i++) 
  {
    uint16_t currentIndex = settings.selectedConfig;
    uint16_t nextIndex = settings.selectedConfig += 1;
    
    RGBConfig nextGroup;
    readData(PartitionID::RGB_CONFIGS, nextGroup, nextIndex);
    writeData(PartitionID::RGB_CONFIGS, nextGroup, currentIndex);
  }
  settings.totalConfig--;
  if (settings.selectedConfig >= settings.totalConfig) settings.selectedConfig = settings.totalConfig - 1;
  writeData(PartitionID::SETTINGS, settings);
  loadRGBConfig(settings.selectedConfig);
}

uint8_t TMemoryManager::readBrightnessForMode(uint8_t mode) 
{
  if(mode == 1) return 255;
  uint8_t val = 255;
  if(mode == 2) val = settings.brightnessMode2;
  if(mode == 3) val = settings.brightnessMode3;
  if(mode == 4) val = settings.brightnessMode4;
  if(val == 255 && mode != 1) return 125; 
  return val;
}

void TMemoryManager::saveBrightnessForMode(uint8_t mode, uint8_t brightness)
{
  if(mode == 2) settings.brightnessMode2 = brightness;
  if(mode == 3) settings.brightnessMode3 = brightness;
  if(mode == 4) settings.brightnessMode4 = brightness;
  writeData(PartitionID::SETTINGS, settings);
}