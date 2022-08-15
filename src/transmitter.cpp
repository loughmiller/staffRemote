#include "transmitter.h"

Transmitter::Transmitter(RH_ASK driver,
      const byte authByteStart,
      const byte authByteEnd) {
  this->driver = driver;
  this->authByteStart = authByteStart;
  this->authByteEnd = authByteEnd;

  Serial.println("setup transmitter");
  // vw_set_tx_pin(this->transmit_pin);
  // vw_setup(2000);	 // Bits per sec
  if (!this->driver.init()) {
    Serial.println("transmitter init failed");
  }

  Serial.println("transmitter ready");
}

void Transmitter::sendMessage(byte messageType, byte data) {
  Serial.println("Transmitter::sendMessage");
  if (messageType == 0) return;  // ignore type

  byte msg[5] = {this->authByteStart,
    this->messageID,
    messageType,
    data,
    this->authByteEnd};

  // for (uint8_t i = 0; i < strlen(msg); i++) {
  //   Serial.print(msg[i]);
  //   Serial.print("\t");
  // }
  // Serial.println();

  for(uint_fast8_t i=0; i<3;i++) {
    driver.send((byte *)msg, strlen(msg));
    driver.waitPacketSent();
    delay(50);
  }

  this->messageID++;
}

void Transmitter::sendSync(byte messageType, uint32_t sync) {
  Serial.println("Transmitter::sendSync");
  byte msg[8] = {this->authByteStart,
    this->messageID,
    messageType,
    (byte)(sync >> 24),
    (byte)(sync >> 16),
    (byte)(sync >> 8),
    (byte)(sync),
    this->authByteEnd};

  Serial.print(this->messageID);
  Serial.print("\t");
  Serial.print(messageType);
  Serial.print("\t");
  Serial.print(sync);
  Serial.println("");

  driver.send((uint8_t *)msg, sizeof(msg));
  driver.waitPacketSent();

  this->messageID++;
}