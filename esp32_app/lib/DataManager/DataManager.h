#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "../../include/state.h"
#include "../../include/config.h"

class DataManager {
private:
  Preferences preferences;
  bool initialized;
  
public:
  DataManager();
  ~DataManager();
  
  bool begin();
  void end();
  void saveConfig(const AppConfig& cfg);
  bool loadConfig(AppConfig& cfg);
  void logEvent(const String& event);
  void saveSensorData(const SensorData& data);
  void cleanupOldData(int maxAgeDays = 30);
  void exportDataToJson();
  void resetAllData();
  bool isInitialized() const { return initialized; }
};

extern DataManager dataManager;

#endif