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
#pragma pack(push, 1)
struct VersionInfo 
{
  uint8_t major;
  uint8_t minor;
  uint8_t patch;

  char    phase;
  uint8_t iteration;

  // Get the phase value for comparison purposes
  uint8_t getPhaseValue() const 
  {
    switch (phase) 
    {
      case 'A': return 1;
      case 'B': return 2;
      case 'R': return 3;
      case 'S': return 4;
      default:  return 0;
    }
  }

  // Operators for comparison
  bool operator<(const VersionInfo& other) const 
  {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;

    uint8_t w1 = getPhaseValue();
    uint8_t w2 = other.getPhaseValue();
    if (w1 != w2) return w1 < w2;

    return iteration < other.iteration;
  }

  bool operator==(const VersionInfo& other) const 
  {
    return major == other.major && 
           minor == other.minor && 
           patch == other.patch && 
           phase == other.phase && 
           iteration == other.iteration;
  }
};
#pragma pack(pop)

// --- SYSTEM VERSION INFORMATION ---
// Define the current firmware version and the minimum supported version
// S = Stable, R = Release Candidate, B = Beta, A = Alpha
namespace SystemVersion 
{
  // Stable > Release Candidate > Beta > Alpha
  constexpr VersionInfo FIRMWARE      = {1, 6, 0, 'B', 2};
  constexpr VersionInfo MIN_SUPPORTED = {1, 5, 0, 'S', 0};
}

// --- ERROR CODES ---

// 0 = No Error, 100 = Operation Success, 101 = Operation Error
// 11x = Firmware Related Errors/Warnings
// 12x = EEPROM Related Errors/Warnings
// 13x = Program Related Errors/Warnings
// 14x = Internal RAM Related Errors/Warnings
// ABC --> A = Main Error Source, B = Sub Error Source, C = Specific Error Code
enum class StatusCode : uint16_t 
{
  NONE                    =  0,
  OPERATION_OK            = 100,
  OPERATION_ERROR         = 101,

  FIRMWARE_VERSION_ERROR  = 111,
  VERSION_NOT_SUPPORTED   = 112,
  DOWNGRADED_FIRMWARE     = 113,

  EEPROM_FULL             = 121,
  ACCESS_VIOLATION        = 122,
  BUFFER_OVERFLOW         = 123,
  INTEGRITY_ERROR         = 124,

  PROGRAM_LOOP_FAILURE    = 131,

  RAM_FAILURE             = 141,
  RAM_LOW                 = 142

  // --- Additional Error Codes
};

#endif