#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <VirtualWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>
#include <ArducamSSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)
#include "menuItem.h"

// RADIO TRANSMITTER
const byte authByteStart = 117;
const byte authByteEnd = 115;

const uint_fast8_t transmit_pin = 12;
const uint_fast16_t buttonDelay = 300;

uint8_t messageID = 0;

const byte brightness = 2;

const float touchThreshold = 1.25;

// COLOR SENSOR
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint8_t readHue();
uint8_t calcHue(float r, float g, float b);

// BUTTON PINS
const int buttons_vcc_pin = 15;
#define BUTTONS_INPUT A0
#define TOUCH_1 17

bool colorSensorOn = true;

// DISPLAY
#define OLED_RESET  16  // Pin 15 -RESET digital signal
ArducamSSD1306 display(OLED_RESET); // FOR I2C

const uint_fast8_t font2Width = 9;
const uint_fast8_t font2Pad = 2;
const uint_fast8_t font3Width = 14;
const uint_fast8_t font3Pad = 3;

const uint_fast8_t line1Y = 18;
const uint_fast8_t line2Y = 43;
const uint_fast8_t singleLineY = 35;

MenuItem * menuItems[1];

uint_fast8_t currentMenuItem = 0;

// function defs
void simpleDisplay(const char *message);
void stealColor();
void clearColor();
void menu();
void sendMessage(byte messageType, byte data);

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
  Serial.println("setup transmitter");
  vw_set_tx_pin(transmit_pin);
  vw_setup(2000);	 // Bits per sec
  Serial.println("transmitter ready");

  simpleDisplay("booting   complete");


  // MENU ITEMS

  // brightness
  menuItems[0] = new MenuItem(display,
    brightness,
    "Brightness",
    "",
    224,
    4);

    menuItems[currentMenuItem]->updateDisplay();
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

  buttons = analogRead(BUTTONS_INPUT);
  // Serial.println(buttons);

  if (!buttonStabalizing) {
    buttonStabalizing = true;
    delay(50);
    return;
  }

  if (buttons > 950) {
    menu();
    return;
  }

  if (buttons > 600) {
    menuItems[currentMenuItem]->decrementValue();
    return;
  }

  if (buttons > 400) {
    menuItems[currentMenuItem]->incrementValue();
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

  buttonStabalizing = false;
  delay(buttonDelay);
}

///////////////////////////////////////////////////////////////////
// \ LOOP END /
///////////////////////////////////////////////////////////////////


void menu() {
  // displayState("MENU", "");
  // display.display();
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
