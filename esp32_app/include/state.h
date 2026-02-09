#ifndef STATE_H
#define STATE_H

#include <Arduino.h>

// Holds one reading from the ultrasonic sensor
struct SensorData {
  float distance = -1.0;
  unsigned long timestamp = 0;
  bool object_detected = false;
  bool alert_active = false; // From JSON 'a'
  int mode = 0;              // From JSON 'm'
  float pi_history[5] = {0}; // From JSON 'h'
  String status = "Initializing";
  
  bool isValid() const {
    return distance > 0 && distance < 400; // Valid ultrasonic range
  }
};

class SystemState {
private:
  static const int HISTORY_SIZE = 100;
  SensorData history[HISTORY_SIZE];
  int historyIndex = 0;
  int historyCount = 0;

public:
  // Current sensor reading
  SensorData currentData;

  // System tracking
  unsigned long lastAlertTime = 0;
  unsigned long systemUptime = 0;
  bool wifiConnected = false;
  String wifiMode = "AP";
  int wifiSignalStrength = 0;

  // Pi connection tracking
  unsigned long lastPiHeartbeat = 0;
  bool piConnected = false;

  SystemState();

  // Update methods
  void updateData(const SensorData& newData);
  void addToHistory(const SensorData& data);
  void updateUptime();

  // Accessors
  SensorData getHistory(int index) const;
  int getHistoryCount() const;
  String getFormattedUptime() const;

  // Pi connection check (15s timeout)
  bool isPiConnected() const;

  bool hasRecentAlert() const;
  float getAverageDistance(int samples) const;
  int getDetectionCount(int timeWindow) const;
};

// Declare global instance
extern SystemState systemState;

#endif // STATE_H
