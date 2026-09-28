# Disaster Network Build Guide — English

## 1. Before assembly

This guide uses the frozen **Disaster Network Rev-A pin contract**. The canonical net/GPIO assignment is in `hardware/REV-A_PINOUT.md`; the physical PCB schematic/layout must match it before fabrication or rework.

The default LoRa pins are SCK=12, MISO=13, MOSI=11, CS=10, RESET=9, DIO0=8. These are deliberately separate from ESP32-S3 strapping pins. Espressif lists GPIO0, GPIO3, GPIO45 and GPIO46 as strapping pins. https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html

### Power safety

Use 3.3 V logic. Confirm your particular SX1278 carrier's VCC requirement and regulator. A radio that transmits unreliably or causes resets is often a power-supply problem; use short ground/power wiring and adequate bulk decoupling.

## 2. Software setup

Install PlatformIO. From the repo root:

```bash
cp firmware/common/include/disaster_secrets.h.example firmware/common/include/disaster_secrets.h
```

Change the placeholder AES key before any real deployment. Keep the real `disaster_secrets.h` private; it is git-ignored.

Build targets:

```bash
pio run -e command-center
pio run -e field-sensor-node
pio run -e sos-panic-node
pio run -e messaging-node
```

The repo uses PlatformIO's `esp32-s3-devkitc-1` board target. https://docs.platformio.org/en/latest/boards/espressif32/esp32-s3-devkitc-1.html

## 3. Critical MISO/strapping-pin upload workaround

Some already-built project assemblies route SX1278 MISO to an ESP32-S3 strapping pin. That can interfere with boot/download behavior.

For those boards:

1. Power off.
2. Electrically isolate or unplug **LoRa MISO only**.
3. Enter normal ESP32-S3 download mode and upload firmware.
4. Disconnect USB/power before reconnecting the radio MISO line.
5. Reconnect MISO and reset.

Do not remove or alter a board's boot resistor without checking the exact board schematic. ESP32-S3 boot configuration is sampled on strapping pins during reset. https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html

The source comment at `firmware/common/include/disaster_config.h` and `disaster_radio.cpp` calls out this project-specific history.

## 4. Command Center node

### Wiring

Connect the SX1278 as described in `docs/wiring-diagrams/overview.md`.

The baseline firmware uses the **USB serial monitor as the display**, keeping the hardware BOM minimal. An OLED/TFT can be added later without changing the radio protocol.

### Flash

```bash
pio run -e command-center -t upload
pio device monitor -b 115200
```

Expected output:

```text
=== Disaster Network / Command Center ===
LoRa ready. Listening for authenticated packets...
Command Center: listening...
```

### First-boot test

1. Keep the Command Center powered.
2. Flash a Field, SOS, or Messaging node with the same AES key.
3. Trigger one packet.
4. Confirm the Command Center prints source ID, destination ID, message type, sequence number and RF metrics.
5. An SOS packet must print `!!! EMERGENCY SOS ALERT !!!`.

## 5. Field Sensor node

### Wiring

SX1278 plus a generic analog sensor/test potentiometer:

```text
Sensor signal -> GPIO15
Sensor GND    -> GND
Sensor VCC    -> appropriate supply
```

The firmware intentionally does not pretend to know the final sensor part. Replace the ADC conversion with the production sensor driver later.

### Flash

```bash
pio run -e field-sensor-node -t upload
pio device monitor -b 115200
```

### First-boot test

Expected:

```text
=== Disaster Network / Field Sensor Node ===
LoRa ready. Reporting generic ADC sensor data every 30 s.
TX: ADC=... uptime_s=...
```

The node sends an initial `FIELD ONLINE` message and then one generic sensor report every 30 seconds.

## 6. SOS Panic node

### Wiring

Button:

```text
GPIO17 ---- momentary push button ---- GND
```

Buzzer:

```text
GPIO18 ---- buzzer driver / active buzzer ---- GND
```

If the buzzer load is more than a GPIO should drive, use a transistor/MOSFET stage.

### Flash

```bash
pio run -e sos-panic-node -t upload
pio device monitor -b 115200
```

### First-boot test

1. Wait for `Ready. Press the panic button...`.
2. Press the button once.
3. Confirm the serial output says `SOS TX OK`.
4. Confirm the buzzer produces `... --- ...`.
5. Confirm the Command Center prints the SOS alert.

The Morse player uses `millis()` deadlines and a small state machine; it does not call `delay()` while generating the SOS pattern.

## 7. Messaging node

### Wiring

```text
Keypad row 0 -> GPIO1
Keypad row 1 -> GPIO2
Keypad row 2 -> GPIO16
Keypad row 3 -> GPIO17
Keypad col 0 -> GPIO4
Keypad col 1 -> GPIO5
Keypad col 2 -> GPIO6
Keypad col 3 -> GPIO7
```

### Flash

```bash
pio run -e messaging-node -t upload
pio device monitor -b 115200
```

### First-boot test

Preset keys:

```text
1 STATUS OK       5 TRAPPED
2 NEED WATER      6 FIRE
3 NEED MEDICAL    7 INJURED
4 EVACUATE        8 LOCATION SENT
```

Press `B` to enter custom mode:

```text
2-9 = multi-tap letters/numbers
0   = space
*   = backspace
A   = clear
#   = send
D   = cancel
```

For multi-tap, press a key repeatedly within 800 ms to cycle its letters. Pause, then choose the next key.

## 8. Network interoperability test

Use one Command Center and one sender first.

1. Confirm all nodes use identical LoRa frequency/modem parameters.
2. Confirm all nodes use the same 16-byte AES key.
3. Send a preset message.
4. Trigger SOS.
5. Change the key on one node and confirm that its packets are rejected by the Command Center.

The radio library itself does **not** provide encryption, so application-layer encryption in the common library is mandatory for this project. https://github.com/sandeepmistry/arduino-LoRa#faq

## 9. Production-readiness checklist

- Freeze exact board and PCB revision.
- Replace demo/shared AES key and implement secure provisioning.
- Add replay-window logic and acknowledgements/retries.
- Validate legal 433 MHz frequency, power, bandwidth, and duty-cycle requirements for the deployment country.
- Validate antenna matching and enclosure/RF layout.
- Validate battery capacity, brownout behavior, charging, and thermal margins.
- Decide the command-center physical display.
- Replace the generic field ADC with the actual sensor interface.
- Implement the CanSat interface after its electrical/protocol contract is frozen.
