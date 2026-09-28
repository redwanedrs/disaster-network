# Disaster Network Rev-A — Exact Pinout Contract

This file is the **canonical logical/net pin contract** for the Rev-A Disaster Network hardware/firmware pairing.

> **Important:** this freezes the firmware interface, not the physical copper routing of an already-manufactured PCB. A board schematic/layout must match these nets before fabrication or rework is signed off.

## Board assumptions

- MCU: ESP32-S3 DevKitC-1-compatible module/carrier.
- Radio: SX1278 433 MHz SPI module.
- Logic: 3.3 V.
- GPIOs reserved by this contract are not to be reassigned per-node without changing the Rev-A pinout document and firmware configuration together.
- ESP32-S3 strapping pins GPIO0, GPIO3, GPIO45, GPIO46 are intentionally unused for application I/O.
- GPIO26–32 are reserved/avoided because they may be connected to flash/PSRAM on ESP32-S3 module variants.
- GPIO19/20 are avoided because USB/JTAG use can conflict with application peripherals.

## J1 — MCU power/programming

| J1 pin / net | ESP32-S3 | Direction | Purpose |
|---|---:|---|---|
| 1 | 3V3 | PWR | regulated 3.3 V rail |
| 2 | GND | PWR | common ground |
| 3 | USB | — | USB programming/serial via carrier board |

## J2 — SX1278 LoRa

| Net | ESP32-S3 GPIO | SX1278 pin | Notes |
|---|---:|---|---|
| LORA_SCK | GPIO12 | SCK | SPI clock |
| LORA_MISO | GPIO13 | MISO | SPI data from radio |
| LORA_MOSI | GPIO11 | MOSI | SPI data to radio |
| LORA_CS | GPIO10 | NSS/CS | chip select |
| LORA_RST | GPIO9 | RESET | radio reset |
| LORA_DIO0 | GPIO8 | DIO0 | RX/TX interrupt/status |
| LORA_3V3 | 3V3 | VCC | verify SX1278 carrier regulator requirements |
| LORA_GND | GND | GND | common ground |

### MISO upload rule

**Rev-A canonical MISO is GPIO13.** GPIO13 is not an ESP32-S3 strapping pin. If an older assembly has MISO routed elsewhere, do not silently reinterpret that board as Rev-A. Use the documented isolate-MISO upload workaround for that legacy board and label it as a legacy revision.

## J3 — Field sensor node

| Net | GPIO | Direction | Purpose |
|---|---:|---|---|
| FIELD_SENSOR_ADC | GPIO15 | IN | generic analog sensor/test input |
| FIELD_GND | GND | PWR | sensor ground |
| FIELD_VCC | 3V3 or regulated sensor rail | PWR | sensor supply, per sensor datasheet |

## J4 — SOS panic node

| Net | GPIO | Direction | Purpose |
|---|---:|---|---|
| SOS_BUTTON | GPIO17 | IN | momentary button to GND; internal pull-up |
| SOS_BUZZER_CTRL | GPIO18 | OUT | active-buzzer/transistor-driver control |
| SOS_GND | GND | PWR | common ground |

The buzzer output is a **control signal**, not a promise that GPIO18 directly drives a high-current buzzer. Use a transistor/MOSFET stage when required by the buzzer load.

## J5 — Messaging node 4×4 keypad

| Keypad signal | GPIO | Direction | Electrical behavior |
|---|---:|---|---|
| ROW0 | GPIO1 | OUT | one row LOW at a time |
| ROW1 | GPIO2 | OUT | one row LOW at a time |
| ROW2 | GPIO16 | OUT | one row LOW at a time |
| ROW3 | GPIO17 | OUT | one row LOW at a time |
| COL0 | GPIO4 | IN | internal pull-up |
| COL1 | GPIO5 | IN | internal pull-up |
| COL2 | GPIO6 | IN | internal pull-up |
| COL3 | GPIO7 | IN | internal pull-up |

GPIO17 is reused between the SOS button and Messaging keypad because these are **different node assemblies**; they are never populated as simultaneous functions on one node.

## J6 — Optional status LED

| Net | GPIO | Purpose |
|---|---:|---|
| STATUS_LED | GPIO48 | optional status LED |

GPIO48 is optional. If the selected ESP32-S3 carrier does not expose it or already assigns it to another onboard function, populate no external LED and define `STATUS_LED_PIN=-1` in that hardware variant.

## Rev-A conflict matrix

| Resource | Rev-A GPIO(s) | Collision check |
|---|---|---|
| LoRa SPI | 8, 9, 10, 11, 12, 13 | reserved on all nodes |
| Field ADC | 15 | field node only |
| SOS button/buzzer | 17, 18 | SOS node only |
| Messaging keypad | 1, 2, 4, 5, 6, 7, 16, 17 | messaging node only |
| Status LED | 48 | optional |
| ESP32-S3 strapping | 0, 3, 45, 46 | **unused** |
| Flash/PSRAM sensitive | 26–32 | **unused** |
| USB/JTAG-sensitive | 19, 20 | **unused** |

## Bring-up acceptance test

Before calling a PCB Rev-A compatible:

1. With the radio disconnected, the ESP32-S3 must enter download mode and flash normally.
2. With the radio connected, `LoRa.begin()` must succeed using J2.
3. Oscilloscope or logic-analyzer check must show GPIO12=SCK, GPIO11=MOSI, GPIO13=MISO, GPIO10=CS during a packet exchange.
4. Verify GPIO17/GPIO18 operation on the SOS assembly.
5. Verify all 16 keypad positions map according to the firmware keymap on the Messaging assembly.
6. Record the actual PCB revision/silkscreen and keep it with the build log.
