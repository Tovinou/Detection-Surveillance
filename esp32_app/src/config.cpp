#include "config.h"
#include <cstring>

// Initialize the global config object with your specific hardware pins
AppConfig config;

void initConfig() {
  // Hardware configuration
  config.hardware.uart_rx_pin = 16;
  config.hardware.uart_tx_pin = 17;
  config.hardware.spi_cs_pin = 5;
  config.hardware.spi_mosi_pin = 23;
  config.hardware.spi_miso_pin = 19;
  config.hardware.spi_sck_pin = 18;
  config.hardware.status_led_pin = 2;
  config.hardware.trigger_pin = 4;
  config.hardware.echo_pin = 15;
  config.hardware.i2c_sda_pin = 21;
  config.hardware.i2c_scl_pin = 22;

  // WiFi configuration
  config.wifi.enable_station_mode = true;
  strcpy(config.wifi.sta_ssid, "xxx");
  strcpy(config.wifi.sta_password, "xxx");
  strcpy(config.wifi.ap_ssid, "surveillance_system");
  strcpy(config.wifi.ap_password, "password");
  config.wifi.connection_timeout = 10000;

  // System configuration
  config.system.enable_web_server = true;
  config.system.distance_threshold = 50.0;
  config.system.alert_cooldown = 30000;
  config.system.web_refresh_interval = 2;
  config.system.enable_data_logging = true;
  config.system.history_size = 100;
  config.system.sensor_read_interval = 500;
  config.system.enable_uart = true;
  config.system.enable_spi = true;
  config.system.spi_clock_speed = 1000000;

  // Telegram configuration
  config.telegram.enable_telegram = false;
  strcpy(config.telegram.bot_token, "");
  strcpy(config.telegram.chat_id, "");
  config.telegram.check_interval = 5000;

  // MQTT configuration
  config.mqtt.enable_mqtt = false;
  strcpy(config.mqtt.server, "mqtt.broker.com");
  config.mqtt.port = 1883;
  strcpy(config.mqtt.topic, "esp32/surveillance");
  strcpy(config.mqtt.username, "");
  strcpy(config.mqtt.password, "");

  // Raspberry Pi communication configuration
  config.raspberry_pi.enable_communication = true;
  config.raspberry_pi.uart_baud_rate = 115200;
  config.raspberry_pi.spi_transfer_size = 4096;
  config.raspberry_pi.heartbeat_interval = 5000;
  config.raspberry_pi.connection_timeout = 10000;
}