#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_task_wdt.h>

// Include all modules
#include "../include/config.h"
#include "../include/state.h"
#include "../lib/HtmlPage/html_page.h"

#include "../lib/WebServerModule/WebServerModule.h"
#include "../lib/TelegramModule/TelegramModule.h"
#include "../lib/MqttModule/MqttModule.h"
#include "../lib/UartModule/UartModule.h"
#include "../lib/SpiModule/SpiModule.h"
#include "../lib/DataManager/DataManager.h"
#include "../lib/PiCommunication Module/PiCommunication.h"

// Global instances
WebServerModule webServer;
TelegramModule telegramBot;
MqttModule mqttClient;

// System status variables
bool systemInitialized = false;
unsigned long lastReconnectAttempt = 0;
int reconnectAttempts = 0;
const int MAX_RECONNECT_ATTEMPTS = 5;

// WiFi setup function
void setupWiFi() {
  Serial.println("\n🌐 Setting up WiFi...");
  
  if (config.wifi.enable_station_mode && strlen(config.wifi.sta_ssid) > 0) {
    WiFi.begin(config.wifi.sta_ssid, config.wifi.sta_password);
    Serial.print("🔌 Connecting to WiFi: " + String(config.wifi.sta_ssid));
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startTime < config.wifi.connection_timeout) {
      delay(500);
      Serial.print(".");
      
      // Blink status LED while connecting
      digitalWrite(config.hardware.status_led_pin, !digitalRead(config.hardware.status_led_pin));
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      systemState.wifiConnected = true;
      systemState.wifiMode = "Station";
      systemState.wifiSignalStrength = WiFi.RSSI();
      Serial.println("\n✅ WiFi connected! IP: " + WiFi.localIP().toString());
      digitalWrite(config.hardware.status_led_pin, LOW);
      return;
    } else {
      Serial.println("\n❌ Failed to connect to WiFi");
      digitalWrite(config.hardware.status_led_pin, HIGH);
    }
  }
  
  // Fallback to AP mode
  Serial.println("📡 Starting AP Mode...");
  WiFi.softAP(config.wifi.ap_ssid, config.wifi.ap_password);
  systemState.wifiConnected = true;
  systemState.wifiMode = "AP";
  systemState.wifiSignalStrength = 0;
  Serial.println("✅ AP Mode - IP: " + WiFi.softAPIP().toString());
}

// Process incoming sensor data from Pi
void processPiSensorData() {
  piComm.handle();
}

// Process alerts and notifications
void processAlerts() {
  // Check if we need to send alerts for object detection
  if (systemState.currentData.object_detected) {
    unsigned long currentTime = millis();
    
    // Check alert cooldown
    if (currentTime - systemState.lastAlertTime > config.system.alert_cooldown) {
      String alertMessage = "🚨 Object detected at " + 
                           String(systemState.currentData.distance, 1) + "cm";
      
      // Send Telegram alert
      if (config.telegram.enable_telegram) {
        if (telegramBot.sendAlert(alertMessage)) {
          Serial.println("📱 Telegram alert sent");
        } else {
          Serial.println("❌ Failed to send Telegram alert");
        }
      }
      
      // Send MQTT alert
      if (config.mqtt.enable_mqtt) {
        if (mqttClient.publishAlert(alertMessage)) {
          Serial.println("📡 MQTT alert sent");
        } else {
          Serial.println("❌ Failed to send MQTT alert");
        }
      }
      
      systemState.lastAlertTime = currentTime;
      Serial.println("🔔 Alert sent: " + alertMessage);
    }
  }
}

// System initialization
void setupSystem() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n🚀 ESP32 Surveillance System v3.0");
  Serial.println("=================================");
  
  // Initialize configuration
  initConfig();
  
  // Initialize status LED
  pinMode(config.hardware.status_led_pin, OUTPUT);
  digitalWrite(config.hardware.status_led_pin, HIGH);
  
  // Initialize data manager first (loads config)
  if (!dataManager.begin()) {
    Serial.println("❌ DataManager initialization failed!");
  }
  
  // Load configuration
  dataManager.loadConfig(config);
  
  // FORCE CONFIG OVERRIDES (Fix for mismatch and errors)
  config.system.distance_threshold = 40.0;
  config.telegram.enable_telegram = false;
  Serial.println("ℹ️ Config forced: Threshold=40cm, Telegram=Disabled");
  
  // Setup WiFi
  setupWiFi();
  
  // Initialize Pi communication
  if (config.raspberry_pi.enable_communication) {
    piComm.begin();
    Serial.println("✅ Pi communication initialized");
  }
  
  // Initialize HTML Page dependencies
  HtmlPage::setSystemState(&systemState);
  HtmlPage::setConfig(&config);
  
  // Initialize service modules
  Serial.println("\n🔧 Initializing service modules...");
  if (config.system.enable_web_server) {
    if (webServer.begin()) {
      Serial.println("✅ Web server started");
    } else {
      Serial.println("❌ Failed to start web server");
    }
  }
  
  if (config.telegram.enable_telegram) {
    if (telegramBot.begin()) {
      Serial.println("✅ Telegram bot initialized");
    } else {
      Serial.println("❌ Failed to initialize Telegram bot");
    }
  }
  
  if (config.mqtt.enable_mqtt) {
    if (mqttClient.begin()) {
      Serial.println("✅ MQTT client initialized");
    } else {
      Serial.println("❌ Failed to initialize MQTT client");
    }
  }
  
  // Configure time for timestamps
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  // Initialize system state
  systemState.currentData.timestamp = millis();
  systemState.currentData.status = "System Ready ✅";
  systemState.piConnected = false;
  
  // Enable watchdog timer for system stability
  esp_task_wdt_init(10, true);
  esp_task_wdt_add(NULL);
  
  digitalWrite(config.hardware.status_led_pin, LOW);
  systemInitialized = true;
  
  Serial.println("\n✅ System initialization complete!");
  String dashboardIP = (systemState.wifiMode == "Station") ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  Serial.println("📡 Dashboard available at: http://" + dashboardIP);
  Serial.println("=================================\n");
}

// Main loop
void loop() {
  // Feed the watchdog to prevent system reset
  esp_task_wdt_reset();
  
  // Update system uptime
  systemState.updateUptime();
  
  // Process sensor data from Raspberry Pi
  processPiSensorData();
  
  // Process alerts and notifications
  processAlerts();
  
  // Handle Telegram messages
  if (config.telegram.enable_telegram) {
    telegramBot.handleMessages();
  }
  
  // Handle MQTT client
  if (config.mqtt.enable_mqtt) {
    mqttClient.handleClient();
  }
  
  // System status LED blink (slow blink when normal, fast when alert)
  static unsigned long lastBlink = 0;
  unsigned long blinkInterval = systemState.currentData.object_detected ? 200 : 1000;
  
  if (millis() - lastBlink > blinkInterval) {
    digitalWrite(config.hardware.status_led_pin, !digitalRead(config.hardware.status_led_pin));
    lastBlink = millis();
  }
  
  // System health monitoring
  static unsigned long lastHealthCheck = 0;
  if (millis() - lastHealthCheck > 30000) { // Every 30 seconds
    Serial.println("💓 System Health - Uptime: " + systemState.getFormattedUptime() +
                  " | Free RAM: " + String(ESP.getFreeHeap()) + " bytes" +
                  " | Pi Connected: " + String(systemState.isPiConnected() ? "Yes" : "No") +
                  " | Distance: " + String(systemState.currentData.distance, 1) + "cm");
    lastHealthCheck = millis();
  }
  
  // Brief delay to prevent overwhelming the system
  delay(50);
}

// Setup function
void setup() {
  setupSystem();
}
