#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Colors (I feel it's better to use primary colours)
#define COLOR_OFF pixels.Color(0, 0, 0)
// White for booting (this will only be visible for less than a second)
#define COLOR_BOOT pixels.Color(255, 255, 255)
// Blue for WiFi connecting
#define COLOR_WIFI_CONN pixels.Color(0, 0, 255)
// Green for WiFi connected
#define COLOR_WIFI_OK pixels.Color(0, 255, 0)
// Yellow for time syncing
#define COLOR_TIME_SYNC pixels.Color(255, 255, 0)
// Red for time sync failed
#define COLOR_TIME_FAIL pixels.Color(255, 0, 0)
// Red for SD fail
#define COLOR_SD_FAIL pixels.Color(255, 0, 0)
// Red for door open
#define COLOR_DOOR_OPEN pixels.Color(255, 0, 0)
// Green for door closed
#define COLOR_DOOR_CLS pixels.Color(0, 255, 0)
// Red for general error
#define COLOR_ERROR pixels.Color(255, 0, 0)

void setupNeoPixel()
{
    pixels.begin();
    pixels.clear();
    pixels.show();
}

void setStatusColor(uint32_t color, bool blink = false)
{
    pixels.clear();
    if (blink)
    {
        pixels.setPixelColor(0, color);
        pixels.show();
        delay(500);

        pixels.setPixelColor(0, COLOR_OFF);
        pixels.show();
        delay(500);
    }
    else
    {
        pixels.setPixelColor(0, color);
        pixels.show();
    }
}

void pulseColor(uint32_t color)
{
    for (int i = 0; i < 256; i += 5)
    {
        pixels.setPixelColor(0, pixels.Color(
            (color >> 16 & 0xFF) * i / 255,
            (color >> 8 & 0xFF) * i / 255,
            (color & 0xFF) * i / 255
        ));
        pixels.show();
        delay(10);
    }

    for (int i = 255; i >= 0; i -= 5)
    {
        pixels.setPixelColor(0, pixels.Color(
            (color >> 16 & 0xFF) * i / 255,
            (color >> 8 & 0xFF) * i / 255,
            (color & 0xFF) * i / 255
        ));
        pixels.show();
        delay(10);
    }
}

#endif