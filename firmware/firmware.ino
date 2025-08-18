#include "config.h"
#include "credentials.h"
#include "timeutils.h"
#include "sdlogger.h"
#include "webui.h"
#include "display.h"

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
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    Wire.begin(OLED_SDA, OLED_SCL);
    setupDisplay();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_TIMEOUT) 
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
        setupTime();
    }
    else
    {
        Serial.println("\nWiFi connection failed");
    }

    if (!initSD())
    {
        Serial.println("SD card initialization failed!");
        while (1)
        {
            delay(1000);
        }
    }

    setupWebUI();

    lastRawState = digitalRead(REED_PIN);
    currentDoorState = lastRawState == HIGH ? "OPEN" : "CLOSED";
    Serial.println("System ready. Initial state: " + currentDoorState);

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
            digitalWrite(LED_PIN, currentState ? HIGH : LOW);
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
                Serial.printf("Door CLOSED after %lu seconds at %s\n",
                              lastDuration, getTime().c_str());
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
