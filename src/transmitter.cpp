#include "transmitter.h"

Transmitter::Transmitter(const uint_fast8_t transmit_pin,
      const byte authByteStart,
      const byte authByteEnd) {
  this->transmit_pin = transmit_pin;
  this->authByteStart = authByteStart;
  this->authByteEnd = authByteEnd;

  Serial.println("setup transmitter");
  vw_set_tx_pin(this->transmit_pin);
  vw_setup(2000);	 // Bits per sec
  Serial.println("transmitter ready");
}

void Transmitter::sendMessage(byte messageType, byte data) {
  byte msg[5] = {this->authByteStart,
    this->messageID,
    messageType,
    data,
    this->authByteEnd};

  Serial.print(this->messageID);
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

  this->messageID++;
}