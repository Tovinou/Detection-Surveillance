#ifndef MQTT_MODULE_H
#define MQTT_MODULE_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "../../include/state.h"
#include "../../include/config.h"

class MqttModule {
private:
  WiFiClient* wifiClient;
  PubSubClient* mqttClient;
  bool initialized;
  unsigned long lastReconnectAttempt;
  
  void callback(char* topic, byte* payload, unsigned int length);
  bool reconnect();
  void handleCommand(const String& command);

public:
  MqttModule();
  ~MqttModule();
  
  bool begin();
  void stop();
  void handleClient();
  bool publishData(const SensorData& data);
  bool publishAlert(const String& message);
  bool isConnected() const;
};

extern MqttModule mqttClient;

#endif