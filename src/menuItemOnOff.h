#include "menuItem.h"

#ifndef MENU_ITEM_STEAL_COLOR_H
#define MENU_ITEM_STEAL_COLOR_H

class MenuItemOnOff : public MenuItem {

  public :
    MenuItemOnOff(ArducamSSD1306& display,
      Transmitter& transmitter,
      byte messageType,
      const char *line1,
      const char *line2,
      const uint8_t value,
      const uint_fast8_t increment);
    void incrementValue();
    void decrementValue();
    void setValue(uint_fast8_t value, bool display = true);
};

#endif /* MENU_ITEM_STEAL_COLOR_H */
