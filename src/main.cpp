#include <Arduino.h>
#include <VirtualWire.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>

// RADIO TRANSMITTER
const int transmit_pin = 12;
byte messageID = 0;

const byte colorReadMessage = 0;
const byte colorClearMessage = 1;
const byte brightnessUpMessage = 2;
const byte brightnessDownMessage = 3;
const byte densityUpMessage = 4;
const byte densityDownMessage = 5;

const uint_fast16_t touchThreshold = 1300;

void stealColor();
void clearColor();
void brightnessUp();
void brightnessDown();
void densityUp();
void densityDown();
void sendMessage(byte messageID, byte messageType, byte data);

// COLOR SENSOR
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint8_t readHue();
uint8_t calcHue(float r, float g, float b);

// BUTTON PINS
#define BUTTON_1 7
#define BUTTON_2 8
#define BUTTON_3 9
#define BUTTON_4 14
#define BUTTON_5 15

// TOUCH BUTTONS
#define TOUCH_1 0

bool colorSensorOn = true;

void setup()
{
  delay(6000);
  Serial.begin(9600);	// Debugging only
  Serial.println("setup");

  pinMode(BUTTON_1, INPUT_PULLUP);  // Use pullup where buttons connect to ground on press
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  pinMode(BUTTON_5, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  // TRANSMITTER SETUP
  Serial.println("setup transmitter");
  vw_set_tx_pin(transmit_pin);
  vw_setup(2000);	 // Bits per sec
  Serial.println("transmitter ready");

  // COLOR SENSOR SETUP
  if (tcs.begin()) {
    Serial.println("Found color sensor");
    tcs.setInterrupt(true);
  } else {
    Serial.println("No color sensor found!");
    while (true) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
    }
  }
}

bool readComplete = false;

int button1 = 0;
int button2 = 0;
int button3 = 0;
int button4 = 0;
int button5 = 0;
int touch1 = 0;

void loop()
{
  button1 = !digitalRead(BUTTON_1);
  button2 = !digitalRead(BUTTON_2);
  button3 = !digitalRead(BUTTON_3);
  button4 = !digitalRead(BUTTON_4);
  button5 = !digitalRead(BUTTON_5);

  // Serial.print(mode);
  // Serial.print("\t");
  Serial.print(button1);
  Serial.print("\t");
  Serial.print(button2);
  Serial.print("\t");
  Serial.print(button3);
  Serial.print("\t");
  Serial.print(button4);
  Serial.print("\t");
  Serial.print(button5);
  Serial.print("\t");
  Serial.print(touch1);

  Serial.println("");
  // currentTime = millis();

  if (button1) {
    brightnessUp();
  }

  if (button2) {
    brightnessDown();
  }

  if (button3) {
    densityUp();
  }

  if (button4) {
    densityUp();
  }

  if (button5) {
    colorSensorOn = !colorSensorOn;
    if (!colorSensorOn) {
      clearColor();
    }
  }

  if (colorSensorOn) {
    digitalWrite(LED_BUILTIN, HIGH);
    touch1 = touchRead(TOUCH_1);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
    touch1 = 0;
  }

  if (touch1 >= touchThreshold && !readComplete) {
    stealColor();
    readComplete = true;
  }

  if (touch1 < touchThreshold) {
    readComplete = false;
  }

  // debounce
  if (button1 || button2 || button3 || button4 || button5) {
    delay(300);
  }

  delay(50);
}

void stealColor() {
  byte hue = readHue();
  sendMessage(messageID, colorReadMessage, hue);
}

void clearColor() {
  sendMessage(messageID, colorClearMessage, 0);
}

void brightnessUp() {
  sendMessage(messageID, brightnessUpMessage, 0);
}

void brightnessDown() {
  sendMessage(messageID, brightnessDownMessage, 0);
}

void densityUp() {
  sendMessage(messageID, densityUpMessage, 0);
}

void densityDown() {
  sendMessage(messageID, densityDownMessage, 0);
}

void sendMessage(byte messageID, byte messageType, byte data) {
    byte msg[3] = {messageID, messageType, data};

    vw_send((uint8_t *)msg, 3);
    vw_wait_tx(); // Wait until the whole message is gone
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
