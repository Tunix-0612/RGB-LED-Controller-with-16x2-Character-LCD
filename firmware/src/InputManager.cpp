#include "InputManager.h"

InputManager::InputManager(byte pinUp, byte pinDown, byte pinLeft, byte pinRight) 
{
  buttons[BTN_UP].pin = pinUp;
  buttons[BTN_DOWN].pin = pinDown;
  buttons[BTN_LEFT].pin = pinLeft;
  buttons[BTN_RIGHT].pin = pinRight;

  for (byte i = 0; i < BTN_COUNT; i++) 
  {
    buttons[i].lastRawState = HIGH;
    buttons[i].debouncedState = HIGH;
    buttons[i].lastDebounceTime = 0;
    buttons[i].pressStartTime = 0;
    buttons[i].longPressHandled = false;
  }
}

void InputManager::begin() { for (byte i = 0; i < BTN_COUNT; i++) pinMode(buttons[i].pin, INPUT_PULLUP); }

void InputManager::update() 
{
  unsigned long currentMillis = millis();

  for (byte i = 0; i < BTN_COUNT; i++) 
  {
    bool rawRead = digitalRead(buttons[i].pin);
    if (rawRead != buttons[i].lastRawState) 
    {
      buttons[i].lastDebounceTime = currentMillis;
      buttons[i].lastRawState = rawRead;
    }

    if ((currentMillis - buttons[i].lastDebounceTime) > debounceDelay) 
    {
      if (rawRead != buttons[i].debouncedState) 
      {
        buttons[i].debouncedState = rawRead;
        if (buttons[i].debouncedState == LOW) 
        {
          buttons[i].pressStartTime = currentMillis;
          buttons[i].longPressHandled = false;
        }
      }
    }
  }
}

ButtonEvent InputManager::getEvent(ButtonID btn)
{
  unsigned long currentMillis = millis();
  ButtonState &b = buttons[btn];

  if (b.debouncedState == LOW)
  {
    if (!b.longPressHandled && (currentMillis - b.pressStartTime >= longPressTime)) 
    {
      b.longPressHandled = true;
      return BTN_EVENT_LONG_PRESS;
    }
  } 
  else 
  {
    if (b.pressStartTime > 0) 
    {
      bool wasLong = b.longPressHandled;
      b.pressStartTime = 0;
      b.longPressHandled = false;

      if (!wasLong) return BTN_EVENT_CLICK;
    }
  }
  return BTN_EVENT_NONE;
}

bool InputManager::isPressed(ButtonID btn) { return (buttons[btn].debouncedState == LOW); }