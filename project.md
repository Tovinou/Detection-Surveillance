# Distance Detection Surveillance Project

This project implements a distance detection surveillance system using a Raspberry Pi Zero and an ESP32. It features ultrasonic distance sensing, visual and audio feedback, and wireless communication for monitoring and alerts.

## Overview

*   **Raspberry Pi Zero:** Acts as the primary sensor node. It reads distance data from an HC-SR04 ultrasonic sensor, provides immediate visual feedback via a Freenove 8 RGB LED module, and audio feedback via a buzzer. It communicates status and data to the ESP32.
*   **ESP32:** Acts as a wireless gateway and dashboard. It receives data from the Pi via UART/SPI, hosts a web server for real-time monitoring, and can send alerts (e.g., via Telegram or MQTT).

## Hardware Components

*   **Raspberry Pi Zero W**
*   **Ultrasonic Sensor (HC-SR04)**
*   **Freenove 8 RGB LED Module**
*   **Active Buzzer**
*   **ESP32 Development Board** (e.g., ESP32-DevKitC)
*   **MicroSD Card** (for Pi OS and logs)
*   **Level Shifter / Voltage Divider** (for 5V Echo signal to 3.3V Pi GPIO)

## Software Architecture

### Raspberry Pi (Python)
The Pi runs a modular Python application:
*   `main.py`: Main application entry point and orchestration.
*   `config.py`: Centralized configuration (pins, thresholds, settings).
*   `sensors.py`: Handles ultrasonic sensor interactions.
*   `led_display.py`: Manages LED visual feedback patterns.
*   `buzzer.py`: Controls audio feedback.
*   `communication.py`: Handles UART/SPI communication with ESP32.
*   `logger.py`: Handles local logging to files.

### ESP32 (C++ / PlatformIO)
The ESP32 firmware is built using PlatformIO:
*   `main.cpp`: Main setup and loop, handling WiFi and coordination.
*   `config.cpp/h`: Hardware and system configuration.
*   `state.cpp`: Manages system state and sensor data.
*   `html_page.h`: Web interface templates.

## Getting Started

Please refer to [Readme.md](Readme.md) for detailed installation instructions, wiring diagrams, and usage guides.
