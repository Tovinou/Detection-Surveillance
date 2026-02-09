#include "PiCommunication.h"
#include "../../include/config.h"
#include <ArduinoJson.h>

// Global instance (will be defined in main.cpp)
extern SystemState systemState;
extern AppConfig config;

PiCommunication piComm(systemState);

void PiCommunication::begin() {
    // Setup UART (Pi → ESP32)
    if (config.system.enable_uart) {
        Serial2.begin(config.raspberry_pi.uart_baud_rate, SERIAL_8N1, 
                     config.hardware.uart_rx_pin, config.hardware.uart_tx_pin);
        Serial.println("✓ UART communication with Pi initialized");
    }
    
    // Setup SPI (Pi → ESP32 for faster data)
    if (config.system.enable_spi) {
        SPI.begin(config.hardware.spi_sck_pin, 
                  config.hardware.spi_miso_pin, 
                  config.hardware.spi_mosi_pin, 
                  config.hardware.spi_cs_pin);
        pinMode(config.hardware.spi_cs_pin, INPUT_PULLUP);
        Serial.println("✓ SPI communication with Pi initialized");
    }
}

void PiCommunication::handle() {
    // Check for UART messages from Pi
    if (config.system.enable_uart && Serial2.available()) {
        while (Serial2.available()) {
            char c = Serial2.read();
            if (c == '\n') {
                processPiMessage(receivedData);
                receivedData = "";
            } else {
                receivedData += c;
            }
        }
    }
    
    // Check for SPI data from Pi
    checkSPIData();
}

void PiCommunication::processPiMessage(String message) {
    Serial.print("Received from Pi: ");
    Serial.println(message);
    
    // Check if message is JSON
    if (message.startsWith("{")) {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error) {
            float distance = doc["d"];
            int mode = doc["m"];
            int alert = doc["a"];
            JsonArray history = doc["h"];
            
            // Update SystemState
            systemState.currentData.distance = distance;
            systemState.currentData.mode = mode;
            systemState.currentData.alert_active = (alert == 1);
            systemState.currentData.timestamp = millis();
            systemState.currentData.object_detected = (distance <= config.system.distance_threshold) || systemState.currentData.alert_active;
            
            if (systemState.currentData.object_detected) {
                systemState.currentData.status = "Object Detected 🚨";
            } else {
                systemState.currentData.status = "Normal ✅";
            }
            
            // Update history snippet
            int i = 0;
            for(float val : history) {
                if(i < 5) systemState.currentData.pi_history[i++] = val;
            }
            
            systemState.addToHistory(systemState.currentData);
            systemState.lastPiHeartbeat = millis();
            return;
        } else {
            Serial.print("JSON Parse Error: ");
            Serial.println(error.c_str());
        }
    }

    // Check for Pi Boot/Console messages
    if (message.indexOf("Debian") != -1 || message.indexOf("login:") != -1 || message.indexOf("Linux") != -1) {
        Serial.println("⚠️ Pi Serial Console detected! Please disable it using 'sudo raspi-config' -> Interface Options -> Serial Port");
        return;
    }

    // Parse legacy messages from Raspberry Pi
    if (message.startsWith("DISTANCE:")) {
        float distance = message.substring(9).toFloat();
        updateSystemState(distance);
    }
    else if (message.startsWith("ALERT:")) {
        String alertLevel = message.substring(6);
        triggerAlert(alertLevel);
    }
    else if (message.startsWith("STATUS:")) {
        String status = message.substring(7);
        updateSystemStatus(status);
    }
    else if (message.startsWith("SYSTEM:")) {
        String systemMsg = message.substring(7);
        Serial.println("System message from Pi: " + systemMsg);
    }
    else if (message.startsWith("HEARTBEAT:")) {
        systemState.lastPiHeartbeat = millis();
        Serial.println("✓ Pi heartbeat received");
    }
    else if (message.startsWith("ERROR:")) {
        String errorMsg = message.substring(6);
        Serial.println("✗ Pi error: " + errorMsg);
    }
}

void PiCommunication::updateSystemState(float distance) {
    systemState.currentData.distance = distance;
    systemState.currentData.timestamp = millis();
    systemState.currentData.object_detected = (distance <= config.system.distance_threshold);
    
    if (distance <= config.system.distance_threshold) {
        systemState.currentData.status = "Object Detected 🚨";
    } else {
        systemState.currentData.status = "Normal ✅";
    }
    
    // Add to history
    systemState.addToHistory(systemState.currentData);
}

void PiCommunication::triggerAlert(String level) {
    Serial.print("Alert from Pi: ");
    Serial.println(level);
    
    // Visual alert on ESP32
    if (config.hardware.status_led_pin > 0) {
        digitalWrite(config.hardware.status_led_pin, HIGH);
        delay(200);
        digitalWrite(config.hardware.status_led_pin, LOW);
    }
}

void PiCommunication::updateSystemStatus(String status) {
    systemState.currentData.status = status;
}

void PiCommunication::checkSPIData() {
    // Basic SPI slave implementation
    if (config.system.enable_spi && digitalRead(config.hardware.spi_cs_pin) == LOW) {
        // SPI communication active - implement your protocol here
        // This would handle high-speed data like images
    }
}