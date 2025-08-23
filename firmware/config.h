#ifndef CONFIG_H
#define CONFIG_H

// Pins
#define REED_PIN 4
#define NEOPIXEL_PIN 13   // Using NeoPixel instead of LED, same pin as before
#define SD_CS_PIN 5
#define OLED_ADDRESS 0x3C // You can check back of your OLED (where capacitor is connected)
#define OLED_SDA 21
#define OLED_SCL 22

// Settings
#define DEBOUNCE_MS 30
#define TIME_SYNC_RETRIES 10
#define TIME_SYNC_DELAY 1000
#define WIFI_TIMEOUT 20000
#define TIME_SYNC_TIMEOUT 10000
#define NEOPIXEL_COUNT 1   // Single NeoPixel (adjust if using strip or ring)

// Files
#define LOG_FILE "/doorlog.csv"
#define STATS_FILE "/stats.json"

// Time  
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 19800   // UTC+5:30 (India)
#define DAYLIGHT_OFFSET_SEC 0

#endif