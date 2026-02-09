#include "DataManager.h"
#include <SPIFFS.h>

DataManager dataManager;

DataManager::DataManager() : initialized(false) {}

DataManager::~DataManager() {
    end();
}

bool DataManager::begin() {
    if (initialized) return true;

    if (!preferences.begin("surveillance", false)) {
        Serial.println(" Failed to initialize preferences");
        return false;
    }

    initialized = true;
    
    // Initialize SPIFFS for file storage
    if (!SPIFFS.begin(true)) {
        Serial.println(" Failed to initialize SPIFFS");
        // Continue without SPIFFS - use preferences only
    } else {
        Serial.println(" SPIFFS initialized");
    }

    Serial.println(" DataManager initialized");
    return true;
}

void DataManager::end() {
    if (initialized) {
        preferences.end();
        SPIFFS.end();
        initialized = false;
    }
    Serial.println(" DataManager stopped");
}

void DataManager::saveConfig(const AppConfig& cfg) {
    if (!initialized) return;

    preferences.putFloat("threshold", cfg.system.distance_threshold);
    preferences.putULong("cooldown", cfg.system.alert_cooldown);
    preferences.putInt("refresh", cfg.system.web_refresh_interval);
    preferences.putBool("telegram", cfg.telegram.enable_telegram);
    preferences.putBool("web_server", cfg.system.enable_web_server);
    preferences.putBool("data_logging", cfg.system.enable_data_logging);
    
    // WiFi settings
    preferences.putString("sta_ssid", cfg.wifi.sta_ssid);
    preferences.putString("sta_password", cfg.wifi.sta_password);
    preferences.putBool("enable_station", cfg.wifi.enable_station_mode);
    
    // Telegram settings
    preferences.putString("bot_token", cfg.telegram.bot_token);
    preferences.putString("chat_id", cfg.telegram.chat_id);

    Serial.println(" Configuration saved to preferences");
}

bool DataManager::loadConfig(AppConfig& cfg) {
    if (!initialized) return false;

    // System settings
    cfg.system.distance_threshold = preferences.getFloat("threshold", 50.0);
    cfg.system.alert_cooldown = preferences.getULong("cooldown", 5000);
    cfg.system.web_refresh_interval = preferences.getInt("refresh", 2);
    cfg.system.enable_web_server = preferences.getBool("web_server", true);
    cfg.system.enable_data_logging = preferences.getBool("data_logging", true);
    
    // WiFi settings
    strncpy((char*)cfg.wifi.sta_ssid, preferences.getString("sta_ssid", "").c_str(), 32);
    strncpy((char*)cfg.wifi.sta_password, preferences.getString("sta_password", "").c_str(), 64);
    cfg.wifi.enable_station_mode = preferences.getBool("enable_station", false);
    
    // Telegram settings
    strncpy((char*)cfg.telegram.bot_token, preferences.getString("bot_token", "").c_str(), 64);
    strncpy((char*)cfg.telegram.chat_id, preferences.getString("chat_id", "").c_str(), 32);
    cfg.telegram.enable_telegram = preferences.getBool("telegram", true);

    Serial.println(" Configuration loaded from preferences");
    
    // Log loaded configuration
    Serial.println(" Loaded Config - Threshold: " + String(cfg.system.distance_threshold) + 
                  "cm, Cooldown: " + String(cfg.system.alert_cooldown/1000) + "s");
    
    return true;
}

void DataManager::logEvent(const String& event) {
    if (!initialized || !config.system.enable_data_logging) return;

    // Log to serial
    Serial.println(" Event: " + event);
    
    // Log to file if SPIFFS is available
    if (SPIFFS.exists("/events.log")) {
        File file = SPIFFS.open("/events.log", FILE_APPEND);
        if (file) {
            file.println("[" + String(millis()) + "] " + event);
            file.close();
        }
    }
}

void DataManager::saveSensorData(const SensorData& data) {
    if (!initialized || !config.system.enable_data_logging) return;

    // Save to preferences (limited storage)
    String key = "data_" + String(millis());
    String value = String(data.distance) + "," + 
                   String(data.timestamp) + "," + 
                   String(data.object_detected) + "," + 
                   data.status;
    
    preferences.putString(key.c_str(), value);
    
    // Save to file if SPIFFS is available
    if (SPIFFS.exists("/sensor_data.csv")) {
        File file = SPIFFS.open("/sensor_data.csv", FILE_APPEND);
        if (file) {
            file.println(String(data.timestamp) + "," + 
                        String(data.distance, 2) + "," + 
                        String(data.object_detected) + "," + 
                        data.status);
            file.close();
        }
    }
}

void DataManager::cleanupOldData(int maxAgeDays) {
    if (!initialized) return;

    unsigned long maxAgeMs = maxAgeDays * 24 * 60 * 60 * 1000;
    unsigned long currentTime = millis();
    
    // This is a simplified cleanup - in production you'd want more sophisticated logic
    Serial.println("🧹 Cleaning up data older than " + String(maxAgeDays) + " days");
    
    // Note: Preferences doesn't have built-in timestamp for keys
    // This would need a more complex implementation for production use
    
    logEvent("Data cleanup performed");
}

// Additional utility methods
void DataManager::exportDataToJson() {
    if (!initialized) return;
    
    // This would export data to JSON format for backup/analysis
    Serial.println(" Exporting data to JSON (placeholder)");
}

void DataManager::resetAllData() {
    if (!initialized) return;
    
    preferences.clear();
    Serial.println(" All data reset");
}