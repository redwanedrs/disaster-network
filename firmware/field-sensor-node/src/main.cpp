#include <Arduino.h>
#include "disaster_radio.h"
#include "disaster_secrets.h"
#include "disaster_config.h"
#include "cansat_stub.h"

using namespace Disaster;

Radio radio;
uint32_t sequenceNo = 0;
uint32_t lastReportMs = 0;

static void sendText(MessageType type, const String& text) {
  Packet p;
  p.source = NODE_ID;
  p.destination = COMMAND_CENTER_NODE_ID;
  p.type = type;
  p.sequence = ++sequenceNo;
  p.payloadLen = min<size_t>(text.length(), MAX_PAYLOAD_LEN);
  memcpy(p.payload, text.c_str(), p.payloadLen);
  if (!radio.send(p)) Serial.println("TX failed");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Disaster Network / Field Sensor Node ===");

  pinMode(FIELD_SENSOR_ADC_PIN, INPUT);
  radio.setKey(DISASTER_AES_KEY);
  if (!radio.begin()) {
    Serial.println("ERROR: LoRa initialization failed.");
    while (true) delay(1000);
  }

  Serial.println("LoRa ready. Reporting generic ADC sensor data every 30 s.");
  sendText(MessageType::STATUS, "FIELD ONLINE");
}

void loop() {
  Packet incoming;
  if (radio.receive(incoming)) {
    Serial.printf("RX from %u: type=%s payload="", incoming.source, messageTypeName(incoming.type));
    for (uint8_t i = 0; i < incoming.payloadLen; ++i) Serial.write(incoming.payload[i]);
    Serial.println(""");
  }

  if (millis() - lastReportMs >= SENSOR_REPORT_PERIOD_MS) {
    lastReportMs = millis();
    uint16_t raw = analogRead(FIELD_SENSOR_ADC_PIN);
    char msg[64];
    snprintf(msg, sizeof(msg), "ADC=%u uptime_s=%lu", raw,
             static_cast<unsigned long>(millis() / 1000));
    sendText(MessageType::SENSOR, msg);
    Serial.printf("TX: %s\n", msg);
  }
}
