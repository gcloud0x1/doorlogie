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
return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>)rawliteral" + title + R"rawliteral(</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body 
    {
        font-family: Arial, sans-serif;
        margin: 20px;
    }

    .card
    {
        border: 1px solid #ddd;
        border-radius: 5px;
        padding: 15px;
        margin-bottom: 20px;
    }

    .status
    {
        font-size: 1.5em;
        font-weight: bold;
        padding: 10px;
        text-align: center;
    }

    .open
    {
        color: white;
        background-color: #dc3545;
    }

    .closed
    {
        color: white;
        background-color: #28a745;
    }

    table
    {
        width: 100%;
        border-collapse: collapse;
        margin-top: 10px;
    }

    th,td
    {
        border: 1px solid #ddd;
        padding: 8px;
        text-align: left;
    }

    th
    {
        background-color: #f2f2f2;
    }

    button
    {
        padding: 8px 15px;
        margin: 5px;
        cursor: pointer;
    }

    .btn-danger
    {
        background-color: #dc3545;
        color: white;
        border: none;
    }

    .time-info
    {
        color: #666;
        font-style: italic;
    }

    .chart-container
    {
        position: relative;
        height: 300px;
        width: 100%;
        margin: 20px 0;
    }

    .stats-table
    {
        height: 300px;
        overflow-y: auto;
        margin: 10px 0;
    }

    .dashboard
    {
        display: grid;
        grid-template-columns: repeat(2, 1fr);
        gap: 20px;
    }

    .card-header
    {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-bottom: 15px;
    }

    @media (max-width: 768px)
    {
        .dashboard 
        {
            grid-template-columns: 1fr;
        }
    }

  </style>
  <script>
    const socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    let doorStatsChart = null;
    
    socket.onopen = function()
    {
      console.log('WebSocket connected');
      refreshStatus();
    };

    socket.onmessage = function(event)
    {
      const [status, duration] = event.data.split('|');
      updateStatus(status, duration);
    };

    socket.onerror = function(error)
    {
      console.error('WebSocket error:', error);
    };

    function updateStatus(status, duration) 
    {
      const statusEl = document.getElementById('doorStatus');
      statusEl.className = 'status ' + status.toLowerCase();
      statusEl.textContent = 'Door is: ' + status;
      document.getElementById('duration').textContent = 'Last open duration: ' + duration + ' seconds';
      document.getElementById('lastUpdate').textContent = 'Last updated: ' + new Date().toLocaleTimeString();
    }

    function refreshStatus()
    {
      if (socket.readyState === WebSocket.OPEN)
      {
        socket.send('getStatus');
      } else {
        console.warn('WebSocket not ready, retrying...');
      }
      setTimeout(refreshStatus, 1000);
    }
    
    function loadChartData(attempt = 1, maxAttempts = 3) 
    {
      const fallbackData = {
        '2025-08-18': 3,
        '2025-08-19': 5,
        '2025-08-20': 2
      };

      fetch('/stats-data')
        .then(response => {
          if (!response.ok)
          {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.json();
        })
        .then(data => {
          console.log('Chart data from server:', data);
          const chartData = Object.keys(data).length > 0 ? data : fallbackData;
          const dates = Object.keys(chartData).sort();
          const counts = dates.map(date => chartData[date]);
          
          const ctx = document.getElementById('doorStatsChart')?.getContext('2d');
          if (!ctx) 
          {
            console.error('Canvas element not found');
            document.querySelector('.chart-container').innerHTML = '<p>Chart canvas not found.</p>';
            return;
          }
          if (doorStatsChart) 
          {
            doorStatsChart.destroy();
          }
          
          doorStatsChart = new Chart(ctx, {
            type: 'bar',
            data: {
              labels: dates,
              datasets: [{
                label: 'Number of Door Opens',
                data: counts,
                backgroundColor: 'rgba(54, 162, 235, 0.5)',
                borderColor: 'rgba(54, 162, 235, 1)',
                borderWidth: 1
              }]
            },
            options: {
              responsive: true,
              maintainAspectRatio: false,
              scales: {
                y: {
                  beginAtZero: true,
                  title: { display: true, text: 'Number of Opens' },
                  ticks: { stepSize: 1 }
                },
                x: {
                  title: { display: true, text: 'Date' }
                }
              }
            }
          });
        })
        .catch(error => {
          console.error('Error fetching chart data:', error);
          if (attempt < maxAttempts) 
          {
            console.warn(`Error on attempt ${attempt}, retrying...`);
            setTimeout(() => loadChartData(attempt + 1, maxAttempts), 1000);
          }
          else 
          {
            console.log('Using fallback data due to repeated errors');
            const ctx = document.getElementById('doorStatsChart')?.getContext('2d');
            if (!ctx) 
            {
              document.querySelector('.chart-container').innerHTML = '<p>Chart canvas not found.</p>';
              return;
            }
            if (doorStatsChart)
            {
              doorStatsChart.destroy();
            }
            doorStatsChart = new Chart(ctx, {
              type: 'bar',
              data: {
                labels: Object.keys(fallbackData).sort(),
                datasets: [{
                  label: 'Number of Door Opens (Fallback)',
                  data: Object.values(fallbackData),
                  backgroundColor: 'rgba(54, 162, 235, 0.5)',
                  borderColor: 'rgba(54, 162, 235, 1)',
                  borderWidth: 1
                }]
              },
              options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                  y: { beginAtZero: true, title: { display: true, text: 'Number of Opens' }, ticks: { stepSize: 1 } },
                  x: { title: { display: true, text: 'Date' } }
                }
              }
            });
          }
        });
    }
    
    window.onload = function() {
      if (typeof Chart === 'undefined') 
      {
        console.error('Chart.js failed to load');
        document.querySelector('.chart-container').innerHTML = '<p>Chart.js library failed to load. Please check your internet connection.</p>';
      } else 
      {
        loadChartData();
      }
    };
  </script>
</head>
<body>
)rawliteral";
}

String getHTMLFooter()
{
    return R"rawliteral(
</body>
</html>
)rawliteral";
}

void handleRoot()
{
    String lowerState = currentDoorState;
    lowerState.toLowerCase();
    
    String html = getHTMLHeader("Door Monitor");
    html += R"rawliteral(
  <h1>Door Logie</h1>
  <p class="time-info">System time: )rawliteral" + getTime() + R"rawliteral(</p>
  
  <div class="dashboard">
)rawliteral";

    html += R"rawliteral(
    <div class="card">
      <h2>Door Status</h2>
      <div id="doorStatus" class="status )rawliteral";
    html += lowerState;
    html += R"rawliteral(">Door is: )rawliteral";
    html += currentDoorState;
    html += R"rawliteral(</div>
      <p id="duration">Last open duration: )rawliteral";
    html += String(lastOpenDuration);
    html += R"rawliteral( seconds</p>
      <p id="lastUpdate" class="time-info">Last updated: Loading...</p>
    </div>
)rawliteral";

    html += R"rawliteral(
    <div class="card">
      <h2>System Status</h2>
      <p><strong>WiFi:</strong> )rawliteral";
    html += (WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    html += R"rawliteral(</p>
      <p><strong>IP Address:</strong> )rawliteral";
    html += (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "N/A");
    html += R"rawliteral(</p>
      <p><strong>Time Sync:</strong> )rawliteral";
    html += (timeSynced ? "Synchronized" : "Not Synced");
    html += R"rawliteral(</p>
      <p><strong>SD Card:</strong> )rawliteral";
    html += (SD.exists(LOG_FILE) ? "Available" : "Error");
    html += R"rawliteral(</p>
    </div>
)rawliteral";

    html += R"rawliteral(
    <div class="card">
      <div class="card-header">
        <h2>Open Frequency</h2>
        <button onclick="location.href='/stats'">View Full Table</button>
      </div>
      <div class="chart-container">
        <canvas id="doorStatsChart"></canvas>
      </div>
    </div>
)rawliteral";

    html += R"rawliteral(
    <div class="card">
      <div class="card-header">
        <h2>Recent Activity</h2>
        <div>
          <button onclick="location.href='/history'">Full History</button>
          <button class="btn-danger" onclick="if(confirm('Clear all logs?')) location.href='/clear'">Clear Logs</button>
        </div>
      </div>
      <div class="stats-table">
        <table>
          <tr><th>Timestamp</th><th>State</th><th>Duration</th></tr>
)rawliteral";

    File logFile = SD.open(LOG_FILE);
    int rowCount = 0;
    if (logFile)
    {
        logFile.readStringUntil('\n');
        int fileSize = logFile.size();
        int readPos = max(fileSize - 1024, 0);
        logFile.seek(readPos);
        if (readPos > 0)
        {
            logFile.readStringUntil('\n');
        }
        
        String lines[10];
        while (logFile.available() && rowCount < 10)
        {
            String line = logFile.readStringUntil('\n');
            line.trim();
            if (line.length() > 0 && !line.startsWith("Timestamp") && line.indexOf(',') > 0) 
            {
                lines[rowCount] = line;
                rowCount++;
            }
        }
        logFile.close();
        
        int displayCount = min(rowCount, 5);
        for (int i = rowCount - 1; i >= rowCount - displayCount; i--)
        {
            String line = lines[i];
            int firstComma = line.indexOf(',');
            int lastComma = line.lastIndexOf(',');
            if (firstComma > 0 && lastComma > firstComma)
            {
                html += "<tr><td>" + line.substring(0, firstComma) + "</td>";
                html += "<td>" + line.substring(firstComma + 1, lastComma) + "</td>";
                html += "<td>" + line.substring(lastComma + 1) + "</td></tr>";
            }
        }
    }
    
    if (rowCount == 0)
    {
        html += "<tr><td colspan='3'>No recent activity</td></tr>";
    }
    
    html += R"rawliteral(
        </table>
        </div>
        </div>
        </div>
)rawliteral";

    html += getHTMLFooter();
    server.send(200, "text/html", html);
}

void handleHistory()
{
    String html = getHTMLHeader("Door History");
    html += "<h1>Door Event History</h1><table><tr><th>Timestamp</th><th>State</th><th>Duration (s)</th></tr>";

    File logFile = SD.open(LOG_FILE);
    if (logFile)
    {
        logFile.readStringUntil('\n');
        while (logFile.available())
        {
            String line = logFile.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;
            int firstComma = line.indexOf(',');
            int lastComma = line.lastIndexOf(',');
            if (firstComma > 0 && lastComma > firstComma)
            {
                html += "<tr><td>" + line.substring(0, firstComma) + "</td>";
                html += "<td>" + line.substring(firstComma + 1, lastComma) + "</td>";
                html += "<td>" + line.substring(lastComma + 1) + "</td></tr>";
            }
        }
        logFile.close();
    }
    else
    {
        html += "<tr><td colspan='3'>No history available</td></tr>";
    }
    
    html += "</table><p><a href='/'>Back to Dashboard</a></p>";
    html += getHTMLFooter();
    server.send(200, "text/html", html);
}

void handleStats()
{
    String html = getHTMLHeader("Door Statistics");
    html += "<h1>Daily Open Counts</h1><table><tr><th>Date</th><th>Open Count</th></tr>";

    File statsFile = SD.open(STATS_FILE);
    if (statsFile)
    {
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, statsFile);
        statsFile.close();

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
    
    html += "</table><p><a href='/'>Back to Dashboard</a></p>";
    html += getHTMLFooter();
    server.send(200, "text/html", html);
}

void handleStatsData()
{
    Serial.println("Handling /stats-data request");
    File statsFile = SD.open(STATS_FILE);
    if (!statsFile) 
    {
        Serial.println("Error: STATS_FILE not found");
        server.send(404, "application/json", "{\"error\":\"Stats file not found\"}");
        return;
    }
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, statsFile);
    statsFile.close();
    if (error) 
    {
        Serial.println("Error: JSON parsing failed - " + String(error.c_str()));
        server.send(500, "application/json", "{\"error\":\"Failed to parse stats data\"}");
        return;
    }
    if (doc.size() == 0) 
    {
        Serial.println("Warning: STATS_FILE is empty");
        server.send(200, "application/json", "{}");
        return;
    }
    String jsonStr;
    serializeJson(doc, jsonStr);
    Serial.println("Sending stats data: " + jsonStr);
    server.send(200, "application/json", jsonStr);
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
    server.on("/stats-data", handleStatsData);
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