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

void MenuItemOnOff::setValue(uint_fast8_t value, bool display) {
  Serial.print("MenuItemOnOff::setValue");
  // Serial.print("\t");
  // Serial.println(value);

  Serial.print(value/4);
  Serial.print("\t");
  Serial.println(((value / 4) % 2));
  if (((value / 4) % 2) == 1) {
    // ON
    MenuItem::setValue(255, display);
  } else {
    // OFF
    MenuItem::setValue(0, display);
  }
}