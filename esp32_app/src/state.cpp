#include "state.h"
#include <Arduino.h>
#include "config.h"

SystemState systemState;

SystemState::SystemState() {
  // Initialize with safe default values
  currentData.distance = 0.0;
  currentData.timestamp = millis();
  currentData.object_detected = false;
  currentData.status = "Initializing ✅";
  
  // Initialize history array with safe values
  for (int i = 0; i < HISTORY_SIZE; i++) {
    history[i].distance = 0.0;
    history[i].timestamp = 0;
    history[i].object_detected = false;
    history[i].status = "No Data";
  }
  
  // Initialize system state
  lastAlertTime = 0;
  systemUptime = 0;
  wifiConnected = false;
  wifiMode = "AP";
  wifiSignalStrength = 0;
  lastPiHeartbeat = 0;
  piConnected = false;
  
  // Initialize history tracking
  historyIndex = 0;
  historyCount = 0;
}

void SystemState::updateData(const SensorData& newData) {
  currentData = newData;
  addToHistory(newData);
}

void SystemState::addToHistory(const SensorData& data) {
  // Only add valid data to history
  if (data.isValid()) {
    history[historyIndex] = data;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) {
      historyCount++;
    }
  }
}

void SystemState::updateUptime() {
  systemUptime = millis() / 1000;
}

SensorData SystemState::getHistory(int index) const {
  if (index < 0 || index >= historyCount) {
    SensorData empty;
    empty.status = "Invalid Index";
    return empty;
  }
  
  // Calculate actual index in circular buffer
  int actualIndex = (historyIndex - index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
  return history[actualIndex];
}

int SystemState::getHistoryCount() const {
  return historyCount;
}

String SystemState::getFormattedUptime() const {
  unsigned long seconds = systemUptime;
  
  // Handle very large uptimes safely
  if (seconds > 31536000) { // More than 1 year
    unsigned long years = seconds / 31536000;
    seconds %= 31536000;
    unsigned long days = seconds / 86400;
    return String(years) + "y " + String(days) + "d";
  }
  
  unsigned long days = seconds / 86400;
  seconds %= 86400;
  unsigned long hours = seconds / 3600;
  seconds %= 3600;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  
  if (days > 0) {
    return String(days) + "d " + String(hours) + "h " + String(minutes) + "m";
  } else if (hours > 0) {
    return String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
  } else if (minutes > 0) {
    return String(minutes) + "m " + String(seconds) + "s";
  } else {
    return String(seconds) + "s";
  }
}

bool SystemState::hasRecentAlert() const {
  return currentData.object_detected && 
         (millis() - lastAlertTime < 30000); // 30-second window
}

bool SystemState::isPiConnected() const {
  return (millis() - lastPiHeartbeat) < config.raspberry_pi.connection_timeout;
}

float SystemState::getAverageDistance(int samples) const {
  if (samples <= 0 || samples > historyCount) {
    return currentData.distance;
  }
  
  float sum = 0.0;
  int validSamples = 0;
  
  for (int i = 0; i < samples; i++) {
    SensorData data = getHistory(i);
    if (data.isValid()) {
      sum += data.distance;
      validSamples++;
    }
  }
  
  return validSamples > 0 ? sum / validSamples : 0.0;
}

int SystemState::getDetectionCount(int timeWindow) const {
  if (timeWindow <= 0) {
    return 0;
  }
  
  unsigned long windowStart = millis() - (timeWindow * 1000);
  int count = 0;
  
  for (int i = 0; i < historyCount; i++) {
    SensorData data = getHistory(i);
    if (data.timestamp >= windowStart && data.object_detected) {
      count++;
    }
  }
  
  return count;
}
