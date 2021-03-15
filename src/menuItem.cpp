#include "menuItem.h"

MenuItem::MenuItem(ArducamSSD1306& display,
      Transmitter& transmitter,
      byte messageType,
      const char *line1,
      const char *line2,
      const uint8_t value,
      const uint_fast8_t increment) {
  this->display = &display;
  this->transmitter = &transmitter;
  this->messageType = messageType;
  this->line1 = line1;
  this->line2 = line2;
  this->value = value;
  this->increment = increment;
}

void MenuItem::incrementValue() {
  this->setValue(this->value + this->increment);
}

void MenuItem::decrementValue() {
  this->setValue(this->value - this->increment);
}

void MenuItem::setValue(uint_fast8_t value) {
  this->value = value % 256;
  this->transmitter->sendMessage(this->messageType, this->value);
  this->displayNameAndGauge();
}

uint_fast8_t MenuItem::getValue() {
  return this->value;
}

void MenuItem::setIncrement(uint_fast8_t increment) {
  this->increment = increment;
}

void MenuItem::displayName() {
  this->displayMenuLabel(this->line1, this->line2);
  this->display->display();
}

void MenuItem::displayNameAndGauge() {
  this->displayMenuLabel(this->line1, this->line2);
  this->drawGauge();
  this->display->display();
}

void MenuItem::displayLineFont3(const uint_fast8_t y, const char *line) {
  const uint_fast8_t pixelWidth = strlen(line) * (this->font3Width + this->font3Pad);
  const uint_fast8_t offset = ((128 - pixelWidth)/2) - 2;
  this->display->setTextSize(3);
  this->display->setCursor(offset, y);
  // display.print(strlen(line));
  // display.print(" ");
  // display.print(pixelWidth);
  // display.print(" ");
  // display.print(offset);
  this->display->print(line);
}

void MenuItem::displayLineFont2(const uint_fast8_t y, const char *line) {
  const uint_fast8_t pixelWidth = strlen(line) * (this->font2Width + this->font2Pad);
  const uint_fast8_t offset = ((128 - pixelWidth)/2) - 2;
  this->display->setTextSize(2);
  this->display->setCursor(offset, y);
  // display.print(strlen(line));
  // display.print(" ");
  // display.print(pixelWidth);
  // display.print(" ");
  // display.print(offset);
  this->display->print(line);
}

void MenuItem::displayMenuLabel(const char *line1, const char *line2) {
  this->display->clearDisplay();
  if (strlen(line1) > 7) {
    this->displayLineFont2(this->singleLineY, line1);
  } else if (strlen(line2) < 1) {
    this->displayLineFont3(this->singleLineY, line1);
  } else {
    this->displayLineFont3(this->line1Y, line1);
    this->displayLineFont3(this->line2Y, line2);
  }
}

void MenuItem::drawGauge() {
  uint_fast8_t width = this->value / 2;
  this->display->drawRect(0, 0, 128, 16, 1);
  this->display->fillRect(0, 0, width, 16, 1);
}
