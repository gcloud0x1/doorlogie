#ifndef CONFIG_H
#define CONFIG_H

// Pins
#define REED_PIN 4
#define LED_PIN 13
#define SD_CS_PIN 5

// Settings
#define DEBOUNCE_MS 30
#define TIME_SYNC_RETRIES 10
#define TIME_SYNC_DELAY 1000
#define WIFI_TIMEOUT 20000

// Files
#define LOG_FILE "/doorlog.csv"
#define STATS_FILE "/stats.json"

// Time
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 19800  // UTC+5:30 (This is for India)
#define DAYLIGHT_OFFSET_SEC 0

#endif