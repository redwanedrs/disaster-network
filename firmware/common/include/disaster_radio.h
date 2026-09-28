#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "disaster_config.h"
#include "disaster_protocol.h"

namespace Disaster {

class Radio {
 public:
  bool begin();
  bool send(const Packet& packet);
  bool receive(Packet& packet, int* rssi = nullptr, float* snr = nullptr);
  void setKey(const uint8_t key[16]);

 private:
  uint8_t key_[16] = {};
};

} // namespace Disaster
