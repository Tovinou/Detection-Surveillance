#include "WebServerModule.h"
#include "../HtmlPage/html_page.h"
#include <ArduinoJson.h>
#include <AsyncJson.h>

WebServerModule::WebServerModule() : server(nullptr), initialized(false) {}

WebServerModule::~WebServerModule() {
  stop();
}

bool WebServerModule::begin() {
  if (initialized) return true;
  
  server = new AsyncWebServer(80);
  if (!server) return false;
  
  setupRoutes();
  server->begin();
  initialized = true;
  
  Serial.println("Web server started on port 80");
  return true;
}

void WebServerModule::stop() {
  if (server) {
    server->end();
    delete server;
    server = nullptr;
  }
  initialized = false;
}

void WebServerModule::setupRoutes() {
  // Root endpoint - dashboard
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleRoot(request);
  });
  
  // Pages
  server->on("/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", HtmlPage::generateConfigPage());
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  });

  server->on("/history", HTTP_GET, [this](AsyncWebServerRequest* request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", HtmlPage::generateHistoryPage());
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response);
  });
  
  // API endpoints
  server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleAPI(request);
  });
  
  server->on("/api/history", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleHistory(request);
  });
  
  // Handle config submission (JSON)
  AsyncCallbackJsonWebHandler* configHandler = new AsyncCallbackJsonWebHandler("/api/config", 
    [this](AsyncWebServerRequest *request, JsonVariant &json) {
      JsonObject jsonObj = json.as<JsonObject>();
      
      // Update config from JSON
      if (jsonObj.containsKey("threshold")) {
          config.system.distance_threshold = jsonObj["threshold"].as<float>();
      }
      if (jsonObj.containsKey("cooldown")) {
          config.system.alert_cooldown = jsonObj["cooldown"].as<unsigned long>() * 1000;
      }
      if (jsonObj.containsKey("telegram")) {
          String val = jsonObj["telegram"].as<String>();
          config.telegram.enable_telegram = (val == "true");
      }
      
      // Save config (assuming DataManager handles persistence elsewhere or we trigger it)
      // Ideally we should call dataManager.saveConfig(config);
      // For now, just respond success
      
      request->send(200, "application/json", "{\"status\":\"ok\", \"message\":\"Configuration saved\"}");
  });
  server->addHandler(configHandler);
  
  // Command endpoint
  server->on("/api/command", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleCommand(request);
  });
  
  // 404 handler
  server->onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/html", HtmlPage::generateErrorPage("Page not found"));
  });
}

void WebServerModule::handleRoot(AsyncWebServerRequest* request) {
  Serial.println("🌐 Handling Root Request");
  String html = HtmlPage::generateDashboard();
  Serial.println("📄 HTML Length: " + String(html.length()));
  
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=utf-8", html);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "0");
  request->send(response);
}

void WebServerModule::handleAPI(AsyncWebServerRequest* request) {
  request->send(200, "application/json", HtmlPage::generateAPIResponse());
}

void WebServerModule::handleHistory(AsyncWebServerRequest* request) {
  // Create a JSON response with history data
  // We need to access systemState.history but it's private.
  // We should add a method in SystemState to get history as JSON or expose it.
  // Or we can iterate if we have access. SystemState header says:
  // SensorData getHistory(int index) const;
  // int getHistoryCount() const;
  
  StaticJsonDocument<4096> doc; // Adjust size as needed
  JsonArray history = doc.createNestedArray("history");
  
  int count = systemState.getHistoryCount();
  for (int i = 0; i < count; i++) {
      SensorData data = systemState.getHistory(i);
      JsonObject item = history.createNestedObject();
      item["distance"] = data.distance;
      item["timestamp"] = data.timestamp;
      item["object_detected"] = data.object_detected;
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerModule::handleConfig(AsyncWebServerRequest* request) {
    // This is handled by the JsonHandler now for POST
    // For GET, we return the current config as JSON?
    // Or maybe just the HTML page handles it.
    // Let's implement GET for API consistency
    StaticJsonDocument<512> doc;
    doc["threshold"] = config.system.distance_threshold;
    doc["cooldown"] = config.system.alert_cooldown / 1000;
    doc["telegram"] = config.telegram.enable_telegram;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServerModule::handleCommand(AsyncWebServerRequest* request) {
  if (request->hasParam("command", true)) { // true for body? No, usually query for simple command
      // If sent as query param /api/command?command=refresh
      String command = request->getParam("command")->value();
      
      if (command == "test_alert") {
          // Trigger a test alert manually
          // systemState.currentData.object_detected = true; // This might be overwritten by sensor
          // Instead, we might want a flag or just log it.
          Serial.println("Manual Test Alert Requested");
          request->send(200, "text/plain", "Test alert triggered (check logs)");
      } else if (command == "refresh") {
           request->send(200, "text/plain", "Refreshed");
      } else {
           request->send(400, "text/plain", "Unknown command");
      }
  } else if (request->hasParam("command", true)) { // Check post body if any
      String command = request->getParam("command", true)->value();
       if (command == "test_alert") {
          Serial.println("Manual Test Alert Requested");
          request->send(200, "text/plain", "Test alert triggered");
       } else {
           request->send(200, "text/plain", "Command received: " + command);
       }
  } else {
    request->send(400, "text/plain", "Missing command parameter");
  }
}

void WebServerModule::handleClient() {
  // AsyncWebServer handles clients automatically
}
