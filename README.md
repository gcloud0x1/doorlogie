# DoorLogie 🚪🔐

DoorLogie is a lightweight door activity monitoring system powered by **ESP32**.  
It uses a reed switch to detect door open/close events, logs them locally, and provides both offline and online access to the data.  

---

## Setup

### 1. Clone this repo
```bash
git clone https://github.com/gcloud0x1/doorlogie
cd doorlogie
```

### 2. Create credentials.h
```bash
touch credentials.h
```

### 3. Add Wi-Fi credentials
```bash
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_CAT_NAME_ASTEROID_DESTROYER";

#endif
```
### 4. Select your board
| Board   | Status       |
|---------|-------------|
| ESP32   | ✅ Supported |
| ESP8266 | ⚠️ Planned   |

### 5. Upload the code
