# Distance Detection Surveillance Project with Raspberry Pi Zero and ESP32

A camera-less distance detection system that uses an ultrasonic sensor on a Raspberry Pi Zero to detect objects within a set distance threshold, with visual feedback through a Freenove 8 RGB LED Module and wireless communication via ESP32.

## Features

- **Distance-based object detection** using ultrasonic sensor (HC-SR04)
- **Visual feedback** through Freenove 8 RGB LED Module with multiple display modes
- **Wireless communication** between Raspberry Pi Zero and ESP32
- **Web dashboard** for monitoring distance data
- **Optional Telegram alerts** for object detection
- **Data logging** and history tracking
- **Configurable system parameters**

## Hardware Requirements

### Raspberry Pi Zero
- Raspberry Pi Zero W
- Ultrasonic sensor (HC-SR04)
- Freenove 8 RGB LED Module
- Active Buzzer
- Jumper wires
- Resistors (1kΩ + 2kΩ for voltage divider on Echo pin)
- MicroSD card (8GB+)
- Power supply (5V)

### ESP32
- ESP32 development board (e.g., ESP32-DevKitC)
- Jumper wires
- Power supply (5V)

## Wiring Diagram

### Ultrasonic Sensor to Raspberry Pi Zero
- **VCC** → Pi 5V (Pin 2/4)
- **GND** → Pi GND (Pin 6)
- **Trig** → Pi GPIO 17 (Pin 11)
- **Echo** → Voltage divider (Echo to 1kΩ resistor → Pi GPIO 27 → 2kΩ to GND)

### Freenove 8 RGB LED Module to Raspberry Pi Zero
- **V** → Pi 5V (Pin 2/4)
- **G** → Pi GND (Pin 6)
- **S** → Pi GPIO 18 (Pin 12)

### Buzzer to Raspberry Pi Zero
- **Positive (+)** → Pi GPIO 23 (Pin 16)
- **Negative (-)** → Pi GND (Pin 14)

### Raspberry Pi Zero to ESP32
**For UART:**
- Pi TX (Pin 8) → ESP32 RX (GPIO 16)
- Pi RX (Pin 10) → ESP32 TX (GPIO 17)
- GND shared

**For SPI:**
- Pi SCLK (GPIO 11, Pin 23) → ESP32 GPIO 18 (SCK)
- Pi MOSI (GPIO 10, Pin 19) → ESP32 GPIO 23 (MOSI)
- Pi MISO (GPIO 9, Pin 21) → ESP32 GPIO 19 (MISO)
- Pi CE0 (GPIO 8, Pin 24) → ESP32 GPIO 5 (CS)
- GND shared

## Software Requirements

### Raspberry Pi Zero
- Raspberry Pi OS Lite
- Python 3
- **Required Python libraries:**
    - `RPi.GPIO`
    - `rpi_ws281x`
    - `adafruit-circuitpython-neopixel`
    - `pyserial`
    - `spidev`

### ESP32
- Arduino IDE or PlatformIO
- ESP32 core for Arduino
- **Required libraries:**
    - `WiFi`
    - `AsyncWebServer`
    - `UniversalTelegramBot` (optional)
    - `PubSubClient` (for MQTT, optional)

## Project Structure

### Raspberry Pi Zero (Python)
- **`config.py`**: Contains all configuration constants, making it easy to change settings without modifying the main logic:
    - Hardware pin configurations
    - LED settings
    - Sensor thresholds
    - Communication parameters
- **`sensors.py`**: Handles all interactions with the ultrasonic sensor:
    - GPIO initialization
    - Distance measurement
    - Error handling for sensor timeouts
- **`led_display.py`**: Controls all aspects of the LED display:
    - Multiple display modes (Normal, History, Directional)
    - Status indicators
    - Alert patterns
    - Visual feedback for system status
- **`buzzer.py`**: Controls audio feedback:
    - Alarm triggering
    - Status beeps
- **`logger.py`**: Handles system logging:
    - Writes events to log files
    - Manages log rotation
- **`communication.py`**: Handles data transmission to the ESP32:
    - UART communication for alerts
    - SPI communication for distance data
    - Error handling and status reporting
- **`main.py`**: The main application file that orchestrates everything:
    - System initialization
    - Main application loop
    - Coordination between modules

### ESP32 (C++)
- **`config.cpp`**: Configuration management for the ESP32:
    - Hardware pin assignments
    - WiFi settings
    - System parameters
    - Communication settings
- **`state.cpp`**: System state management:
    - Current sensor data
    - Historical data tracking
    - System status monitoring
    - Utility methods for data analysis
- **`html_page.h`**: HTML page generation for the web dashboard:
    - Dashboard page
    - API responses
    - Configuration page
    - History page
    - Error pages
- **`main.cpp`**: Main ESP32 application:
    - WiFi setup
    - Pi communication handling
    - Alert processing
    - Web server management
    - System health monitoring

## Installation and Setup

### Raspberry Pi Zero Setup
1. Install Raspberry Pi OS Lite on the microSD card.
2. Boot up the Raspberry Pi and connect to WiFi.
3. Install required Python libraries:
    ```bash
    sudo apt update
    sudo apt install python3-pip python3-serial
    sudo pip3 install rpi_ws281x adafruit-circuitpython-neopixel
    ```
4. Clone or download the project files to the Raspberry Pi.
5. Configure the settings in `config.py` as needed.
6. Enable SPI and UART on the Raspberry Pi:
    ```bash
    sudo raspi-config
    # Navigate to Interfacing Options > SPI > Enable
    # Navigate to Interfacing Options > Serial > Enable, but disable serial console
    ```

### ESP32 Setup
1. Install the Arduino IDE or PlatformIO.
2. Install the ESP32 core for Arduino.
3. Install required libraries:
    - `WiFi`
    - `AsyncWebServer`
    - `UniversalTelegramBot` (if using Telegram alerts)
    - `PubSubClient` (if using MQTT)
4. Open the project files in the Arduino IDE or PlatformIO.
5. Configure the settings in `config.cpp` as needed.
6. Upload the code to the ESP32.

## Usage
1. Connect all hardware components as described in the wiring diagram.
2. Power on the Raspberry Pi Zero.
3. Run the main application:
    ```bash
    python3 main.py
    ```
4. Power on the ESP32.
5. Connect to the ESP32's WiFi hotspot or access the web dashboard at the ESP32's IP address.

The system will now detect objects within the configured distance threshold and provide visual feedback through the LED module.

### LED Display Modes
The Freenove 8 RGB LED Module supports three display modes that automatically cycle:
- **Normal Mode:** Shows current distance as a bar graph (green to red gradient).
- **History Mode:** Displays recent distance measurements as a gradient from blue (oldest) to purple (newest).
- **Directional Mode:** Shows detection direction (requires multiple sensors).

The last LED on the strip indicates communication status:
- **Blue:** Idle/Connected
- **Yellow:** Sending data
- **Red:** Communication error

## Troubleshooting

### Common Issues

**LED Module Not Working**
- Check wiring connections.
- Ensure the LED module is receiving adequate power.
- Verify the GPIO pin configuration in `config.py`.

**Ultrasonic Sensor Not Reading Correctly**
- Check wiring, especially the voltage divider for the Echo pin.
- Verify GPIO pin configurations in `config.py`.
- Check for obstacles that might interfere with the sensor.

**ESP32 Not Connecting to Raspberry Pi**
- Verify UART/SPI wiring connections.
- Check baud rate settings.
- Ensure both devices are properly grounded.

**Web Dashboard Not Accessible**
- Check if the ESP32 is connected to WiFi.
- Verify the ESP32's IP address.
- Check if the web server is enabled in the configuration.

## Future Enhancements
- **Camera Integration:** Add camera functionality for visual verification of detected objects.
- **Multiple Sensors:** Add more ultrasonic sensors for directional detection.
- **Machine Learning:** Implement ML algorithms for object classification.
- **Mobile App:** Develop a mobile application for remote monitoring.
- **Cloud Integration:** Add cloud storage and analytics capabilities.

## License
This project is open source and available under the MIT License.

## Contributing
Contributions are welcome! Please feel free to submit pull requests or create issues for bugs and feature requests.
