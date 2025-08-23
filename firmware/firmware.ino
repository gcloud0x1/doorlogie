#include "config.h"
#include "credentials.h"
#include "timeutils.h"
#include "sdlogger.h"
#include "webui.h"
#include "display.h"
#include "neopixel.h"
#include "icons.h"

volatile unsigned long doorOpenTime = 0;
volatile unsigned long lastDuration = 0;
volatile int lastRawState = HIGH;
volatile bool lastStableState = false;
volatile unsigned long lastDebounceTime = 0;

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n=== Door Logie ===");

    pinMode(REED_PIN, INPUT_PULLUP);

    setupNeoPixel();
    setStatusColor(COLOR_BOOT);

    // OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    setupDisplay();

    // WiFi
    setStatusColor(COLOR_WIFI_CONN);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    unsigned long wifiStart = millis();

    const int WIFI_SEARCH_FRAMES = 3000 / 32;

    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_TIMEOUT)
    {
        pulseColor(COLOR_WIFI_CONN);

        for (int i = 0; i < WIFI_SEARCH_FRAMES; i++)
        {
            wifiSearching();
            delay(32);

            if (WiFi.status() == WL_CONNECTED)
            {
                break;
            }
        }

        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        setStatusColor(COLOR_WIFI_OK);
        Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

        wifiSuccess();
        delay(2000);

        setStatusColor(COLOR_TIME_SYNC);
        
        unsigned long timeSyncStart = millis();
        while (millis() - timeSyncStart < 3000)
        {
            timeSync();
            delay(32);
        }

        setupTime();

        if (timeSynced)
        {
            setStatusColor(COLOR_WIFI_OK);
            timeSuccess();
            delay(2000);
        }
        else
        {
            setStatusColor(COLOR_TIME_FAIL);
            delay(2000);
            timeFail();
            delay(2000);
        }
    }
    else
    {
        setStatusColor(COLOR_ERROR);
        
        wifiFail();
        delay(2000);

        Serial.println("\nWiFi connection failed");
        delay(2000);
    }

    // SD
    if (!initSD())
    {
        setStatusColor(COLOR_SD_FAIL);
        Serial.println("SD card initialization failed!");
        sdFail();
        while (1)
        {
            pulseColor(COLOR_SD_FAIL);
        }
    }

    setupWebUI();

    lastRawState = digitalRead(REED_PIN);
    currentDoorState = lastRawState == HIGH ? "OPEN" : "CLOSED";
    Serial.println("System ready. Initial state: " + currentDoorState);

    setStatusColor(lastRawState == HIGH ? COLOR_DOOR_OPEN : COLOR_DOOR_CLS);

    updateDisplay();
}

void loop()
{
    static unsigned long lastStatusBroadcast = 0;
    static unsigned long lastDisplayUpdate = 0;

    int currentReading = digitalRead(REED_PIN);
    if (currentReading != lastRawState)
    {
        lastDebounceTime = millis();
        lastRawState = currentReading;
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_MS)
    {
        bool currentState = (lastRawState == HIGH);

        if (currentState != lastStableState)
        {
            lastStableState = currentState;
            setStatusColor(currentState ? COLOR_DOOR_OPEN : COLOR_DOOR_CLS);
            currentDoorState = currentState ? "OPEN" : "CLOSED";

            if (currentState)
            {
                doorOpenTime = millis();
                updateDailyStats();
                Serial.println("Door OPENED at " + getTime());
            }
            else if (doorOpenTime > 0)
            {
                lastDuration = (millis() - doorOpenTime) / 1000;
                lastOpenDuration = lastDuration;
                logEvent("CLOSED", lastDuration);
                doorOpenTime = 0;
                Serial.printf("Door CLOSED after %lu seconds at %s\n", lastDuration, getTime().c_str());
            }

            String statusUpdate = currentDoorState + "|" + String(lastOpenDuration);
            webSocket.broadcastTXT(statusUpdate);
            updateDisplay();
        }
    }

    if (millis() - lastStatusBroadcast >= 1000)
    {
        String statusUpdate = currentDoorState + "|" + String(lastOpenDuration);
        webSocket.broadcastTXT(statusUpdate);
        lastStatusBroadcast = millis();
    }

    if (millis() - lastDisplayUpdate >= 1000)
    {
        updateDisplay();
        lastDisplayUpdate = millis();
    }

    handleWeb();
    delay(10);
}