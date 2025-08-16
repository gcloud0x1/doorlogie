#pragma once
#include <WiFi.h>
#include "time.h"
#include "credentials.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // This is for IST (UTC +5:30)
const int daylightOffset_sec = 0;
bool timeSynced = false;

void setupTime()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    timeSynced = true;
}

String getTime()
{
    if (timeSynced)
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            char buffer[25];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
            return String(buffer) + " IST";
        }
    }
    return String(millis());
}
