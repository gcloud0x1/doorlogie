#ifndef DISPLAY_H
#define DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "timeutils.h"
#include "webui.h"
#include "icons.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

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
    display.println("Door Logie");
    display.println("Initializing...");
    display.display();
}

void updateDisplay()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Door Logie");

    // ip
    display.setCursor(0, 10);
    if (WiFi.status() == WL_CONNECTED)
    {
        display.print("IP: ");
        display.println(WiFi.localIP().toString());
    }
    else
    {
        wifiFail();
        display.println("IP: No WiFi");
        delay(2000);
    }

    // door status (update with icon maybe?)
    display.setCursor(0, 20);
    display.print("Status: ");
    display.println(currentDoorState);

    // showing duration
    display.setCursor(0, 30);
    display.print("Last Open: ");
    display.print(lastOpenDuration);
    display.println("s");

    // showing time (bug here)
    display.setCursor(0, 40);
    display.print("Time: ");
    display.println(getTime());

    display.display();
}

#endif