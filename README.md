# BREATHE — VOC Detector

An air-quality monitor that reads volatile organic compound (VOC) levels with an
MQ-138 sensor, shows them on an LCD, sounds a buzzer above a threshold, and streams
the readings over Bluetooth Low Energy (BLE) to a web page.

## How it works

```
MQ-138 sensor ──> Arduino Uno ──(UART)──> ESP32 ──(BLE)──> Web page (Web Bluetooth)
                     │
                     ├─> 16x2 LCD  (live value)
                     └─> Buzzer    (alert above threshold)
```

- **Arduino Uno** (`arduino_voc/arduino_voc.ino`) reads the MQ-138 on `A0`, prints the
  value to a 16x2 LCD, drives a buzzer when the reading exceeds the threshold, and sends
  each reading to the ESP32 over a SoftwareSerial link (one value per line).
- **ESP32** (`esp32_voc/esp32_voc.ino`) receives readings on `Serial2` and re-publishes
  them as a BLE characteristic that supports read + notify.
- **Web page** (`index.html`) connects to the ESP32 via Web Bluetooth and polls the
  characteristic once per second, logging the value to the browser console.

## Hardware

| Part | Notes |
|------|-------|
| Arduino Uno | 5V logic |
| ESP32 dev module | 3.3V logic — do **not** feed 5V into its pins |
| MQ-138 VOC sensor | Analog output, needs a warm-up period |
| 16x2 character LCD (HD44780) | Parallel interface |
| Passive/active buzzer | |
| Logic level shifter | For the Arduino 5V TX → ESP32 3.3V RX line |
| Common ground | All boards must share GND |

## Wiring

### MQ-138 → Arduino
| MQ-138 | Arduino |
|--------|---------|
| VCC | 5V |
| GND | GND |
| AOUT | A0 |

### LCD (HD44780) → Arduino
Pin mapping from the sketch: `LiquidCrystal lcd(12, 11, 5, 4, 3, 2)` → RS, E, D4, D5, D6, D7.

| LCD | Arduino |
|-----|---------|
| RS | D12 |
| E | D11 |
| D4 | D5 |
| D5 | D4 |
| D6 | D3 |
| D7 | D2 |
| VSS | GND |
| VDD | 5V |
| RW | GND |
| A (backlight +) | 5V (via resistor) |
| K (backlight -) | GND |
| V0 (contrast) | wiper of a 10k potentiometer between 5V and GND |

### Buzzer → Arduino
| Buzzer | Arduino |
|--------|---------|
| + | D8 |
| - | GND |

### Arduino ↔ ESP32 (UART)
The Arduino's TX is 5V and the ESP32 is **not** 5V-tolerant. Use a logic level shifter on
the Arduino → ESP32 direction.

| Signal | Arduino | ESP32 |
|--------|---------|-------|
| Arduino TX → ESP32 RX | D7 (TX) | GPIO16 (RX) **via level shifter** |
| ESP32 TX → Arduino RX | D6 (RX) | GPIO17 (TX) |
| Ground | GND | GND (shared) |

> The ESP32 → Arduino direction is 3.3V into a 5V input, which the Uno reads fine, so no
> shifter is needed on that line. Only the 5V → 3.3V direction needs one.

## Firmware setup (Arduino IDE)

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. **ESP32 core**: in *File → Preferences → Additional Boards Manager URLs* add
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`,
   then install "esp32 by Espressif Systems" from *Tools → Board → Boards Manager*.
3. Open and upload `arduino_voc/arduino_voc.ino` to the Arduino Uno.
   - Uses the built-in `LiquidCrystal` and `SoftwareSerial` libraries (no extra install).
4. Open and upload `esp32_voc/esp32_voc.ino` to the ESP32.
   - Uses the BLE libraries bundled with the ESP32 core (no extra install).
5. Keep the ESP32 disconnected from the Arduino TX line while uploading, then wire it up.

> Note: the Arduino sketch has a `delay(60000)` warm-up at boot, so the MQ-138 has time to
> heat up before it starts reporting. The LCD stays on "VOC Detector" during that minute.

## Running the web page

The page uses the **Web Bluetooth API** (`navigator.bluetooth`), which has strict browser
and security requirements.

1. Use **Google Chrome or Microsoft Edge** (Chromium-based). Firefox and Safari do not
   support Web Bluetooth.
2. Serve the page over **`https://` or `http://localhost`**. Web Bluetooth is blocked on
   `file://` pages, so double-clicking `index.html` will not work. Run a local server, e.g.:
   ```powershell
   # from the BREATHE folder, using Python
   python -m http.server 8000
   # then open http://localhost:8000 in Chrome/Edge
   ```
3. Click **Connect to ESP32**, pick `ESP32-VOC-Sensor` in the pairing dialog.
4. Open the browser DevTools console (F12) to see the `PPM:` readings — the current page
   logs values to the console rather than showing them on screen.

## BLE identifiers (must match between ESP32 and web page)

| | Value |
|---|---|
| Device name | `ESP32-VOC-Sensor` |
| Service UUID | `12345678-1234-5678-1234-56789abcdef0` |
| Characteristic UUID | `abcd1234-5678-1234-5678-abcdef123456` |

## Windows compatibility

Everything in this project works on Windows, with these notes:

- **Arduino & ESP32 firmware** — Fully supported. Use the Arduino IDE for Windows. You may
  need USB serial drivers depending on your board:
  - ESP32 boards commonly use **CP210x** (Silicon Labs) or **CH340** USB-UART chips; install
    the matching Windows driver if the board doesn't enumerate as a COM port.
  - Genuine Arduino Unos work with the built-in driver; clones often need the **CH340** driver.
- **Web Bluetooth page** — Works on Windows **only in Chrome or Edge**, and only when the
  page is served over `https`/`localhost` (not `file://`). Windows 10/11 with a working
  Bluetooth LE adapter is required. Firefox and Safari will not work.
- No part of the code uses OS-specific paths or shell commands, so there is nothing to
  change for Windows beyond the driver/browser notes above.

## Known limitations / notes

- The MQ-138 reading is the **raw uncalibrated ADC value (0–1023)**, not calibrated PPM.
  The `VOC_THRESHOLD` of `300` is a raw-count threshold and should be tuned for your sensor
  and environment.
- `script.js` and `styles.css` are currently commented-out drafts; the working web client
  lives inline inside `index.html`.
