#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "timeutils.h"
#include "webui.h"
#include "icons.h"
#include "neopixel.h"

#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SLIDE_INTERVAL 3000

unsigned long lastSlideChange = 0;
int displayPage = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setupDisplay()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("SSD1306 allocation failed");
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setFont(&FreeSerifBold12pt7b);
    display.setCursor(5, 35);
    display.println(F("Door Logie"));
    display.setFont();
    display.display();
}

void checkWifi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi lost. Reconnecting...");
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        unsigned long startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_TIMEOUT)
        {
            for (int i = 0; i < WIFI_FRAME_COUNT; i++)
            {
                wifiSearching();
                
                if (WiFi.status() == WL_CONNECTED) break;
            }
            pulseColor(COLOR_WIFI_CONN);
            delay(FRAME_DELAY);
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("WiFi reconnected!");
            wifiSuccess();
            delay(2000);
        }
        else
        {
            Serial.println("WiFi reconnect failed");
            wifiFail();
            delay(2000);
        }
    }
}


void updateDisplay()
{
    unsigned long now = millis();

    if (now - lastSlideChange > SLIDE_INTERVAL) 
    {
        displayPage = (displayPage + 1) % 5;
        lastSlideChange = now;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    switch (displayPage) 
    {
        case 0:
            display.setFont(&FreeSerifBold12pt7b);
            display.setCursor(5, 35);
            display.println(F("Door Logie"));
            display.setFont();
            break;

        case 1:
            if (WiFi.status() == WL_CONNECTED) 
            {
                display.setFont(&FreeSerifBold12pt7b);
                display.setCursor(0, 15);
                display.println(F("IP :"));

                display.setFont(&FreeSerifBold9pt7b);
                display.setCursor(0, 40);
                display.println(WiFi.localIP().toString());
                display.setFont();
            }
            else
            {
                wifiFail();
                checkWifi();
                return;
            }
            break;

        case 2:
            display.setFont(&FreeSerifBold12pt7b);
            display.setCursor(0, 20);
            display.println(F("Last Open"));

            display.setFont(&FreeSerifBold12pt7b);
            display.setCursor(0, 50);
            display.print(lastOpenDuration);
            display.println(F("s"));
            display.setFont();
            break;

        case 3:
            display.setFont(&FreeSerifBold12pt7b);
            display.setCursor(0, 15);
            display.println(F("Status :"));

            display.setFont(&FreeSerifBold12pt7b);
            display.setCursor(0, 35);
            display.println(currentDoorState);
            display.setFont();
            break;

        case 4:
            display.setFont(&FreeSerifBold12pt7b);
            display.setCursor(0, 15);
            display.println(F("Date-Time :"));

            display.setFont(&FreeSerifBold9pt7b);
            String timeStr = getTime();
            String dateStr = timeStr.substring(0, 10);
            String hourStr = timeStr.substring(11);
            display.setCursor(0, 35);
            display.println(dateStr);
            display.setCursor(0, 55);
            display.println(hourStr);
            display.setFont();
            break;
    }

    display.display();
}

#endif