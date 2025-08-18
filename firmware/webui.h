#ifndef WEBUI_H
#define WEBUI_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
#include "credentials.h"
#include "sdlogger.h"

WebServer server(80);
WebSocketsServer webSocket(81);
String currentDoorState = "UNKNOWN";
unsigned long lastOpenDuration = 0;

String getHTMLHeader(const String& title)
{
    return R"(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>)" + title + R"(</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    .card { border: 1px solid #ddd; border-radius: 5px; padding: 15px; margin-bottom: 20px; }
    .status { font-size: 1.5em; font-weight: bold; padding: 10px; text-align: center; }
    .open { color: white; background-color: #dc3545; }
    .closed { color: white; background-color: #28a745; }
    table { width: 100%; border-collapse: collapse; margin-top: 10px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #f2f2f2; }
    button { padding: 8px 15px; margin: 5px; cursor: pointer; }
    .btn-danger { background-color: #dc3545; color: white; border: none; }
    .time-info { color: #666; font-style: italic; }
  </style>
  <script>
    const socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    
    socket.onmessage = function(event) {
      const [status, duration] = event.data.split('|');
      updateStatus(status, duration);
    };

    function updateStatus(status, duration) {
      const statusEl = document.getElementById('doorStatus');
      statusEl.className = 'status ' + status.toLowerCase();
      statusEl.textContent = 'Door is: ' + status;
      document.getElementById('duration').textContent = 'Last open duration: ' + duration + ' seconds';
      document.getElementById('lastUpdate').textContent = 'Last updated: ' + new Date().toLocaleTimeString();
    }

    function refreshStatus() {
      socket.send('getStatus');
      setTimeout(refreshStatus, 1000);
    }
    
    window.onload = function() {
      refreshStatus();
    };
  </script>
</head>
<body>
)";
}

void handleRoot()
{
    String lowerState = currentDoorState;
    lowerState.toLowerCase();
    
    String html = getHTMLHeader("Door Monitor");
    html += R"(
  <h1>Door Logie</h1>
  <p class="time-info">System time: )" + getTime() + R"(</p>
  
  <div class="card">
    <h2>Current Status</h2>
    <div id="doorStatus" class="status )" + lowerState + R"(">
      Door is: )" + currentDoorState + R"(
    </div>
    <p id="duration">Last open duration: )" + String(lastOpenDuration) + R"( seconds</p>
    <p id="lastUpdate" class="time-info">Last updated: Loading...</p>
  </div>

  <div class="card">
    <h2>Actions</h2>
    <button onclick="location.href='/history'">View Full History</button>
    <button onclick="location.href='/stats'">View Daily Stats</button>
    <button class="btn-danger" onclick="if(confirm('Clear all logs?')) location.href='/clear'">Clear Logs</button>
  </div>
  </body></html>
  )";

    server.send(200, "text/html", html);
}

void handleHistory()
{
    String html = getHTMLHeader("Door History");
    html += "<h1>Door Event History</h1><table><tr><th>Timestamp</th><th>State</th><th>Duration (s)</th></tr>";

    File f = SD.open(LOG_FILE);
    if (f)
    {
        f.readStringUntil('\n');
        while (f.available())
        {
            String line = f.readStringUntil('\n');
            int firstComma = line.indexOf(',');
            int lastComma = line.lastIndexOf(',');
            
            if (firstComma > 0)
            {
                html += "<tr><td>" + line.substring(0, firstComma) + "</td>";
                html += "<td>" + line.substring(firstComma + 1, lastComma) + "</td>";
                html += "<td>" + (lastComma > firstComma ? line.substring(lastComma + 1) : "N/A") + "</td></tr>";
            }
        }
        f.close();
    }
    else
    {
        html += "<tr><td colspan='3'>No history available</td></tr>";
    }
    
    html += "</table><p><a href='/'>Back to Dashboard</a></p></body></html>";
    server.send(200, "text/html", html);
}

void handleStats()
{
    String html = getHTMLHeader("Door Statistics");
    html += "<h1>Daily Open Counts</h1><table><tr><th>Date</th><th>Open Count</th></tr>";

    File f = SD.open(STATS_FILE);
    if (f)
    {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, f);
        f.close();

        if (!error)
        {
            String dates[doc.size()];
            size_t index = 0;
            for (JsonPair kv : doc.as<JsonObject>())
            {
                dates[index++] = kv.key().c_str();
            }
            
            for (size_t i = 0; i < index - 1; i++)
            {
                for (size_t j = i + 1; j < index; j++)
                {
                    if (dates[j] > dates[i])
                    {
                        String temp = dates[i];
                        dates[i] = dates[j];
                        dates[j] = temp;
                    }
                }
            }

            for (size_t i = 0; i < index; i++)
            {
                html += "<tr><td>" + dates[i] + "</td>";
                html += "<td>" + String(doc[dates[i]].as<int>()) + "</td></tr>";
            }
        }
        else
        {
            html += "<tr><td colspan='2'>Error reading stats</td></tr>";
        }
    }
    else
    {
        html += "<tr><td colspan='2'>No statistics available</td></tr>";
    }
    
    html += "</table><p><a href='/'>Back to Dashboard</a></p></body></html>";
    server.send(200, "text/html", html);
}

void handleClear()
{
    if (clearLogs())
    {
        server.send(200, "text/plain", "Logs cleared successfully. Refreshing...<script>setTimeout(()=>location.href='/', 2000)</script>");
    }
    else
    {
        server.send(500, "text/plain", "Error clearing logs");
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
    if (type == WStype_TEXT)
    {
        String message = String((char*)payload);
        if (message == "getStatus")
        {
            String response = currentDoorState + "|" + String(lastOpenDuration);
            webSocket.sendTXT(num, response);
        }
    }
}

void setupWebUI()
{
    if (MDNS.begin("doorlogger"))
    {
        Serial.println("mDNS responder started");
    }
    
    server.on("/", handleRoot);
    server.on("/history", handleHistory);
    server.on("/stats", handleStats);
    server.on("/clear", handleClear);
    server.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void handleWeb()
{
    server.handleClient();
    webSocket.loop();
}

#endif
