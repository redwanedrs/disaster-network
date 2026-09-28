# Pinout and Hardware Notes

## Disaster Network Rev-A ESP32-S3 mapping

The firmware targets a generic ESP32-S3-DevKitC-1-compatible board. The following mapping is a project default, not a guarantee for every ESP32-S3 carrier.

### LoRa / SX1278 (all nodes)

| Function | ESP32-S3 GPIO | SX1278 pin |
|---|---:|---|
| SCK | 12 | SCK |
| MISO | 13 | MISO |
| MOSI | 11 | MOSI |
| Chip Select / NSS | 10 | NSS/CS |
| Reset | 9 | RESET |
| DIO0 | 8 | DIO0 |
| 3V3 | 3V3 | VCC |
| Ground | GND | GND |

The firmware explicitly configures the SPI pins before calling `LoRa.begin()`. The Arduino LoRa library requires custom NSS/reset/DIO0 pins to be configured before `begin()`, and supports a custom SPI interface/frequency. https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md

### Node-specific pins

| Node | Function | GPIO |
|---|---|---:|
| Field | Generic analog sensor | 15 |
| SOS | Panic button | 17 |
| SOS | Buzzer | 18 |
| Messaging | Keypad row 0..3 | 1,2,3,4 |
| Messaging | Keypad col 0..3 | 5,6,7,14 |
| All | Optional status LED | 48 |

## ESP32-S3 strapping-pin issue

ESP32-S3 strapping pins are GPIO0, GPIO3, GPIO45 and GPIO46. Boot mode is sampled through strapping-pin state during reset; after reset these pins can operate as normal GPIOs. https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html

The current default LoRa MISO is GPIO13 specifically to avoid these strapping pins. However, the project history reports a **board-specific LoRa MISO collision with an ESP32-S3 strapping pin**. For any existing PCB/assembly wired that way:

1. Disconnect or electrically isolate LoRa MISO during firmware upload.
2. Enter the board's normal download mode and upload firmware.
3. Reconnect MISO after the upload completes and reset the board.
4. Preferably revise the PCB or add a properly designed isolation/series-resistor solution for a production revision.

Do not change a boot-critical resistor blindly. Verify the exact ESP32-S3 module/board schematic first.

## Avoiding ESP32-S3 resource conflicts

GPIO26-32 are commonly associated with SPI flash/PSRAM, and additional GPIO restrictions depend on the ESP32-S3 module package. GPIO19/20 are used by USB-JTAG by default. The project defaults therefore stay on GPIO1-18 except for GPIO48 as an optional LED. https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html

## Before production

- Verify the physical PCB schematic/layout matches the frozen Rev-A pin contract.
- Freeze exact SX1278 carrier/module part number.
- Validate 433 MHz legal operating parameters for the deployment country.
- Replace demo AES key and document a secure provisioning process.
- Decide whether the command center gets an OLED/TFT/other physical display.
