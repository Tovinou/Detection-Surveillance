#include "MqttModule.h"
#include <ArduinoJson.h>


MqttModule::MqttModule() 
    : wifiClient(nullptr), mqttClient(nullptr), initialized(false), lastReconnectAttempt(0) {}

MqttModule::~MqttModule() {
    stop();
}

bool MqttModule::begin() {
    if (initialized) return true;

    if (config.mqtt.enable_mqtt) {
        wifiClient = new WiFiClient();
        mqttClient = new PubSubClient(*wifiClient);
        mqttClient->setServer(config.mqtt.server, config.mqtt.port);
        mqttClient->setCallback([this](char* topic, byte* payload, unsigned int length) {
            this->callback(topic, payload, length);
        });
        
        lastReconnectAttempt = 0;
        initialized = true;
        
        Serial.println(" MQTT client initialized");
        Serial.println(" Server: " + String(config.mqtt.server) + ":" + String(config.mqtt.port));
        Serial.println(" Topic: " + String(config.mqtt.topic));
        
        // Attempt initial connection
        reconnect();
    } else {
        Serial.println(" MQTT disabled in configuration");
        initialized = false;
    }

    return initialized;
}

void MqttModule::stop() {
    if (mqttClient && mqttClient->connected()) {
        mqttClient->disconnect();
    }
    
    if (mqttClient) {
        delete mqttClient;
        mqttClient = nullptr;
    }
    
    if (wifiClient) {
        delete wifiClient;
        wifiClient = nullptr;
    }
    
    initialized = false;
    Serial.println(" MQTT client stopped");
}

void MqttModule::callback(char* topic, byte* payload, unsigned int length) {
    Serial.println(" MQTT message received on topic: " + String(topic));
    
    // Convert payload to string
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.println(" Message: " + message);
    
    // Handle different topics
    if (String(topic) == String(config.mqtt.topic) + "/command") {
        handleCommand(message);
    }
}

void MqttModule::handleCommand(const String& command) {
    String lowerCaseCommand = command;
    lowerCaseCommand.toLowerCase();
    
    if (lowerCaseCommand == "status") {
        publishData(systemState.currentData);
    } else if (command == "test_alert") {
        publishAlert("Test alert via MQTT");
    } else if (command == "restart") {
        Serial.println(" Restart command received via MQTT");
        ESP.restart();
    }
}

bool MqttModule::reconnect() {
    if (!initialized) return false;

    if (mqttClient->connected()) {
        return true;
    }

    Serial.println(" Attempting MQTT connection...");
    
    String clientId = "ESP32Surveillance-" + String(random(0xffff), HEX);
    
    if (mqttClient->connect(clientId.c_str())) {
        Serial.println(" MQTT connected");
        
        // Subscribe to command topic
        String commandTopic = String(config.mqtt.topic) + "/command";
        if (mqttClient->subscribe(commandTopic.c_str())) {
            Serial.println(" Subscribed to: " + commandTopic);
        } else {
            Serial.println(" Failed to subscribe to: " + commandTopic);
        }
        
        // Publish connection message
        publishAlert("System connected via MQTT");
        
        return true;
    } else {
        Serial.println(" MQTT connection failed, rc=" + String(mqttClient->state()));
        return false;
    }
}

void MqttModule::handleClient() {
    if (!initialized) return;

    if (!mqttClient->connected()) {
        unsigned long currentTime = millis();
        if (currentTime - lastReconnectAttempt > 5000) { // Try every 5 seconds
            lastReconnectAttempt = currentTime;
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient->loop();
    }
}

bool MqttModule::publishData(const SensorData& data) {
    if (!initialized || !mqttClient->connected()) {
        Serial.println(" MQTT not connected, cannot publish data");
        return false;
    }

    StaticJsonDocument<512> doc;
    doc["distance"] = data.distance;
    doc["object_detected"] = data.object_detected;
    doc["status"] = data.status;
    doc["timestamp"] = data.timestamp;
    doc["uptime"] = systemState.systemUptime;
    doc["free_memory"] = ESP.getFreeHeap();

    String jsonStr;
    serializeJson(doc, jsonStr);

    bool success = mqttClient->publish(config.mqtt.topic, jsonStr.c_str());
    
    if (success) {
        Serial.println(" MQTT data published: " + String(data.distance, 1) + "cm");
    } else {
        Serial.println(" MQTT publish failed");
    }
    
    return success;
}

bool MqttModule::publishAlert(const String& message) {
    if (!initialized || !mqttClient->connected()) return false;

    String alertTopic = String(config.mqtt.topic) + "/alert";
    
    StaticJsonDocument<256> doc;
    doc["message"] = message;
    doc["distance"] = systemState.currentData.distance;
    doc["timestamp"] = millis();
    doc["uptime"] = systemState.systemUptime;

    String jsonStr;
    serializeJson(doc, jsonStr);

    bool success = mqttClient->publish(alertTopic.c_str(), jsonStr.c_str());
    
    if (success) {
        Serial.println(" MQTT alert published: " + message);
    } else {
        Serial.println(" MQTT alert publish failed");
    }
    
    return success;
}

bool MqttModule::isConnected() const {
    return initialized && mqttClient && mqttClient->connected();
}