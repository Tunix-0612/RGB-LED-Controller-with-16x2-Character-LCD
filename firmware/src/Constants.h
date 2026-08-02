#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

// --- PINOUT ---
namespace Pins 
{
  constexpr byte RGB_R = 11;
  constexpr byte RGB_G = 10;
  constexpr byte RGB_B = 9;

  constexpr byte BUTTON_UP    = 2;
  constexpr byte BUTTON_DOWN  = 3;
  constexpr byte BUTTON_RIGHT = 5;
  constexpr byte BUTTON_LEFT  = 4;

  constexpr byte LCD_BACKLIGHT = 6;
  constexpr byte LDR           = A7;
}

// --- SOFTWARE VERSION ---
struct VersionInfo 
{
  byte major;
  byte minor;
  byte patch;

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
  constexpr VersionInfo FIRMWARE = {1, 4, 6};
  constexpr VersionInfo MIN_SUPPORTED = {1, 4, 0};
}

// --- ERROR CODES ---
enum class ErrorCode : byte 
{
  NONE                    = 0,
  FIRMWARE_VERSION_ERROR = 11,
  VERSION_NOT_SUPPORTED  = 12,
  DOWNGRADED_FIRMWARE    = 13,
  PROGRAM_LOOP_FAILURE   = 14,
  RAM_FAILURE            = 15
};

#endif