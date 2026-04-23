# ❄️ Cold Storage Monitoring Using Matter Protocol

<p align="center">
  <img src="docs/banner.png" alt="Cold Storage Matter Banner" width="800"/>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?logo=espressif" />
  <img src="https://img.shields.io/badge/Protocol-Matter-green" />
  <img src="https://img.shields.io/badge/Sensor-DHT22%20%7C%20SHT31-orange" />
  <img src="https://img.shields.io/badge/License-GPL--3.0-red" />
  <img src="https://img.shields.io/badge/Made%20by-DigitalMonk-purple" />
</p>

> **Developed by [DigitalMonk](https://digitalmonk.biz) — IoT & Embedded Systems Experts**
> 🔗 Full Case Study: [https://digitalmonk.biz/cold-storage-monitoring-using-matter-protocol/](https://digitalmonk.biz/cold-storage-monitoring-using-matter-protocol/)

---

## 📌 Overview

A smart, scalable **Cold Storage Monitoring System** built using **ESP32** and the **Matter protocol**. This system provides real-time temperature and humidity monitoring across multiple cold storage units — integrated natively with **Apple Home**, **Google Home**, and **Samsung SmartThings**.

Developed for a mid-sized food distribution company managing dairy, meats, and produce across multiple cold storage facilities.

---

## 🧩 Problem Statement

| Issue | Impact |
|---|---|
| No real-time visibility into temp/humidity | Late response to spoilage risks |
| Manual data logging | Human errors, slow response |
| Device/cloud incompatibility | Fragmented monitoring |
| No mobile access | Staff unable to monitor remotely |
| Difficult to scale | High cost per new storage room |

---

## ✅ Solution

A **Matter-over-Wi-Fi / Thread** IoT system using:
- **ESP32** as the core microcontroller
- **DHT22 or SHT31** sensors for temperature & humidity
- **ESP-IDF + ESP-Matter SDK** for Matter protocol integration
- Cross-platform access via **Apple Home, Google Home, SmartThings**
- Custom **threshold alerts** for cold storage compliance

---

## 📊 Results

| Metric | Improvement |
|---|---|
| Manual logging tasks | ⬇️ 95% reduction |
| Response to temp anomalies | ⬆️ 40% faster |
| Regulatory compliance | ✅ Automated logging |
| Platform support | Apple, Google, Samsung |
| Scalability | 12+ units deployed within weeks |

---

## 🛠️ Hardware Components

| Component | Description | Qty |
|---|---|---|
| ESP32 Dev Board | Wi-Fi & BT microcontroller | 1 per unit |
| DHT22 / SHT31 | Temperature & humidity sensor | 1 per unit |
| 16x2 I2C LCD | Local status display | 1 per unit |
| I2C LCD Adapter | Reduces wiring to 4 pins | 1 per unit |
| Thread Border Router | Bridges Thread to IP (HomePod/Nest) | 1 |
| 5V Power Supply | Powers ESP32 & peripherals | 1 per unit |
| Wi-Fi Router | Network connectivity | 1 |

---

## 🔧 Technology Stack

| Layer | Technology |
|---|---|
| **Microcontroller** | ESP32 |
| **Firmware SDK** | ESP-IDF v5.x |
| **Matter SDK** | ESP-Matter |
| **Sensor** | DHT22 / SHT31 |
| **Protocol** | Matter over Wi-Fi / Thread |
| **Smart Platforms** | Apple Home, Google Home, SmartThings |
| **Mobile App** | Android |

---

## 📁 Project Structure

```
cold-storage-matter/
├── main/
│   ├── app_main.c          # Main application — Matter setup & sensor task
│   └── CMakeLists.txt
├── components/
│   └── sensor/
│       ├── sensor.h        # Sensor abstraction header
│       ├── sensor.c        # DHT22 / SHT31 driver implementation
│       └── CMakeLists.txt
├── docs/
│   └── banner.png
├── CMakeLists.txt          # Root project CMake
├── sdkconfig.defaults      # Default build configuration
├── partitions.csv          # Flash partition table
└── README.md
```

---

## ⚙️ Prerequisites

### Software
- [ESP-IDF v5.1+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
- [ESP-Matter SDK](https://github.com/espressif/esp-matter)
- Python 3.8+
- CMake 3.16+

### Hardware
- ESP32 development board
- DHT22 **or** SHT31 sensor
- Matter-compatible Thread Border Router (Apple HomePod Mini / Google Nest Hub)
- Wi-Fi router (2.4GHz)

---

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/cold-storage-matter.git
cd cold-storage-matter
```

### 2. Set Up ESP-IDF and ESP-Matter

```bash
# Install ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh && source export.sh && cd ..

# Install ESP-Matter
git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter && ./install.sh && source ./export.sh && cd ..
```

### 3. Configure the Project

```bash
idf.py menuconfig
```

In menuconfig, select:
- **Cold Storage Config → Sensor Type**: DHT22 or SHT31
- **Cold Storage Config → Alert Thresholds**: Set min/max temp & humidity
- **Component Config → ESP Matter**: Enable Matter support

Or simply use the provided defaults:
```bash
cp sdkconfig.defaults sdkconfig
```

### 4. Build and Flash

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 5. Commission Device via Matter

Use your preferred smart home app to commission:
- **Apple Home**: Scan Matter QR code from device serial output
- **Google Home**: Use "Add device" → Matter
- **Samsung SmartThings**: Add device → Scan QR code

---

## 📐 Wiring Diagram

### DHT22 Wiring
```
ESP32 GPIO 4  ──────── DHT22 DATA
ESP32 3.3V    ──────── DHT22 VCC
ESP32 GND     ──────── DHT22 GND
              (4.7kΩ pull-up between VCC and DATA)
```

### SHT31 Wiring (I2C)
```
ESP32 GPIO 21 (SDA) ── SHT31 SDA
ESP32 GPIO 22 (SCL) ── SHT31 SCL
ESP32 3.3V          ── SHT31 VCC
ESP32 GND           ── SHT31 GND
```

### LCD 16x2 I2C Wiring
```
ESP32 GPIO 21 (SDA) ── LCD SDA
ESP32 GPIO 22 (SCL) ── LCD SCL
ESP32 5V            ── LCD VCC
ESP32 GND           ── LCD GND
```

---

## 🔔 Alert Thresholds (Configurable)

| Parameter | Default | Description |
|---|---|---|
| `TEMP_ALERT_HIGH_C` | -5°C | Alert if temp rises above this |
| `TEMP_ALERT_LOW_C` | -25°C | Alert if temp drops below this |
| `HUMIDITY_ALERT_HIGH_PCT` | 85% | Alert if humidity exceeds this |

Edit in `main/app_main.c` or via menuconfig.

---

## 📱 Smart Home Integration

Once commissioned, the device appears as:
- **Temperature Sensor** — shows live °C reading
- **Humidity Sensor** — shows live % RH reading

Supported platforms:
| Platform | Support |
|---|---|
| 🍎 Apple Home | ✅ Full |
| 🏠 Google Home | ✅ Full |
| 📱 Samsung SmartThings | ✅ Full |
| 🪟 Amazon Alexa | ✅ Full (Matter) |

---

## 🔍 Serial Monitor Output

```
I (1234) cold_storage_matter: ==============================================
I (1234) cold_storage_matter:  Cold Storage Monitor — Matter Protocol
I (1234) cold_storage_matter:  By DigitalMonk | https://digitalmonk.biz
I (1234) cold_storage_matter: ==============================================
I (2345) sensor: ✅ Sensor initialized
I (5678) cold_storage_matter: ✅ Matter Commissioning complete!
I (6789) cold_storage_matter: 🌡 Temp: -18.50°C  💧 Humidity: 62.30%
I (36789) cold_storage_matter: 🌡 Temp: -18.20°C  💧 Humidity: 62.50%
```

---

## 🧪 Simulation Mode

No hardware? Run in simulation mode:

In `sdkconfig.defaults`, comment out both sensor options:
```
# CONFIG_SENSOR_DHT22=y
# CONFIG_SENSOR_SHT31=y
```

The firmware will generate realistic simulated readings for testing.

---

## 📜 License

This project is licensed under the **GNU General Public License v3.0**.
See [LICENSE](LICENSE) for details.

---

## 🏢 About DigitalMonk

**DigitalMonk** is a full-stack IoT and embedded systems development company. If you're looking to build similar solutions, you can <a href="https://digitalmonk.biz/hire-esp32-developer/">hire esp32 developer</a> from our expert team.

> *"From firmware to front-end — full-stack IoT capabilities under one roof."*

- 🌐 Website: [https://digitalmonk.biz](https://digitalmonk.biz)
- 📞 Phone: +91 62846-36956
- ✉️ Email: hello@digitalmonk.biz
- 📍 Jalandhar, Punjab, India | Alpine, CA, USA | Coventry, UK

**Services:**
- IoT & Embedded Development
- ESP32 / Arduino / Raspberry Pi Solutions
- Matter Protocol Integration
- PCB Design & Firmware
- Mobile & Web App Development

🔗 **[Get a Free Project Quote →](https://digitalmonk.biz/contact-us/)**

---

## ⭐ Client Testimonial

> *"DigitalMonk was great to work with, a true pro, and very honest/ethical in their approach. They took my very bare-bones ideas and turned them into a working device."*
> — **Stephanie Dubois** ⭐⭐⭐⭐⭐

---

<p align="center">
  Made with ❤️ by <a href="https://digitalmonk.biz">DigitalMonk</a>
</p>
