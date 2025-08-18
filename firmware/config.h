#ifndef CONFIG_H
#define CONFIG_H

#define REED_PIN 4
#define LED_PIN 13
#define SD_CS_PIN 5

#define OLED_ADDRESS 0x3C
#define OLED_SDA 21
#define OLED_SCL 22

#define DEBOUNCE_MS 30
#define TIME_SYNC_RETRIES 10
#define TIME_SYNC_DELAY 1000
#define WIFI_TIMEOUT 20000

#define LOG_FILE "/doorlog.csv"
#define STATS_FILE "/stats.json"

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 19800   // UTC+5:30 (India)
#define DAYLIGHT_OFFSET_SEC 0

#endif