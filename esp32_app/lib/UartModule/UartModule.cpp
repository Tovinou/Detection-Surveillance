#include "UartModule.h"
#include "../../include/config.h"

UartModule piUart(config.hardware.uart_rx_pin, config.hardware.uart_tx_pin);

UartModule::UartModule(int rxPin, int txPin, long baudRate) 
    : uart(nullptr), rxPin(rxPin), txPin(txPin), baudRate(baudRate), initialized(false) {}

UartModule::~UartModule() {
    end();
}

bool UartModule::begin() {
    if (initialized) return true;

    uart = new HardwareSerial(2); // Use UART2 on ESP32
    uart->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    uart->setTimeout(100); // 100ms timeout
    
    initialized = true;
    
    Serial.println(" UART initialized");
    Serial.println(" RX Pin: " + String(rxPin) + ", TX Pin: " + String(txPin));
    Serial.println(" Baud Rate: " + String(baudRate));
    
    // Send initialization message to Raspberry Pi
    writeString("ESP32_READY\n");
    
    return true;
}

void UartModule::end() {
    if (uart) {
        uart->end();
        delete uart;
        uart = nullptr;
    }
    initialized = false;
    Serial.println(" UART stopped");
}

bool UartModule::available() {
    if (!initialized) return false;
    return uart->available() > 0;
}

String UartModule::readString() {
    if (!initialized) return "";
    
    String data = uart->readStringUntil('\n');
    data.trim();
    
    if (data.length() > 0) {
        Serial.println(" UART received: " + data);
    }
    
    return data;
}

void UartModule::writeString(const String& data) {
    if (!initialized) return;
    
    uart->print(data);
    Serial.println(" UART sent: " + data);
}

void UartModule::writeBytes(const uint8_t* data, size_t length) {
    if (!initialized) return;
    
    uart->write(data, length);
    Serial.println(" UART sent " + String(length) + " bytes");
}