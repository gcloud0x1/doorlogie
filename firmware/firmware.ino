#define REED_PIN 4
#define LED_PIN 13
const unsigned long DEBOUNCE_MS = 30;

int lastRaw = HIGH;
unsigned long lastDebounceTime = 0;
bool doorOpen = false;

void setup()
{
  Serial.begin(115200);
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
  int reading = digitalRead(REED_PIN);
  if (reading != lastRaw)
  {
    lastDebounceTime = millis();
    lastRaw = reading;
  }

  if (millis() - lastDebounceTime > DEBOUNCE_MS)
  {
    bool stableOpen = (reading == HIGH);
    if (stableOpen != doorOpen)
    {
      doorOpen = stableOpen;
      digitalWrite(LED_PIN, doorOpen ? HIGH : LOW);
      Serial.println(doorOpen ? "OPEN" : "CLOSED");
    }
  }
}