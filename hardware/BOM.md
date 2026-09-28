# Disaster Network Hardware BOM

This BOM is intentionally conservative. Confirm the exact module carrier, connector type, battery/power design, and PCB revision before ordering a batch.

| Item | Qty | Used by | Notes |
|---|---:|---|---|
| ESP32-S3-DevKitC-1 (compatible N8/N8R2/N8R8 board) | 4+ | All nodes | Exact board revision is a TODO to lock down. |
| SX1278 433 MHz LoRa module | 4+ | All nodes | 3.3 V logic/module supply required. |
| 433 MHz antenna matched to module | 4+ | All nodes | Use an appropriate 433 MHz antenna/connector. |
| 3.3 V regulator / suitable power subsystem | 4+ | All nodes | Size for ESP32 + LoRa transmit current. |
| Momentary push button | 1 | SOS | Active-low on GPIO17. |
| Active buzzer | 1 | SOS | GPIO18; verify current requirements. |
| 4×4 matrix keypad | 1 | Messaging | Rows GPIO1,2,16,17; columns GPIO4-7. |
| Analog sensor / test potentiometer | 1+ | Field | Demo input on GPIO15; replace with project sensor. |
| USB cable | 4+ | All nodes | Use a cable that supports data. |
| Breadboard/jumper wires | As needed | Prototyping | Keep LoRa wiring short. |
| Optional OLED SSD1306 | 1 | Command Center | Current firmware uses serial output; OLED is a future display option. |

## Important electrical notes

- **3.3 V only at ESP32-S3 and SX1278 logic interfaces.** Do not connect a 5 V GPIO signal directly to the radio.
- Keep LoRa ground and ESP32 ground common.
- Put decoupling close to the LoRa module and ESP32 power entry.
- Check the SX1278 carrier's regulator and level-shifting circuitry; many inexpensive boards differ.
- ESP32-S3 GPIO strapping pins are GPIO0, GPIO3, GPIO45 and GPIO46. Avoid connecting external pull resistors/peripherals to these pins unless the boot behavior is intentionally designed. See `docs/pinout-notes.md`.
