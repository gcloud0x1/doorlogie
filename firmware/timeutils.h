#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <WiFi.h>
#include <time.h>
#include "config.h"

bool timeSynced = false;

void setupTime()
{
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
    Serial.print("Syncing time");
    for (int i = 0; i < TIME_SYNC_RETRIES; i++)
    {
        time_t now = time(nullptr);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 1000))
        {
            timeSynced = true;
            Serial.printf("\nTime synced: %02d:%02d:%02d %02d/%02d/%04d",
                         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                         timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
            return;
        }
        Serial.print(".");
        delay(TIME_SYNC_DELAY);
    }
    Serial.println("\nFailed to sync time - Using relative time");
}

String getTime()
{
    if (!timeSynced)
    {
        static unsigned long startMillis = millis();
        unsigned long elapsed = (millis() - startMillis) / 1000;
        return "[U:" + String(elapsed) + "s]";
    }
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100))
    {
        return "[Time Error]";
    }
    
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buf);
}

String getTodayDate()
{
    if (!timeSynced) return "1970-01-01";
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100))
    {
        return "1970-01-01";
    }
    
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &timeinfo);
    return String(buf);
}

#endif
