#include "disaster_radio.h"
#include "disaster_secrets.h"
#include <string.h>

namespace Disaster {

void Radio::setKey(const uint8_t key[16]) {
  memcpy(key_, key, sizeof(key_));
}

bool Radio::begin() {
  // Custom SPI pin assignment MUST happen before LoRa.begin().
  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_CS_PIN);
  LoRa.setSPI(SPI);
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  LoRa.setSPIFrequency(8E6);

  if (!LoRa.begin(LORA_FREQUENCY_HZ)) return false;
  LoRa.setTxPower(LORA_TX_POWER_DBM);
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH_HZ);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.setPreambleLength(LORA_PREAMBLE);
  LoRa.enableCrc();
  LoRa.receive();
  return true;
}

bool Radio::send(const Packet& packet) {
  uint8_t frame[MAX_PACKET_LEN];
  const size_t frameLen = encodePacket(packet, frame, sizeof(frame), key_);
  if (frameLen == 0) return false;

  LoRa.idle();
  LoRa.beginPacket();
  LoRa.write(frame, frameLen);
  int result = LoRa.endPacket();
  LoRa.receive();
  return result == 1;
}

bool Radio::receive(Packet& packet, int* rssi, float* snr) {
  int packetSize = LoRa.parsePacket();
  if (packetSize <= 0) return false;

  uint8_t frame[MAX_PACKET_LEN];
  size_t len = 0;
  while (LoRa.available() && len < sizeof(frame)) {
    frame[len++] = static_cast<uint8_t>(LoRa.read());
  }

  if (rssi) *rssi = LoRa.packetRssi();
  if (snr) *snr = LoRa.packetSnr();
  return decodePacket(frame, len, packet, key_);
}

} // namespace Disaster
