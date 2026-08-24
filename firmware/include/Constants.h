#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

// --- PINOUT ---
namespace Pins 
{
  constexpr uint8_t RGB_R = 11;
  constexpr uint8_t RGB_G = 10;
  constexpr uint8_t RGB_B = 9;

  constexpr uint8_t BUTTON_UP    = 2;
  constexpr uint8_t BUTTON_DOWN  = 3;
  constexpr uint8_t BUTTON_RIGHT = 5;
  constexpr uint8_t BUTTON_LEFT  = 4;

  constexpr uint8_t LCD_BACKLIGHT = 6;
  constexpr uint8_t LDR           = A7;
}

// --- SOFTWARE VERSION ---
struct VersionInfo 
{
  uint8_t major;
  uint8_t minor;
  uint8_t patch;

  bool operator<(const VersionInfo& other) const 
  {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
  }

  bool operator==(const VersionInfo& other) const 
  {
    return major == other.major && minor == other.minor && patch == other.patch;
  }
};

namespace SystemVersion 
{
  constexpr VersionInfo FIRMWARE = {1, 5, 0};
  constexpr VersionInfo MIN_SUPPORTED = {1, 5, 0};
}

// --- ERROR CODES ---
enum class ErrorCode : uint16_t 
{
  NONE                    =  0,
  FIRMWARE_VERSION_ERROR  = 101,
  VERSION_NOT_SUPPORTED   = 102,
  DOWNGRADED_FIRMWARE     = 103,

  PROGRAM_LOOP_FAILURE    = 111,

  RAM_FAILURE             = 121,
  RAM_LOW                 = 122,

  EEPROM_FULL             = 131,
  ACCESS_VIOLATION        = 132,
  BUFFER_OVERFLOW         = 133,
  INTEGRITY_ERROR         = 134

  // --- Additional Error Codes
};

#endif