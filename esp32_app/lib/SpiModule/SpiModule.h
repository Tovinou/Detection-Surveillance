#ifndef SPI_MODULE_H
#define SPI_MODULE_H

#include <Arduino.h>
#include <SPI.h>
#include "../../include/state.h"
#include "../../include/config.h"

class SpiModule {
private:
  int csPin;
  int mosiPin;
  int misoPin;
  int sckPin;
  bool initialized;
  
  // SPI communication state
  volatile bool dataReady;
  volatile uint8_t buffer[8];
  volatile int bufferIndex;
  volatile bool receiving;
  
  static SpiModule* instance;
  
  static void IRAM_ATTR spiISR();
  void handleInterrupt();

public:
  SpiModule(int csPin, int mosiPin, int misoPin, int sckPin);
  ~SpiModule();
  
  bool begin();
  void end();
  bool hasData();
  float getDistanceData();
  void sendResponse(uint8_t response);
  bool isInitialized() const { return initialized; }
};

extern SpiModule spiSlave;

#endif