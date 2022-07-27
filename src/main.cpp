#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <VirtualWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>
#include <ArducamSSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)
#include "menuItem.h"
#include "menuItemOnOff.h"
#include "transmitter.h"
#include <Encoder.h>



/*
                    ┌───────────────┐
                    │GND         Vin│
                    │0           GND│
                    │1           3.3│
                    │2            23│
                    │3            22│
                    │4            21│
                    │5            20│
                    │6            19│ - SCL
                    │7            18│ - SDA
                    │8            17│
                    │9            16│
        reSwitchPin │10   3 G P   15│
          reDTPin - │11 1 . N G 2 14│ - transmitPin
         reCLKPin - │12 7 3 D M 6 13│
                    └───────────────┘
*/

// RADIO TRANSMITTER
const byte authByteStart = 117;
const byte authByteEnd = 115;
const uint_fast8_t transmitPin = 14;
Transmitter transmitter(transmitPin, authByteStart, authByteEnd);

// COLOR SENSOR
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint8_t readHue();
uint8_t calcHue(float r, float g, float b);

// ROTARY ENCODER
const uint_fast8_t reSwitchPin = 10;
const uint_fast8_t reDTPin = 11;
const uint_fast8_t reCLKPin = 12;
uint8_t reOldPosition = 128;
Encoder myEnc(reDTPin, reCLKPin);

// BUTTON



bool colorSensorOn = true;

// DISPLAY
#define OLED_RESET  16  // -RESET digital signal
ArducamSSD1306 display(OLED_RESET); // FOR I2C


// message types
const byte typeIgnore = 0;
const byte typeCycle = 1;
const byte typeBrightness = 2;
const byte typeDensity = 3;
const byte typeSparkles = 4;
const byte typeHue = 5;
const byte typeStreaks = 7;
const byte typeSolid = 8;
const byte typeSteal = 9;

// MENU STATE
const uint_fast8_t stealColorMenuIndex = 0;
const uint_fast8_t hueMenuIndex = 7;
const uint_fast8_t menuItemsCount = 8;
MenuItem * menuItems[menuItemsCount];
uint_fast8_t currentMenuItem = 0;

// function defs
void simpleDisplay(const char *message);
void stealColor();
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

  // ROTARY ENCODER SETUP
  pinMode(reSwitchPin, INPUT);

  // MENU ITEMS

  menuItems[stealColorMenuIndex] = new MenuItemOnOff(display,
    transmitter,
    typeIgnore,
    "Steal",
    "Color",
    0,
    1);

  // probably can just set this off in the loop after a few seconds
  menuItems[1] = new MenuItemOnOff(display,
    transmitter,
    typeCycle,
    "Cycle",
    "Colors",
    0,
    1);

  menuItems[2] = new MenuItem(display,
    transmitter,
    typeBrightness,
    "Brightness",
    "",
    223,
    8);

  menuItems[3] = new MenuItem(display,
    transmitter,
    typeStreaks,
    "Streaks",
    "",
    1,
    8);

  menuItems[4] = new MenuItem(display,
    transmitter,
    typeDensity,
    "Music",
    "",
    23,
    4);

  menuItems[5] = new MenuItem(display,
    transmitter,
    typeSparkles,
    "Sparkles",
    "",
    65,
    8);

  menuItems[6] = new MenuItemOnOff(display,
    transmitter,
    typeSolid,
    "Solid",
    "",
    0,
    1);

  menuItems[hueMenuIndex] = new MenuItem(display,
    transmitter,
    typeHue,
    "Hue",
    "",
    0,
    2);

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

float gauge = 0.5;

bool lastReSwitchPin = HIGH;

///////////////////////////////////////////////////////////////////
// LOOP
///////////////////////////////////////////////////////////////////
void loop() {
  bool currentReSwitchPin = digitalRead(reSwitchPin);

  // PUSH BUTTON
  if (lastReSwitchPin == HIGH && currentReSwitchPin == LOW) {
    myEnc.write(currentMenuItem * 4);
  }

  // RELEASE BUTTON
  if (lastReSwitchPin == LOW && currentReSwitchPin == HIGH) {
    myEnc.write(menuItems[currentMenuItem]->getValue());
  }

  uint8_t position = myEnc.read();
  if (position != reOldPosition) {
    Serial.print("position: ");
    Serial.println(position);
    reOldPosition = position;


    if (currentReSwitchPin == LOW) {
      currentMenuItem = (position/4) % menuItemsCount;
      menuItems[currentMenuItem]->displayNameAndGauge();
    } else {
      menuItems[currentMenuItem]->setValue(position);
    }

    lastReSwitchPin = currentReSwitchPin;
  }
}

///////////////////////////////////////////////////////////////////
// \ LOOP END /
///////////////////////////////////////////////////////////////////


void menu() {
  menuNext();
}

void up() {
  menuItems[currentMenuItem]->incrementValue();
}

void down() {
  menuItems[currentMenuItem]->decrementValue();
}

void menuNext() {
  currentMenuItem = (currentMenuItem + 1) % menuItemsCount;
  menuItems[currentMenuItem]->displayNameAndGauge();
  delay(200);
}

void menuPrevious() {
  currentMenuItem = (currentMenuItem - 1) % menuItemsCount;
  menuItems[currentMenuItem]->displayNameAndGauge();
  delay(200);
}

void stealColor() {
  byte hue = readHue();

  Serial.println("stealColor");

  menuItems[hueMenuIndex]->setValue(hue);
  transmitter.sendMessage(typeSteal, hue);

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
