#pragma once

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Disaster Network shared defaults
// -----------------------------------------------------------------------------
// REV-A PIN CONTRACT: these GPIO assignments are frozen for the Disaster Network Rev-A
// firmware/hardware pairing. See hardware/REV-A_PINOUT.md.
//
// The ESP32-S3 has strapping pins GPIO0, GPIO3, GPIO45 and GPIO46. Rev-A
// deliberately avoids them. Legacy assemblies may still have LoRa MISO routed
// elsewhere; follow the legacy upload workaround instead of changing Rev-A.

#ifndef LORA_SCK_PIN
#define LORA_SCK_PIN 12
#endif
#ifndef LORA_MISO_PIN
#define LORA_MISO_PIN 13
#endif
#ifndef LORA_MOSI_PIN
#define LORA_MOSI_PIN 11
#endif
#ifndef LORA_CS_PIN
#define LORA_CS_PIN 10
#endif
#ifndef LORA_RST_PIN
#define LORA_RST_PIN 9
#endif
#ifndef LORA_DIO0_PIN
#define LORA_DIO0_PIN 8
#endif

#ifndef LORA_FREQUENCY_HZ
#define LORA_FREQUENCY_HZ 433E6
#endif

#ifndef LORA_TX_POWER_DBM
#define LORA_TX_POWER_DBM 17
#endif

#ifndef LORA_SPREADING_FACTOR
#define LORA_SPREADING_FACTOR 7
#endif

#ifndef LORA_BANDWIDTH_HZ
#define LORA_BANDWIDTH_HZ 125E3
#endif

#ifndef LORA_CODING_RATE
#define LORA_CODING_RATE 5
#endif

#ifndef LORA_PREAMBLE
#define LORA_PREAMBLE 8
#endif

#ifndef COMMAND_CENTER_NODE_ID
#define COMMAND_CENTER_NODE_ID 1
#endif

#ifndef BROADCAST_NODE_ID
#define BROADCAST_NODE_ID 0
#endif

#ifndef MAX_PAYLOAD_LEN
#define MAX_PAYLOAD_LEN 120
#endif

#ifndef SENSOR_REPORT_PERIOD_MS
#define SENSOR_REPORT_PERIOD_MS 30000UL
#endif

// Optional UART-friendly status LED. Change to -1 to disable.
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 48
#endif

// 4x4 keypad — Rev-A Messaging Node only.
#define KEYPAD_R0 1
#define KEYPAD_R1 2
#define KEYPAD_R2 16
#define KEYPAD_R3 17
#define KEYPAD_C0 4
#define KEYPAD_C1 5
#define KEYPAD_C2 6
#define KEYPAD_C3 7

// SOS node defaults.
#define SOS_BUTTON_PIN 17
#define SOS_BUZZER_PIN 18

// Generic analog sensor input for the field node.
#define FIELD_SENSOR_ADC_PIN 15


// Rev-A guardrails: these application pins must not land on ESP32-S3 strapping
// pins or collide with the fixed LoRa SPI/control bus.
static_assert(LORA_SCK_PIN != 0 && LORA_SCK_PIN != 3 && LORA_SCK_PIN != 45 && LORA_SCK_PIN != 46, "Rev-A LoRa SCK uses a strapping GPIO");
static_assert(LORA_MISO_PIN != 0 && LORA_MISO_PIN != 3 && LORA_MISO_PIN != 45 && LORA_MISO_PIN != 46, "Rev-A LoRa MISO uses a strapping GPIO");
static_assert(LORA_MOSI_PIN != 0 && LORA_MOSI_PIN != 3 && LORA_MOSI_PIN != 45 && LORA_MOSI_PIN != 46, "Rev-A LoRa MOSI uses a strapping GPIO");
static_assert(LORA_CS_PIN  != 0 && LORA_CS_PIN  != 3 && LORA_CS_PIN  != 45 && LORA_CS_PIN  != 46, "Rev-A LoRa CS uses a strapping GPIO");
static_assert(LORA_RST_PIN != 0 && LORA_RST_PIN != 3 && LORA_RST_PIN != 45 && LORA_RST_PIN != 46, "Rev-A LoRa RESET uses a strapping GPIO");
static_assert(LORA_DIO0_PIN != 0 && LORA_DIO0_PIN != 3 && LORA_DIO0_PIN != 45 && LORA_DIO0_PIN != 46, "Rev-A LoRa DIO0 uses a strapping GPIO");
static_assert(LORA_MISO_PIN == 13, "Rev-A pin contract requires LoRa MISO on GPIO13");
static_assert(LORA_MOSI_PIN == 11, "Rev-A pin contract requires LoRa MOSI on GPIO11");
static_assert(LORA_SCK_PIN == 12, "Rev-A pin contract requires LoRa SCK on GPIO12");
static_assert(LORA_CS_PIN == 10, "Rev-A pin contract requires LoRa CS on GPIO10");
static_assert(LORA_RST_PIN == 9, "Rev-A pin contract requires LoRa RESET on GPIO9");
static_assert(LORA_DIO0_PIN == 8, "Rev-A pin contract requires LoRa DIO0 on GPIO8");
