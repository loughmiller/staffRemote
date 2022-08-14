#include <Arduino.h>
#include <VirtualWire.h>

#ifndef TRANSMITTER_H
#define TRANSMITTER_H

class Transmitter {
  protected :
    byte authByteStart;
    byte authByteEnd;
    uint_fast8_t transmit_pin;
    uint8_t messageID = 0;


  public :
    Transmitter(const uint_fast8_t transmit_pin,
      const byte authByteStart,
      const byte authByteEnd);
    void sendMessage(byte messageType, byte data);
    void sendSync(byte messageType, uint32_t sync);
};

#endif /* TRANSMITTER_H */
