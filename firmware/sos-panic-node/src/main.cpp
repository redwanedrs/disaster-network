#include <Arduino.h>
#include "disaster_radio.h"
#include "disaster_secrets.h"
#include "disaster_config.h"

using namespace Disaster;

Radio radio;
uint32_t sequenceNo = 0;

// Non-blocking Morse state machine: no delay() is used while SOS is playing.
struct MorsePlayer {
  static constexpr uint16_t UNIT_MS = 140;
  static constexpr uint8_t DOT = 1;
  static constexpr uint8_t DASH = 3;

  bool active = false;
  bool outputOn = false;
  uint8_t symbolIndex = 0;
  uint8_t gapStage = 0;
  uint32_t deadline = 0;

  void start() {
    active = true;
    outputOn = false;
    symbolIndex = 0;
    gapStage = 0;
    digitalWrite(SOS_BUZZER_PIN, LOW);
    deadline = millis();
  }

  void update() {
    if (!active || static_cast<int32_t>(millis() - deadline) < 0) return;
    // "... --- ..."; 0 between symbols inside a letter, 1-unit on/off is
    // implemented by turning the buzzer on for symbol length then off for 1 U.
    static const uint8_t symbols[] = { DOT, DOT, DOT, DASH, DASH, DASH, DOT, DOT, DOT };
    static const uint8_t letterEndAfter[] = { 0,0,1,0,0,1,0,0,1 };

    if (!outputOn) {
      if (symbolIndex >= 9) {
        active = false;
        digitalWrite(SOS_BUZZER_PIN, LOW);
        return;
      }
      outputOn = true;
      digitalWrite(SOS_BUZZER_PIN, HIGH);
      deadline = millis() + symbols[symbolIndex] * UNIT_MS;
    } else {
      outputOn = false;
      digitalWrite(SOS_BUZZER_PIN, LOW);
      ++symbolIndex;
      // 1 U between elements; 3 U after a letter. We already consumed no
      // off-time yet, so add 2 extra units at the end of each S/O letter.
      deadline = millis() + ((symbolIndex > 0 && letterEndAfter[symbolIndex - 1]) ? 3 : 1) * UNIT_MS;
    }
  }
};

MorsePlayer morse;
bool lastButton = HIGH;

static void sendSOS() {
  Packet p;
  p.source = NODE_ID;
  p.destination = COMMAND_CENTER_NODE_ID;
  p.type = MessageType::SOS;
  p.sequence = ++sequenceNo;
  const char* text = "SOS PANIC BUTTON";
  p.payloadLen = strlen(text);
  memcpy(p.payload, text, p.payloadLen);
  Serial.println(radio.send(p) ? "SOS TX OK" : "SOS TX FAILED");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Disaster Network / SOS Panic Node ===");

  pinMode(SOS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SOS_BUZZER_PIN, OUTPUT);
  digitalWrite(SOS_BUZZER_PIN, LOW);

  radio.setKey(DISASTER_AES_KEY);
  if (!radio.begin()) {
    Serial.println("ERROR: LoRa initialization failed.");
    while (true) delay(1000);
  }
  Serial.println("Ready. Press the panic button to transmit SOS and sound buzzer.");
}

void loop() {
  const bool button = digitalRead(SOS_BUTTON_PIN);
  if (lastButton == HIGH && button == LOW) {
    sendSOS();
    morse.start();
  }
  lastButton = button;
  morse.update();

  Packet p;
  if (radio.receive(p)) {
    Serial.printf("RX type=%s from=%u\n", messageTypeName(p.type), p.source);
  }
}
