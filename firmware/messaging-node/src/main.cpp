#include <Arduino.h>
#include "disaster_radio.h"
#include "disaster_secrets.h"
#include "disaster_config.h"

using namespace Disaster;

Radio radio;
uint32_t sequenceNo = 0;

const char KEYMAP[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
const uint8_t ROWS[4] = {KEYPAD_R0, KEYPAD_R1, KEYPAD_R2, KEYPAD_R3};
const uint8_t COLS[4] = {KEYPAD_C0, KEYPAD_C1, KEYPAD_C2, KEYPAD_C3};

struct KeyEvent { char key = 0; bool pressed = false; };
String composeBuffer;
char lastMultiKey = 0;
uint8_t multiIndex = 0;
uint32_t lastMultiMs = 0;
bool composeMode = false;

const char* multiChars(char key) {
  switch (key) {
    case '2': return "ABC2"; case '3': return "DEF3";
    case '4': return "GHI4"; case '5': return "JKL5";
    case '6': return "MNO6"; case '7': return "PQRS7";
    case '8': return "TUV8"; case '9': return "WXYZ9";
    default: return nullptr;
  }
}

KeyEvent scanKeypad() {
  static char previous = 0;
  static uint32_t debounceUntil = 0;
  char found = 0;

  for (uint8_t r = 0; r < 4; ++r) {
    digitalWrite(ROWS[r], LOW);
    for (uint8_t c = 0; c < 4; ++c) {
      if (digitalRead(COLS[c]) == LOW) found = KEYMAP[r][c];
    }
    digitalWrite(ROWS[r], HIGH);
  }

  if (millis() < debounceUntil) return {};
  if (found != 0 && found != previous) {
    previous = found;
    debounceUntil = millis() + 45;
    return {found, true};
  }
  if (found == 0) previous = 0;
  return {};
}

void sendMessage(const String& text) {
  if (text.length() == 0) return;
  Packet p;
  p.source = NODE_ID;
  p.destination = COMMAND_CENTER_NODE_ID;
  p.type = MessageType::TEXT;
  p.sequence = ++sequenceNo;
  p.payloadLen = min<size_t>(text.length(), MAX_PAYLOAD_LEN);
  memcpy(p.payload, text.c_str(), p.payloadLen);
  Serial.printf("TX %s: %s\n", radio.send(p) ? "OK" : "FAILED", text.c_str());
}

void handleKey(char key) {
  if (!composeMode) {
    static const char* presets[8] = {
      "STATUS OK", "NEED WATER", "NEED MEDICAL", "EVACUATE",
      "TRAPPED", "FIRE", "INJURED", "LOCATION SENT"
    };
    if (key >= '1' && key <= '8') {
      sendMessage(presets[key - '1']);
    } else if (key == 'B') {
      composeMode = true;
      composeBuffer = "";
      lastMultiKey = 0;
      Serial.println("CUSTOM MODE: 2-9=letters, 0=space, *=backspace, #=send, A=clear, D=cancel");
    }
    return;
  }

  if (key == '#') {
    sendMessage(composeBuffer);
    composeMode = false;
    composeBuffer = "";
    lastMultiKey = 0;
    return;
  }
  if (key == 'D') {
    composeMode = false;
    composeBuffer = "";
    Serial.println("CUSTOM CANCELED");
    return;
  }
  if (key == 'A') {
    composeBuffer = "";
    lastMultiKey = 0;
    Serial.println("CUSTOM CLEARED");
    return;
  }
  if (key == '*') {
    if (composeBuffer.length()) composeBuffer.remove(composeBuffer.length() - 1);
    lastMultiKey = 0;
    Serial.printf("CUSTOM: %s\n", composeBuffer.c_str());
    return;
  }
  if (key == '0') {
    composeBuffer += ' ';
    lastMultiKey = 0;
    return;
  }

  const char* chars = multiChars(key);
  if (chars) {
    const uint32_t now = millis();
    if (key == lastMultiKey && now - lastMultiMs < 800 && composeBuffer.length()) {
      multiIndex = (multiIndex + 1) % strlen(chars);
      composeBuffer.setCharAt(composeBuffer.length() - 1, chars[multiIndex]);
    } else {
      multiIndex = 0;
      composeBuffer += chars[multiIndex];
    }
    lastMultiKey = key;
    lastMultiMs = now;
    Serial.printf("CUSTOM: %s\n", composeBuffer.c_str());
  }

  if (composeBuffer.length() >= MAX_PAYLOAD_LEN) composeBuffer.remove(MAX_PAYLOAD_LEN);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Disaster Network / Messaging Node ===");

  for (uint8_t r = 0; r < 4; ++r) {
    pinMode(ROWS[r], OUTPUT);
    digitalWrite(ROWS[r], HIGH);
  }
  for (uint8_t c = 0; c < 4; ++c) pinMode(COLS[c], INPUT_PULLUP);

  radio.setKey(DISASTER_AES_KEY);
  if (!radio.begin()) {
    Serial.println("ERROR: LoRa initialization failed.");
    while (true) delay(1000);
  }
  Serial.println("Ready. Keys 1-8 send presets; B starts custom text mode.");
}

void loop() {
  KeyEvent e = scanKeypad();
  if (e.pressed) handleKey(e.key);

  Packet p;
  if (radio.receive(p)) {
    Serial.printf("RX from %u type=%s\n", p.source, messageTypeName(p.type));
  }
}
