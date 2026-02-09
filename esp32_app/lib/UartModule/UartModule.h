#ifndef UART_MODULE_H
#define UART_MODULE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "../../include/state.h"

class UartModule {
private:
  HardwareSerial* uart;
  int rxPin;
  int txPin;
  long baudRate;
  bool initialized;
  
public:
  UartModule(int rxPin, int txPin, long baudRate = 9600);
  ~UartModule();
  
  bool begin();
  void end();
  bool available();
  String readString();
  void writeString(const String& data);
  void writeBytes(const uint8_t* data, size_t length);
  bool isInitialized() const { return initialized; }
};

extern UartModule piUart;

#endif