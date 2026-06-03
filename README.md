# Smart Environment Monitoring Dashboard

This is an ESP32-based IoT mini project for monitoring temperature, humidity, and motion. The ESP32 reads data from a DHT11 sensor and PIR motion sensor, then displays live values on a browser-based dashboard through Wi-Fi.

## Features

- Live temperature monitoring
- Live humidity monitoring
- PIR motion detection
- Fan LED automation when temperature is high
- Ventilation LED automation when humidity is high
- Security LED automation when motion is detected
- ESP32-hosted web dashboard
- Temperature and humidity chart
- Data history table
- PDF report download from dashboard

## Requirements

See [requirements.txt](requirements.txt).

## Hardware Connections

| Component | ESP32 Pin |
|---|---|
| DHT11 DATA | GPIO 26 |
| PIR Sensor OUT | GPIO 27 |
| Fan LED | GPIO 18 |
| Ventilation LED | GPIO 19 |
| Security LED | GPIO 21 |
| DHT11 VCC | 3.3V |
| DHT11 GND | GND |
| PIR VCC | 5V |
| PIR GND | GND |

Each LED should be connected with a 220 ohm resistor.

## Arduino Libraries

Install these libraries in Arduino IDE:

- DHT sensor library by Adafruit
- Adafruit Unified Sensor

Also install the ESP32 board package:

- Boards Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Board package: `esp32` by Espressif Systems

## How to Run

1. Open `SmartEnvironmentMonitor.ino` in Arduino IDE.
2. Select the correct ESP32 board.
3. Select the correct COM port.
4. Upload the sketch to the ESP32.
5. Open Serial Monitor at `115200` baud.
6. Enter the Wi-Fi name and password when requested.
7. Copy the IP address printed in Serial Monitor.
8. Open the IP address in a browser connected to the same Wi-Fi network.

Example:

```text
http://192.168.1.10
```

## Automation Rules

| Condition | Action |
|---|---|
| Temperature greater than 35 C | Fan LED ON |
| Humidity greater than 80% | Ventilation LED ON |
| Motion detected | Security LED ON |

## Project Files

| File | Description |
|---|---|
| `SmartEnvironmentMonitor.ino` | Arduino sketch for ESP32 |
| `ARDIUNO.txt` | Original Arduino code text file |
| `PROJECT1_.pdf` | Project report |
| `requirements.txt` | Project requirements |
| `.gitignore` | Git ignore rules |

## Notes

The dashboard uses external CDN links for Chart.js, jsPDF, and jsPDF AutoTable. The ESP32 and the browser device need internet access if those dashboard features are used.
