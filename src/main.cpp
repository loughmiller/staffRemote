#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <VirtualWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>
#include <ArducamSSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)
#include "menuItem.h"
#include "menuItemStealColor.h"
#include "transmitter.h"

// RADIO TRANSMITTER
const byte authByteStart = 117;
const byte authByteEnd = 115;
const uint_fast8_t transmit_pin = 12;
Transmitter transmitter(transmit_pin, authByteStart, authByteEnd);

// COLOR SENSOR
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint8_t readHue();
uint8_t calcHue(float r, float g, float b);

// BUTTONS
const int buttons_vcc_pin = 15;
#define BUTTONS_INPUT A0
#define TOUCH_1 17

const uint_fast16_t fastButtonDelay = 20;
const uint_fast16_t slowButtonDelay = 500;
uint_fast16_t buttonDelay = fastButtonDelay;
const float touchThreshold = 1.25;

bool colorSensorOn = true;

// DISPLAY
#define OLED_RESET  16  // Pin 15 -RESET digital signal
ArducamSSD1306 display(OLED_RESET); // FOR I2C


// message types
const byte type_brightness = 2;
const byte type_density = 3;
const byte type_sparkles = 4;
const byte type_hue = 5;

// MENU STATE
const uint_fast8_t menuItemsCount = 5;
MenuItem * menuItems[menuItemsCount];
uint_fast8_t currentMenuItem = 0;

// function defs
void simpleDisplay(const char *message);
void stealColor();
void clearColor();
void menu();
void up();
void down();
void menuNext();
void menuPrevious();

///////////////////////////////////////////////////////////////////
// SETUP
///////////////////////////////////////////////////////////////////
void setup()
{
  // DISPLAY - SSD1306 Init
  display.begin();  // Switch OLED
  display.setTextColor(WHITE);

  simpleDisplay("booting");

  // COLOR SENSOR SETUP
  if (tcs.begin()) {
    Serial.println("Found color sensor");
    tcs.setInterrupt(true);
  } else {
    simpleDisplay("no color  sensor");
    delay(5);
  }

  uint_fast16_t colorSensorSetupTime = millis();

  // SETUP SERIAL CONNECTION FOR LOGGING
  while(!Serial && millis() < (colorSensorSetupTime + 10000));
  Serial.println("setup");

  // Use pulldown where buttons connect to vcc on press with resistors
  // to distingish different buttons on one input line
  pinMode(BUTTONS_INPUT, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // TRANSMITTER SETUP
  // const byte authByteStart = 117;
  // const byte authByteEnd = 115;
  // const uint_fast8_t transmit_pin = 12;
  // transmitter = new Transmitter(transmit_pin, authByteStart, authByteEnd);

  // MENU ITEMS

  menuItems[0] = new MenuItem(display,
    transmitter,
    type_brightness,
    "Brightness",
    "",
    223,
    8);

  menuItems[1] = new MenuItem(display,
    transmitter,
    type_density,
    "Density",
    "",
    23,
    4);

  menuItems[2] = new MenuItem(display,
    transmitter,
    type_sparkles,
    "Sparkles",
    "",
    65,
    4);

  menuItems[3] = new MenuItemStealColor(display,
    transmitter,
    type_hue,
    "Steal",
    "Color",
    0,
    1);

  menuItems[4] = new MenuItem(display,
    transmitter,
    type_hue,
    "Hue",
    "",
    0,
    2);

  // menuItems[4] = new MenuItem(display,
  //   transmitter,
  //   type_hue,
  //   "Hue",
  //   "fine",
  //   0,
  //   1);

  simpleDisplay("booting   complete");

  menuItems[currentMenuItem]->displayNameAndGauge();
}
///////////////////////////////////////////////////////////////////
// \ SETUP END /
///////////////////////////////////////////////////////////////////


bool readComplete = false;
bool buttonStabalizing = false;

uint_fast16_t buttons = 0;
uint_fast16_t lastButtons = 0;
uint_fast16_t touch1 = 0;
uint_fast16_t touchAvg = 10000;

float gauge = 0.5;

///////////////////////////////////////////////////////////////////
// LOOP
///////////////////////////////////////////////////////////////////
void loop() {

  delay(buttonDelay);

  buttons = analogRead(BUTTONS_INPUT);
  // Serial.println(buttons);

  if (buttons > 950) {
    menu();
    return;
  }

  if (buttons > 600) {
    down();
    return;
  }

  if (buttons > 400) {
    up();
    return;
  }

  if (colorSensorOn) {
    digitalWrite(LED_BUILTIN, HIGH);
    touch1 = touchRead(TOUCH_1);
    touchAvg = (float)touch1 * 0.02 + (float)touchAvg * 0.98;
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    touch1 = touchAvg;
  }

  if (touch1 >= touchAvg * touchThreshold && !readComplete) {
    stealColor();
    readComplete = true;
  }

  if (touch1 < touchAvg * touchThreshold) {
    readComplete = false;
  }

  buttonDelay = fastButtonDelay;
}

///////////////////////////////////////////////////////////////////
// \ LOOP END /
///////////////////////////////////////////////////////////////////


void menu() {
  buttonDelay = slowButtonDelay;
  menuNext();
}

void up() {
  menuItems[currentMenuItem]->incrementValue();
  buttonDelay = fastButtonDelay;
}

void down() {
  menuItems[currentMenuItem]->decrementValue();
  buttonDelay = fastButtonDelay;
}

void menuNext() {
  currentMenuItem = (currentMenuItem + 1) % menuItemsCount;
  menuItems[currentMenuItem]->displayNameAndGauge();
}

void menuPrevious() {
  currentMenuItem = (currentMenuItem - 1) % menuItemsCount;
}

void stealColor() {
  byte hue = readHue();

  // Serial.println("stealColor");
  // sendMessage(colorReadMessage, hue);
}

void clearColor() {
  // Serial.println("clearColor");
  // sendMessage(colorClearMessage, 0);
}

void simpleDisplay(const char *message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(1, 16);
  display.print(message);
  display.display();
}

///////////////////////////////////////////////////////////////////
// COLOR SENSOR FUNCTIONS
///////////////////////////////////////////////////////////////////

uint8_t readHue() {
  uint16_t clear, red, green, blue;
  float r, g, b;

  tcs.setInterrupt(false);  // LED ON
  delay(100);
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);   // LED OFF

  r = (float)red / (float)clear;
  g = (float)green / (float)clear;
  b = (float)blue / (float)clear;

  return calcHue(r, g, b);
}

uint8_t calcHue(float r, float g, float b) {
  float minC, maxC, delta, hue;

  minC = min(r, min(g, b));
  maxC = max(r, max(g, b));
  delta = maxC - minC;

  if(r == maxC) {
    hue = ( g - b ) / delta;
  } else if (g == maxC) {
    hue = 2 + (b - r) / delta;
  } else {
    hue = 4 + (r - g) / delta;
  }

  hue *= 60; // degrees
  if( hue < 0 ) {
    hue += 360;
  }

  return (uint8_t)((hue/360) * 255);
}

///////////////////////////////////////////////////////////////////
// \ COLOR SENSOR FUNCTIONS END /
///////////////////////////////////////////////////////////////////
