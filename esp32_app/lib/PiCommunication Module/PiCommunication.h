#ifndef PI_COMMUNICATION_H
#define PI_COMMUNICATION_H

#include <Arduino.h>
#include <SPI.h>
#include <HardwareSerial.h>
#include "../../include/config.h"
#include "../../include/state.h"

class PiCommunication {
private:
    SystemState& systemState;
    String receivedData = "";
    
    void processPiMessage(String message);
    void checkSPIData();

public:
    PiCommunication(SystemState& state) : systemState(state) {}
    
    void begin();
    void handle();
    void updateSystemState(float distance);
    void triggerAlert(String level);
    void updateSystemStatus(String status);
};

extern PiCommunication piComm;

#endif