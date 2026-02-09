#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "../../include/state.h"
#include "../../include/config.h"

class WebServerModule {
private:
  AsyncWebServer* server;
  bool initialized = false;
  
  void setupRoutes();
  void handleRoot(AsyncWebServerRequest* request);
  void handleAPI(AsyncWebServerRequest* request);
  void handleConfig(AsyncWebServerRequest* request);
  void handleHistory(AsyncWebServerRequest* request);
  void handleCommand(AsyncWebServerRequest* request);

public:
  WebServerModule();
  ~WebServerModule();
  
  bool begin();
  void stop();
  bool isRunning() const { return initialized; }
  void handleClient();
};

extern WebServerModule webServer;

#endif