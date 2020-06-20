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

// BUTTON POWER PIN
#define BUTTON_1 20
#define BUTTON_2 17

uint_fast8_t mode = 0;
const uint_fast8_t modes = 3;

void setup()
{
  delay(6000);
  Serial.begin(9600);	// Debugging only
  Serial.println("setup");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);

  // TRANSMITTER SETUP
  vw_set_tx_pin(transmit_pin);
  vw_setup(2000);	 // Bits per sec
  randomSeed(analogRead(14));

  // COLOR SENSOR SETUP
  if (tcs.begin()) {
    // Serial.println("Found color sensor");
    tcs.setInterrupt(true);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100000000);
    Serial.println("No color sensor found!");
  }

  digitalWrite(LED_BUILTIN, HIGH);
}

// uint_fast16_t currentTime;
// uint_fast16_t readTime = 0;
bool readComplete = false;

int button1;
int button2;

void loop()
{
  button1 = digitalRead(BUTTON_1);
  button2 = digitalRead(BUTTON_2);

  Serial.print(mode);
  Serial.print("\t");
  Serial.print(button1);
  Serial.print("\t");
  Serial.println(button2);

  // currentTime = millis();


  if (!button1 && !button2) {
    digitalWrite(LED_BUILTIN, LOW);
    readComplete = false;
    messageID++;
  } else {
    digitalWrite(LED_BUILTIN, HIGH);

    if (button1 && button2) {
      mode = (mode + 1) % modes;
    } else {
      switch (mode) {
        case 0:
          if (button1) { stealColor(); }
          if (button2) { clearColor(); }
        break;
        case 1:
          if (button1) { brightnessUp(); }
          if (button2) { brightnessDown(); }
          messageID++;
        break;
        case 2:
          if (button1) { densityUp(); }
          if (button2) { densityDown(); }
          messageID++;
        break;
      }
    }

    // debounce
    delay(300);
  }

  delay(100);
}

void stealColor() {
  byte hue = 128;
  readComplete = true;
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
