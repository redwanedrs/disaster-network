#include <Arduino.h>
#include "disaster_radio.h"
#include "disaster_secrets.h"

using namespace Disaster;

Radio radio;
uint32_t lastStatusMs = 0;

static void printPacket(const Packet& p, int rssi, float snr) {
  Serial.printf("RX src=%u dst=%u type=%s seq=%lu RSSI=%d SNR=%.1f payload=\"",
                p.source, p.destination, messageTypeName(p.type),
                static_cast<unsigned long>(p.sequence), rssi, snr);
  for (uint8_t i = 0; i < p.payloadLen; ++i) {
    char c = static_cast<char>(p.payload[i]);
    Serial.print((c >= 32 && c <= 126) ? c : '.');
  }
  Serial.println("\"");

  if (p.type == MessageType::SOS) {
    Serial.println("!!! EMERGENCY SOS ALERT !!!");
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Disaster Network / Command Center ===");

  radio.setKey(DISASTER_AES_KEY);
  if (!radio.begin()) {
    Serial.println("ERROR: LoRa initialization failed.");
    while (true) delay(1000);
  }

#if STATUS_LED_PIN >= 0
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH);
#endif

  Serial.println("LoRa ready. Listening for authenticated packets...");
}

void loop() {
  Packet p;
  int rssi = 0;
  float snr = 0;
  if (radio.receive(p, &rssi, &snr)) {
    printPacket(p, rssi, snr);
#if STATUS_LED_PIN >= 0
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
#endif
  }

  if (millis() - lastStatusMs >= 10000) {
    lastStatusMs = millis();
    Serial.println("Command Center: listening...");
  }
}
