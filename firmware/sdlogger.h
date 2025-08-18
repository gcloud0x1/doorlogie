#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <SD.h>
#include <ArduinoJson.h>
#include "config.h"
#include "timeutils.h"

bool initSD()
{
    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println("SD card initialization failed!");
        return false;
    }
    
    if (!SD.exists(LOG_FILE))
    {
        File f = SD.open(LOG_FILE, FILE_WRITE);
        if (!f)
        {
            Serial.println("Failed to create log file");
            return false;
        }
        f.println("Timestamp,State,Duration(sec)");
        f.close();
    }
    
    if (!SD.exists(STATS_FILE))
    {
        File f = SD.open(STATS_FILE, FILE_WRITE);
        if (!f)
        {
            Serial.println("Failed to create stats file");
            return false;
        }
        f.println("{}");
        f.close();
    }
    
    return true;
}

void logEvent(const String &state, unsigned long duration = 0)
{
    File f = SD.open(LOG_FILE, FILE_APPEND);
    if (!f) return;
    
    String logLine = getTime() + "," + state;
    if (duration > 0) logLine += "," + String(duration);
    
    f.println(logLine);
    f.close();
}

void updateDailyStats()
{
    StaticJsonDocument<1024> doc;
    
    if (SD.exists(STATS_FILE))
    {
        File f = SD.open(STATS_FILE, FILE_READ);
        if (f)
        {
            DeserializationError error = deserializeJson(doc, f);
            if (error)
            {
                Serial.println("Failed to parse stats: " + String(error.c_str()));
            }
            f.close();
        }
    }

    String today = getTodayDate();
    if (today.length() == 10)
    {
        int currentCount = doc[today] | 0;
        doc[today] = currentCount + 1;
        
        File f = SD.open(STATS_FILE, FILE_WRITE);
        if (f)
        {
            serializeJson(doc, f);
            f.close();
            Serial.println("Stats updated: " + today + " = " + String(currentCount + 1));
        }
    }
}

bool clearLogs()
{
    bool success = true;
    if (SD.exists(LOG_FILE)) success &= SD.remove(LOG_FILE);
    if (SD.exists(STATS_FILE)) success &= SD.remove(STATS_FILE);
    return success && initSD();
}

#endif
