#include "SpiModule.h"

SpiModule spiSlave(config.hardware.spi_cs_pin, config.hardware.spi_mosi_pin, 
                   config.hardware.spi_miso_pin, config.hardware.spi_sck_pin);

SpiModule* SpiModule::instance = nullptr;

SpiModule::SpiModule(int csPin, int mosiPin, int misoPin, int sckPin) 
    : csPin(csPin), mosiPin(mosiPin), misoPin(misoPin), sckPin(sckPin), 
      initialized(false), dataReady(false), bufferIndex(0), receiving(false) {
    instance = this;
}

SpiModule::~SpiModule() {
    end();
    instance = nullptr;
}

bool SpiModule::begin() {
    if (initialized) return true;

    // Initialize SPI in slave mode
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(1000000); // 1MHz
    
    // Set up CS pin interrupt
    pinMode(csPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(csPin), spiISR, FALLING);

    initialized = true;
    
    Serial.println(" SPI slave initialized");
    Serial.println(" Pins - CS: " + String(csPin) + 
                  ", MOSI: " + String(mosiPin) + 
                  ", MISO: " + String(misoPin) + 
                  ", SCK: " + String(sckPin));
    
    return true;
}

void SpiModule::end() {
    if (initialized) {
        detachInterrupt(digitalPinToInterrupt(csPin));
        SPI.end();
        initialized = false;
    }
    Serial.println(" SPI slave stopped");
}

void IRAM_ATTR SpiModule::spiISR() {
    if (instance) {
        instance->handleInterrupt();
    }
}

void IRAM_ATTR SpiModule::handleInterrupt() {
    // CS pin went low - master wants to communicate
    while (digitalRead(csPin) == LOW) {
        uint8_t receivedByte = SPI.transfer(0x00); // Read byte from master
        
        if (receivedByte == 0x01) {  // Start of transmission
            receiving = true;
            bufferIndex = 0;
            dataReady = false;
        } else if (receivedByte == 0xFF) {  // End of transmission
            if (receiving && bufferIndex == 4) { // Expecting 4 bytes for float
                dataReady = true;
            }
            receiving = false;
            bufferIndex = 0;
        } else if (receiving && bufferIndex < 4) {
            buffer[bufferIndex++] = receivedByte;
        }
    }
}

bool SpiModule::hasData() {
    if (dataReady) {
        dataReady = false; // Reset flag after reading
        return true;
    }
    return false;
}

float SpiModule::getDistanceData() {
    if (bufferIndex == 4) {
        // Convert 4 bytes back to float
        union {
            uint8_t bytes[4];
            float value;
        } converter;
        
        for (int i = 0; i < 4; i++) {
            converter.bytes[i] = buffer[i];
        }
        
        float distance = converter.value;
        
        // Validate distance
        if (distance >= 0 && distance <= 400) { // Valid ultrasonic range
            Serial.println(" SPI distance received: " + String(distance, 1) + "cm");
            return distance;
        } else {
            Serial.println(" Invalid SPI distance: " + String(distance, 1));
        }
    }
    
    return -1.0; // Invalid data
}

void SpiModule::sendResponse(uint8_t response) {
    if (!initialized) return;
    
    // Wait for CS to go low (master ready)
    unsigned long startTime = millis();
    while (digitalRead(csPin) == HIGH) {
        if (millis() - startTime > 100) { // Timeout after 100ms
            Serial.println(" SPI send timeout");
            return;
        }
    }
    
    // Send response byte
    SPI.transfer(response);
    Serial.println(" SPI response sent: 0x" + String(response, HEX));
}