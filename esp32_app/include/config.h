#ifndef CONFIG_H
#define CONFIG_H

#define UART_BAUD 115200

struct HardwareConfig {
  // UART Pins (for alerts + basic commands)
  int uart_rx_pin = 16;    // Pi TX (Pin 8) → ESP32 RX (GPIO 16)
  int uart_tx_pin = 17;    // Pi RX (Pin 10) → ESP32 TX (GPIO 17)
  
  // SPI Pins (for image streaming)
  int spi_cs_pin = 5;      // Pi CE0 (GPIO 8, Pin 24) → ESP32 GPIO 5 (CS)
  int spi_mosi_pin = 23;   // Pi MOSI (GPIO 10, Pin 19) → ESP32 GPIO 23 (MOSI)
  int spi_miso_pin = 19;   // Pi MISO (GPIO 9, Pin 21) → ESP32 GPIO 19 (MISO)
  int spi_sck_pin = 18;    // Pi SCLK (GPIO 11, Pin 23) → ESP32 GPIO 18 (SCK)
  
  // System Pins
  int status_led_pin = 2;  // Built-in LED or external status LED
  
  // Sensor Pins (if using HC-SR04 ultrasonic sensor)
  int trigger_pin = 4;     // Ultrasonic sensor trigger pin
  int echo_pin = 15;       // Ultrasonic sensor echo pin
  
  // Optional: I2C pins if needed for other sensors
  int i2c_sda_pin = 21;
  int i2c_scl_pin = 22;
};

struct WifiConfig {
  bool enable_station_mode = true;
  char sta_ssid[32] = "TN_XE4199";
  char sta_password[64] = "MykDyekKits7";
  char ap_ssid[32] = "surveillance_system";
  char ap_password[64] = "password";
  int connection_timeout = 10000;
};

struct SystemConfig {
  bool enable_web_server = true;
  float distance_threshold = 40.0;        // cm
  unsigned long alert_cooldown = 30000;   // ms
  int web_refresh_interval = 2;           // seconds
  bool enable_data_logging = true;
  int history_size = 100;
  int sensor_read_interval = 500;         // ms
  
  // Communication settings
  bool enable_uart = true;                // UART communication with Pi
  bool enable_spi = true;                 // SPI communication with Pi
  int spi_clock_speed = 1000000;          // 1 MHz SPI clock
};

struct TelegramConfig {
  bool enable_telegram = false;
  char bot_token[64] = ""; // Your bot token from BotFather
  char chat_id[32] = ""; // Your chat IDs
  unsigned long check_interval = 5000;
};

struct MqttConfig {
  bool enable_mqtt = false;
  char server[64] = "mqtt.broker.com";
  int port = 1883;
  char topic[128] = "esp32/surveillance";
  char username[32] = "";
  char password[32] = "";
};

struct RaspberryPiConfig {
  bool enable_communication = true;
  int uart_baud_rate = 115200;
  int spi_transfer_size = 4096;           // SPI transfer chunk size
  unsigned long heartbeat_interval = 5000; // ms between heartbeats
  int connection_timeout = 10000;         // ms before considering Pi disconnected
};

struct AppConfig {
  HardwareConfig hardware;
  WifiConfig wifi;
  SystemConfig system;
  TelegramConfig telegram;
  MqttConfig mqtt;
  RaspberryPiConfig raspberry_pi;         // New section for Pi communication
};

extern AppConfig config;
void initConfig();

#endif // CONFIG_H
