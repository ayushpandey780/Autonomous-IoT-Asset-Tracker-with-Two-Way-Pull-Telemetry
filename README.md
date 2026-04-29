# Autonomous IoT Asset Tracker with Two-Way Pull Telemetry

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![Backend](https://img.shields.io/badge/backend-Node.js-green)
![License](https://img.shields.io/badge/license-MIT-blue)

## 📌 Project Overview
This project is an enterprise-grade, resilient IoT asset tracking system developed as a Major Project for the MCA program at Amity University, Jharkhand. It features a hybrid hardware-software architecture that bridges an ESP32 microcontroller with a custom Node.js middleware server to deliver real-time, zero-cost GPS telemetry.

Initially architected for direct GNSS-Cellular positioning via a SIM A7670C module, the project underwent an agile pivot due to a severe hardware voltage clash. The transport layer was abstracted to utilize the ESP32's native 802.11 Wi-Fi, routing data through a local API gateway exposed via Ngrok. This approach bypassed the heavy computational overhead of embedded SSL/HTTPS handshakes required by modern APIs.

## 🚀 Key Features
* **Two-Way Pull Telemetry:** The ESP32 acts as an asynchronous polling client. It sits quietly until interrogated via a WhatsApp command, conserving API calls and preventing spam bans.
* **Middleware API Gateway:** A custom Node.js (`Express.js`) server handles heavy HTTPS handshakes with external services, allowing the ESP32 to send lightweight, unencrypted HTTP GET requests.
* **Multi-Channel Alerting:** Automated payload dispatch via Twilio (WhatsApp) and Nodemailer (Gmail SMTP).
* **Live WebSockets Dashboard:** A real-time `Socket.io` and `Leaflet.js` web interface for live dot-on-map tracking without page refreshes.
* **"Zero-Signal" Fallback:** Intelligent error handling that detects indoor satellite blind spots and formats specialized alerts instead of broken coordinate links.

## 🧰 Hardware Architecture
* **Microcontroller:** ESP32 Development Board (Dual-core, native Wi-Fi).
* **GNSS Module:** BN-220 GPS/GLONASS. Chosen for its compact form factor, lack of failure-prone EEPROM batteries, and rapid Time-To-First-Fix (TTFF).

### Circuit Schematic
```text
       +---------------------------------------+
       |           ESP32 Dev Board             |
       |                                       |
       |  [USB PORT] <---- (To Power/Laptop)   |
       |                                       |
       |  (VIN) 5V [o]--------------------[VCC] (RED)
       |                                       |
       |  (GND)    [o]--------------------[GND] (BLACK)
       |                                       |   BN-220 GPS
       |  (GPIO 18)[o]<-------------------[TX]  (WHITE)
       |                                       |
       |  (GPIO 19)[o]------------------->[RX]  (GREEN)
       |                                       |
       +---------------------------------------+
