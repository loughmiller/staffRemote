#include <Arduino.h>
#include <ArducamSSD1306.h>
#include "transmitter.h"

#ifndef MENU_ITEM_H
#define MENU_ITEM_H

class MenuItem {
  protected :
    ArducamSSD1306 *display;
    Transmitter *transmitter;
    const char *line1;
    const char *line2;
    byte messageType;
    uint_fast8_t value;
    uint_fast8_t increment;

    const uint_fast8_t font2Width = 9;
    const uint_fast8_t font2Pad = 2;
    const uint_fast8_t font3Width = 14;
    const uint_fast8_t font3Pad = 3;

    const uint_fast8_t line1Y = 18;
    const uint_fast8_t line2Y = 43;

    void displayLineFont2(const uint_fast8_t y, const char *line);
    void displayLineFont3(const uint_fast8_t y, const char *line);
    void displayMenuLabel(const char *line1, const char *line2);
    void drawGauge();
    void sendMessage(byte messageType, byte data);

  public :
    MenuItem(ArducamSSD1306& display,
      Transmitter& transmitter,
      byte messageType,
      const char *line1,
      const char *line2,
      const uint8_t value,
      const uint_fast8_t increment);
    void virtual incrementValue();
    void virtual decrementValue();
    void virtual setValue(uint_fast8_t value);
    uint_fast8_t getValue();
    void setIncrement(uint_fast8_t increment);
    void displayNameAndGauge();
};

#endif /* MENU_ITEM_H */
