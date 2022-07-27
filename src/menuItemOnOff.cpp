#include "menuItemOnOff.h"

MenuItemOnOff::MenuItemOnOff(ArducamSSD1306& display,
  Transmitter& transmitter,
  byte messageType,
  const char *line1,
  const char *line2,
  const uint8_t value,
  const uint_fast8_t increment)
: MenuItem(display, transmitter, messageType, line1, line2, value, increment) {
}

void MenuItemOnOff::incrementValue() {
  this->setValue(255);
}

void MenuItemOnOff::decrementValue() {
  this->setValue(0);
}