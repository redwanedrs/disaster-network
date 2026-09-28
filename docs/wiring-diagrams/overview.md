# Wiring Overview

## All nodes: ESP32-S3 ↔ SX1278

```text
ESP32-S3                         SX1278
---------                        ------
3V3 ---------------------------- VCC
GND ---------------------------- GND
GPIO12 ------------------------- SCK
GPIO13 ------------------------- MISO
GPIO11 ------------------------- MOSI
GPIO10 ------------------------- NSS / CS
GPIO9  ------------------------- RESET
GPIO8  ------------------------- DIO0
```

> Existing PCB assemblies that put LoRa MISO on an ESP32-S3 strapping pin must isolate that connection during upload. See `docs/pinout-notes.md`.

## SOS node

```text
GPIO17 ---- momentary switch ---- GND
          (INPUT_PULLUP)

GPIO18 ---- active buzzer + ---- appropriate driver/power stage
                             +-- GND
```

For a buzzer that exceeds a GPIO's safe source/sink current, use a transistor/MOSFET driver rather than powering it directly from GPIO18.

## Messaging node

```text
4x4 keypad
Row 0 -> GPIO1       Col 0 -> GPIO4
Row 1 -> GPIO2       Col 1 -> GPIO5
Row 2 -> GPIO16      Col 2 -> GPIO6
Row 3 -> GPIO17      Col 3 -> GPIO7
```

The firmware drives one row LOW at a time and reads columns with internal pull-ups.

## Field sensor node

```text
Analog sensor output/test potentiometer -> GPIO15
Sensor GND                              -> GND
Sensor VCC                              -> appropriate supply
```

Replace the generic analog example with the actual sensor interface once the sensor part is selected.
