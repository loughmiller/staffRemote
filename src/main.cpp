#include <Arduino.h>
#include <VirtualWire.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>
#include <ArducamSSD1306.h>    // Modification of Adafruit_SSD1306 for ESP8266 compatibility
#include <Adafruit_GFX.h>   // Needs a little change in original Adafruit library (See README.txt file)


// RADIO TRANSMITTER
const byte authByteStart = 117;
const byte authByteEnd = 115;

const uint_fast8_t transmit_pin = 12;
const uint_fast16_t buttonDelay = 300;

uint_fast8_t messageID = 0;

const byte colorReadMessage = 1;
const byte colorClearMessage = 2;
const byte brightnessUpMessage = 3;
const byte brightnessDownMessage = 4;
const byte densityUpMessage = 5;
const byte densityDownMessage = 6;

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

// function defs
void menu();
void up();
void down();
void stealColor();
void clearColor();
void brightnessUp();
void brightnessDown();
void densityUp();
void densityDown();
void sendMessage(byte messageType, byte data);
void displayLineFont3(const uint_fast8_t y, const char *line);
void displayLineFont2(const uint_fast8_t y, const char *line);
void displayState(const char *line1, const char *line2);
void drawGauge(float value);

void setup()
{
  // DISPLAY - SSD1306 Init
  display.begin();  // Switch OLED
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.display();

  // COLOR SENSOR SETUP
  if (tcs.begin()) {
    Serial.println("Found color sensor");
    tcs.setInterrupt(true);
  } else {
    displayState("Color", "Sensor");
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
}

bool readComplete = false;
bool buttonStabalizing = false;

uint_fast16_t buttons = 0;
uint_fast16_t lastButtons = 0;
uint_fast16_t touch1 = 0;
uint_fast16_t touchAvg = 10000;

float gauge = 0.5;

void loop() {
  buttons = analogRead(BUTTONS_INPUT);

  if (!buttonStabalizing) {
    buttonStabalizing = true;
    delay(50);
    return;
  }

  if (buttons > 950) {
    menu();
    return;
  }

  if (buttons > 650) {
    down();
    return;
  }

  if (buttons > 450) {
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

  buttonStabalizing = false;
  delay(buttonDelay);
}

void menu() {
  displayState("MENU", "");
  display.display();
}

void up() {
  displayState("UP", "");
  gauge = min(1, gauge + 0.02);
  drawGauge(gauge);
  display.display();
  brightnessUp();
}

void down() {
  displayState("DOWN", "");
  gauge = max(0, gauge - 0.02);
  drawGauge(gauge);
  display.display();
  brightnessDown();
}

void stealColor() {
  byte hue = readHue();

  // Serial.println("stealColor");
  sendMessage(colorReadMessage, hue);
}

void clearColor() {
  // Serial.println("clearColor");
  sendMessage(colorClearMessage, 0);
}

void brightnessUp() {
  // Serial.println("brightnessUp");
  sendMessage(brightnessUpMessage, 0);
}

void brightnessDown() {
  // Serial.println("brightnessDown");
  sendMessage(brightnessDownMessage, 0);
}

void densityUp() {
  // Serial.println("densityUp");
  sendMessage(densityUpMessage, 0);
}

void densityDown() {
  // Serial.println("densityDown");
  sendMessage(densityDownMessage, 0);
}

void sendMessage(byte messageType, byte data) {
  byte msg[5] = {authByteStart, messageID, messageType, data, authByteEnd};

  Serial.print(messageID);
  Serial.print("\t");
  Serial.print(messageType);
  Serial.print("\t");
  Serial.print(data);
  Serial.println("");

  for(uint_fast8_t i=0; i<3;i++) {
    vw_send((uint8_t *)msg, sizeof(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    delay(25);
  }

  messageID++;
}

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


void displayLineFont3(const uint_fast8_t y, const char *line) {
  const uint_fast8_t pixelWidth = strlen(line) * (font3Width + font3Pad);
  const uint_fast8_t offset = ((128 - pixelWidth)/2) - 2;
  display.setTextSize(3);
  display.setCursor(offset, y);
  // display.print(strlen(line));
  // display.print(" ");
  // display.print(pixelWidth);
  // display.print(" ");
  // display.print(offset);
  display.print(line);
}

void displayLineFont2(const uint_fast8_t y, const char *line) {
  const uint_fast8_t pixelWidth = strlen(line) * (font2Width + font3Pad);
  const uint_fast8_t offset = ((128 - pixelWidth)/2) - 2;
  display.setTextSize(2);
  display.setCursor(offset, y);
  // display.print(strlen(line));
  // display.print(" ");
  // display.print(pixelWidth);
  // display.print(" ");
  // display.print(offset);
  display.print(line);
}

void displayState(const char *line1, const char *line2) {
  display.clearDisplay();
  if (strlen(line1) > 7) {
    displayLineFont2(singleLineY, line1);
  } else if (strlen(line2) < 1) {
    displayLineFont3(singleLineY, line1);
  } else {
    displayLineFont3(line1Y, line1);
    displayLineFont3(line2Y, line2);
  }
}

void drawGauge(float value) {
  uint_fast8_t width = (int)128.0*value;
  display.drawRect(0, 0, 128, 16, 1);
  display.fillRect(0, 0, width, 16, 1);
}