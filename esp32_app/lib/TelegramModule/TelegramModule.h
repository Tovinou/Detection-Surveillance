#ifndef TELEGRAM_MODULE_H
#define TELEGRAM_MODULE_H

#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include "../../include/state.h"
#include "../../include/config.h"

class TelegramModule {
private:
  WiFiClientSecure* client;
  UniversalTelegramBot* bot;
  unsigned long lastCheckTime;
  bool initialized;
  
  void handleCommand(String chatId, String text);
  String generateStatusMessage();

public:
  TelegramModule();
  ~TelegramModule();
  
  bool begin();
  void stop();
  void handleMessages();
  bool sendAlert(const String& message);
  bool sendMessage(const String& chatId, const String& message);
  bool isInitialized() const { return initialized; }
};

extern TelegramModule telegramBot;

#endif