#include <Arduino.h>
#include <VirtualWire.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TCS34725.h>

// RADIO TRANSMITTER
const byte authByteStart = 117;
const byte authByteEnd = 115;

const uint_fast8_t transmit_pin = 12;
const uint_fast16_t buttonDelay = 300;

uint_fast8_t messageID = 0;

const byte colorReadMessage = 0;
const byte colorClearMessage = 1;
const byte brightnessUpMessage = 2;
const byte brightnessDownMessage = 3;
const byte densityUpMessage = 4;
const byte densityDownMessage = 5;

const float touchThreshold = 1.25;

void debounce();
void stealColor();
void clearColor();
void brightnessUp();
void brightnessDown();
void densityUp();
void densityDown();
void sendMessage(byte messageType, byte data);

// COLOR SENSOR
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint8_t readHue();
uint8_t calcHue(float r, float g, float b);

// BUTTON PINS
const int buttons_vcc_pin = 15;
#define BUTTONS_INPUT A0
#define TOUCH_1 17

bool colorSensorOn = true;

void setup()
{
  // SETUP SERIAL CONNECTION FOR LOGGING
  while(!Serial && millis() < 10000);
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

//   // COLOR SENSOR SETUP
//   if (tcs.begin()) {
//     Serial.println("Found color sensor");
//     tcs.setInterrupt(true);
//   } else {
//     Serial.println("No color sensor found!");
//     while (true) {
//       digitalWrite(LED_BUILTIN, LOW);
//       delay(200);
//       digitalWrite(LED_BUILTIN, HIGH);
//       delay(200);
//     }
//   }
}

bool readComplete = false;
bool buttonStabalizing = false;

uint_fast16_t buttons = 0;
uint_fast16_t lastButtons = 0;
uint_fast16_t touch1 = 0;
uint_fast16_t touchAvg = 10000;

void loop() {
  buttons = analogRead(BUTTONS_INPUT);

  if (!buttonStabalizing) {
    buttonStabalizing = true;
    delay(50);
    return;
  }

  if (buttons > 950) {
    colorSensorOn = !colorSensorOn;
    if (colorSensorOn) {
      digitalWrite(LED_BUILTIN, HIGH);
      clearColor();
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
    delay(1000);
    return;
  }

  if (buttons > 775) {
    brightnessDown();
    return;
  }

  if (buttons > 650) {
    brightnessUp();
    return;
  }

  if (buttons > 450) {
    densityUp();
    return;
  }

  if (buttons > 350) {
    densityDown();
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

  // if (touch1 >= touchAvg * touchThreshold && !readComplete) {
  //   stealColor();
  //   readComplete = true;
  // }

  // if (touch1 < touchAvg * touchThreshold) {
  //   readComplete = false;
  // }

  buttonStabalizing = false;
  delay(buttonDelay);
}

void stealColor() {
  byte hue = 0; // readHue();

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

  for(uint_fast8_t i=0; i<5;i++) {
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
