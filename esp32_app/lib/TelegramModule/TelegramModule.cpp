#include "TelegramModule.h"



TelegramModule::TelegramModule() 
    : client(nullptr), bot(nullptr), lastCheckTime(0), initialized(false) {}

TelegramModule::~TelegramModule() {
    stop();
}

bool TelegramModule::begin() {
    if (initialized) return true;

    if (config.telegram.enable_telegram && strlen(config.telegram.bot_token) > 0) {
        client = new WiFiClientSecure();
        client->setCACert(TELEGRAM_CERTIFICATE_ROOT);
        bot = new UniversalTelegramBot(config.telegram.bot_token, *client);
        lastCheckTime = millis();
        initialized = true;
        
        Serial.println(" Telegram bot initialized");
        Serial.println(" Bot Token: " + String(config.telegram.bot_token).substring(0, 10) + "...");
        Serial.println(" Chat ID: " + String(config.telegram.chat_id));
    } else {
        Serial.println(" Telegram bot disabled or invalid configuration");
        initialized = false;
    }

    return initialized;
}

void TelegramModule::stop() {
    if (bot) {
        delete bot;
        bot = nullptr;
    }
    if (client) {
        delete client;
        client = nullptr;
    }
    initialized = false;
    Serial.println(" Telegram bot stopped");
}

void TelegramModule::handleMessages() {
    if (!initialized) return;

    if (millis() - lastCheckTime > config.telegram.check_interval) {
        int numNewMessages = bot->getUpdates(bot->last_message_received + 1);

        for (int i = 0; i < numNewMessages; i++) {
            String chatId = String(bot->messages[i].chat_id);
            String text = bot->messages[i].text;
            text.trim();
            text.toLowerCase();

            Serial.println(" Telegram command: " + text + " from: " + chatId);

            // Security check: only respond to authorized chat ID
            if (chatId != config.telegram.chat_id) {
                Serial.println(" Unauthorized chat ID: " + chatId);
                bot->sendMessage(chatId, " Unauthorized access. Your chat ID: " + chatId, "");
                continue;
            }

            handleCommand(chatId, text);
        }

        lastCheckTime = millis();
    }
}

void TelegramModule::handleCommand(String chatId, String text) {
    String response;

    if (text == "/start" || text == "/help") {
        response = " *ESP32 Surveillance Bot Commands:*\n\n";
        response += " `/status` - Current system status\n";
        response += " `/history` - Recent distance readings\n";
        response += " `/config` - System configuration\n";
        response += " `/test` - Send test alert\n";
        response += " `/restart` - Restart system\n";
        response += " `/help` - This help message\n\n";
        response += " _Distance Threshold: " + String(config.system.distance_threshold) + "cm_";

    } else if (text == "/status") {
        response = generateStatusMessage();
        
    } else if (text == "/history") {
        response = " *Recent Distance History:*\n\n";
        int count = min(5, systemState.getHistoryCount());
        for (int i = 0; i < count; i++) {
            SensorData data = systemState.getHistory(i);
            response += "• " + String(data.distance, 1) + "cm - ";
            response += (data.object_detected ? "🚨 Alert" : " Normal");
            response += "\n";
        }
        if (count == 0) response += "No data available yet";
        
    } else if (text == "/config") {
        response = " *System Configuration:*\n\n";
        response += " Threshold: " + String(config.system.distance_threshold) + "cm\n";
        response += " Cooldown: " + String(config.system.alert_cooldown/1000) + "s\n";
        response += " Refresh: " + String(config.system.web_refresh_interval) + "s\n";
        response += " Telegram: " + String(config.telegram.enable_telegram ? "Enabled" : "Disabled") + "\n";
        response += " WiFi Mode: " + systemState.wifiMode + "\n";
        response += " Uptime: " + systemState.getFormattedUptime();
        
    } else if (text == "/test") {
        response = " *Test Alert Sent!*\n\n";
        response += "This is a test notification from your surveillance system.\n";
        response += "System time: " + String(millis()/1000) + "s";
        
    } else if (text == "/restart") {
        response = " *System Restarting...*\n\n";
        response += "ESP32 will restart in 3 seconds.";
        bot->sendMessage(chatId, response, "");
        delay(3000);
        ESP.restart();
        return;
        
    } else {
        response = " Unknown command. Send `/help` for available commands.";
    }

    if (bot->sendMessage(chatId, response, "Markdown")) {
        Serial.println(" Telegram response sent");
    } else {
        Serial.println(" Failed to send Telegram response");
    }
}

String TelegramModule::generateStatusMessage() {
    String message = " *Surveillance System Status*\n\n";
    message += " *Distance:* " + String(systemState.currentData.distance, 1) + " cm\n";
    message += " *Alert:* " + String(systemState.currentData.object_detected ? "ACTIVE 🚨" : "Clear ✅") + "\n";
    message += " *Status:* " + systemState.currentData.status + "\n";
    message += " *Uptime:* " + systemState.getFormattedUptime() + "\n";
    message += " *WiFi:* " + systemState.wifiMode + "\n";
    message += " *Memory:* " + String(ESP.getFreeHeap()) + " bytes\n";
    message += " *Data Points:* " + String(systemState.getHistoryCount()) + "\n\n";
    
    if (systemState.currentData.object_detected) {
        message += " *Object detected within " + String(config.system.distance_threshold) + "cm!*";
    } else {
        message += " *System monitoring normally*";
    }
    
    return message;
}

bool TelegramModule::sendAlert(const String& message) {
    if (!initialized) return false;

    // Check cooldown period
    unsigned long currentTime = millis();
    if (currentTime - systemState.lastAlertTime < config.system.alert_cooldown) {
        Serial.println(" Alert cooldown active, skipping Telegram alert");
        return false;
    }

    String fullMessage = " *SURVEILLANCE ALERT* 🚨\n\n";
    fullMessage += message + "\n\n";
    fullMessage += " Current Distance: " + String(systemState.currentData.distance, 1) + "cm\n";
    fullMessage += " Time: " + systemState.getFormattedUptime() + "\n";
    fullMessage += " Threshold: " + String(config.system.distance_threshold) + "cm";

    bool success = bot->sendMessage(config.telegram.chat_id, fullMessage, "Markdown");
    
    if (success) {
        Serial.println(" Telegram alert sent successfully");
        systemState.lastAlertTime = currentTime;
    } else {
        Serial.println(" Failed to send Telegram alert");
    }
    
    return success;
}

bool TelegramModule::sendMessage(const String& chatId, const String& message) {
    if (!initialized) return false;
    return bot->sendMessage(chatId, message, "");
}