#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define PIR_PIN 27
#define FAN_LED 18
#define VENT_LED 19
#define SECURITY_LED 21

float temperature = 0;
float humidity = 0;
int motion = LOW;
bool fanOn = false;
bool ventilationOn = false;
bool securityLightOn = false;

String ssid = "";
String password = "";

WebServer server(80);

// ======================================================
// GET WIFI CREDENTIALS - Serial only used here
// ======================================================

void getWiFiCredentials() {
  Serial.println();
  Serial.println("============================================");
  Serial.println("    ESP32 Smart Environment Monitor");
  Serial.println("============================================");
  Serial.println();
  Serial.print("Enter WiFi Name (SSID): ");
  while (ssid == "") {
    if (Serial.available()) {
      ssid = Serial.readStringUntil('\n');
      ssid.trim();
    }
  }
  Serial.println(ssid);

  Serial.print("Enter WiFi Password   : ");
  while (password == "") {
    if (Serial.available()) {
      password = Serial.readStringUntil('\n');
      password.trim();
    }
  }
  Serial.print("[");
  for (int i = 0; i < password.length(); i++) Serial.print("*");
  Serial.println("]");
  Serial.println();
}

// ======================================================
// SENSOR READING - No serial prints, silent
// ======================================================

void readSensors() {
  temperature = dht.readTemperature();
  humidity    = dht.readHumidity();
  motion      = digitalRead(PIR_PIN);

  if (isnan(temperature) || isnan(humidity)) return;

  fanOn          = (temperature > 35);
  ventilationOn  = (humidity > 80);
  securityLightOn = (motion == HIGH);

  digitalWrite(FAN_LED,      fanOn          ? HIGH : LOW);
  digitalWrite(VENT_LED,     ventilationOn  ? HIGH : LOW);
  digitalWrite(SECURITY_LED, securityLightOn ? HIGH : LOW);
}

// ======================================================
// RESET HANDLER
// ======================================================

void handleReset() {
  server.send(200, "text/plain", "Resetting ESP32...");
  delay(500);
  ESP.restart();
}

// ======================================================
// JSON API - Reads sensors fresh on every browser request
// ======================================================

void handleData() {
  readSensors();

  String json = "{";
  json += "\"temperature\":" + String(temperature, 1) + ",";
  json += "\"humidity\":"    + String(humidity, 1)    + ",";
  json += "\"motion\":\""    + String(motion == HIGH ? "Detected" : "No Motion") + "\",";
  json += "\"fan\":\""       + String(fanOn ? "ON" : "OFF")             + "\",";
  json += "\"ventilation\":\"" + String(ventilationOn  ? "ON" : "OFF")  + "\",";
  json += "\"security\":\""  + String(securityLightOn ? "ON" : "OFF")   + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// ======================================================
// DASHBOARD PAGE
// ======================================================

void handleDashboard() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>IoT Dashboard</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jspdf/2.5.1/jspdf.umd.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jspdf-autotable/3.5.28/jspdf.plugin.autotable.min.js"></script>
<style>
* { box-sizing: border-box; margin: 0; padding: 0; }
body {
  font-family: Arial, sans-serif;
  background: #f0f4f8;
  padding: 20px;
  color: #1e293b;
}
h1 { text-align: center; font-size: 24px; margin-bottom: 24px; }
.cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 14px;
  max-width: 1100px;
  margin: auto;
}
.card {
  background: white;
  border-radius: 14px;
  padding: 20px 14px;
  text-align: center;
  box-shadow: 0 4px 14px rgba(0,0,0,0.08);
}
.card h2  { font-size: 14px; color: #64748b; margin-bottom: 10px; }
.value    { font-size: 26px; font-weight: bold; color: #2563eb; }
.on       { color: #16a34a; }
.off      { color: #dc2626; }
.detected { color: #f59e0b; }
.btn-row {
  display: flex;
  justify-content: center;
  gap: 12px;
  flex-wrap: wrap;
  margin: 24px auto;
  max-width: 1100px;
}
button {
  padding: 10px 24px;
  border: none;
  border-radius: 8px;
  font-size: 15px;
  font-weight: bold;
  cursor: pointer;
  transition: opacity 0.2s;
}
button:hover  { opacity: 0.85; }
.btn-pdf      { background: #2563eb; color: white; }
.btn-reset    { background: #dc2626; color: white; }
.btn-clear    { background: #64748b; color: white; }
.section {
  background: white;
  border-radius: 14px;
  padding: 20px;
  max-width: 1100px;
  margin: 20px auto;
  box-shadow: 0 4px 14px rgba(0,0,0,0.08);
}
.section h2 { font-size: 17px; color: #334155; margin-bottom: 16px; }
table { width: 100%; border-collapse: collapse; font-size: 14px; }
th {
  background: #2563eb;
  color: white;
  padding: 10px 8px;
  text-align: center;
}
td { padding: 8px; text-align: center; border-bottom: 1px solid #e2e8f0; }
tr:nth-child(even) { background: #f8fafc; }
tr:hover           { background: #e0f2fe; }
.status { text-align: center; margin-top: 10px; font-size: 13px; color: #94a3b8; }
</style>
</head>
<body>

<h1>&#127760; Smart Environment Monitoring Dashboard</h1>

<div class="cards">
  <div class="card"><h2>&#127777; Temperature</h2><div class="value" id="temperature">--</div></div>
  <div class="card"><h2>&#128167; Humidity</h2><div class="value" id="humidity">--</div></div>
  <div class="card"><h2>&#128694; Motion</h2><div class="value" id="motion">--</div></div>
  <div class="card"><h2>&#127744; Cooling Fan</h2><div class="value" id="fan">--</div></div>
  <div class="card"><h2>&#127787; Ventilation</h2><div class="value" id="ventilation">--</div></div>
  <div class="card"><h2>&#128274; Security Light</h2><div class="value" id="security">--</div></div>
</div>

<div class="btn-row">
  <button class="btn-pdf"   onclick="downloadPDF()">&#11015; Download PDF</button>
  <button class="btn-clear" onclick="clearHistory()">&#128465; Clear History</button>
  <button class="btn-reset" onclick="resetESP()">&#9851; Reset ESP32</button>
</div>

<div class="section">
  <h2>&#128200; Temperature &amp; Humidity Graph</h2>
  <canvas id="myChart" height="100"></canvas>
</div>

<div class="section">
  <h2>&#128203; Data History Table</h2>
  <table id="dataTable">
    <thead>
      <tr>
        <th>Time</th><th>Temp (C)</th><th>Humidity (%)</th>
        <th>Motion</th><th>Fan</th><th>Ventilation</th><th>Security</th>
      </tr>
    </thead>
    <tbody id="tableBody"></tbody>
  </table>
</div>

<p class="status" id="status">Connecting...</p>

<script>
const MAX_POINTS = 30;
let labels    = [];
let tempData  = [];
let humData   = [];
let tableRows = [];

const ctx = document.getElementById('myChart').getContext('2d');
const chart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: labels,
    datasets: [
      {
        label: 'Temperature (C)',
        data: tempData,
        borderColor: '#ef4444',
        backgroundColor: 'rgba(239,68,68,0.1)',
        tension: 0.4, fill: true, pointRadius: 3
      },
      {
        label: 'Humidity (%)',
        data: humData,
        borderColor: '#2563eb',
        backgroundColor: 'rgba(37,99,235,0.1)',
        tension: 0.4, fill: true, pointRadius: 3
      }
    ]
  },
  options: {
    responsive: true,
    animation: false,
    scales: { y: { beginAtZero: false, min: 0, max: 100 } }
  }
});

function updateDashboard() {
  fetch('/data')
    .then(r => r.json())
    .then(data => {
      document.getElementById('temperature').innerHTML = data.temperature + " C";
      document.getElementById('humidity').innerHTML    = data.humidity + " %";

      let motionEl = document.getElementById('motion');
      motionEl.innerHTML = data.motion;
      motionEl.className = 'value ' + (data.motion === 'Detected' ? 'detected' : '');

      let fanEl = document.getElementById('fan');
      fanEl.innerHTML = data.fan;
      fanEl.className = 'value ' + (data.fan === 'ON' ? 'on' : 'off');

      let ventEl = document.getElementById('ventilation');
      ventEl.innerHTML = data.ventilation;
      ventEl.className = 'value ' + (data.ventilation === 'ON' ? 'on' : 'off');

      let secEl = document.getElementById('security');
      secEl.innerHTML = data.security;
      secEl.className = 'value ' + (data.security === 'ON' ? 'on' : 'off');

      let now = new Date().toLocaleTimeString();

      if (labels.length >= MAX_POINTS) {
        labels.shift(); tempData.shift(); humData.shift();
      }
      labels.push(now);
      tempData.push(data.temperature);
      humData.push(data.humidity);
      chart.update();

      tableRows.push({
        time: now, temp: data.temperature, hum: data.humidity,
        motion: data.motion, fan: data.fan, vent: data.ventilation, sec: data.security
      });

      let tbody = document.getElementById('tableBody');
      let row = tbody.insertRow(0);
      row.innerHTML =
        '<td>' + now            + '</td>' +
        '<td>' + data.temperature + '</td>' +
        '<td>' + data.humidity  + '</td>' +
        '<td>' + data.motion    + '</td>' +
        '<td>' + data.fan       + '</td>' +
        '<td>' + data.ventilation + '</td>' +
        '<td>' + data.security  + '</td>';

      document.getElementById('status').innerHTML = 'Last updated: ' + now;
    })
    .catch(() => {
      document.getElementById('status').innerHTML = 'Connection lost...';
    });
}

function clearHistory() {
  labels.length = 0; tempData.length = 0; humData.length = 0;
  tableRows = [];
  chart.update();
  document.getElementById('tableBody').innerHTML = '';
}

function resetESP() {
  if (confirm('Reset ESP32 now?')) {
    fetch('/reset').then(() => {
      document.getElementById('status').innerHTML = 'Resetting... reconnecting in 5s';
      setTimeout(updateDashboard, 5000);
    }).catch(() => {});
  }
}

function downloadPDF() {
  const { jsPDF } = window.jspdf;
  const doc = new jsPDF();
  doc.setFontSize(16);
  doc.text('Smart Environment Monitoring Report', 14, 18);
  doc.setFontSize(11);
  doc.text('Generated: ' + new Date().toLocaleString(), 14, 27);
  let rows = tableRows.map(r => [
    r.time, r.temp + ' C', r.hum + ' %',
    r.motion, r.fan, r.vent, r.sec
  ]);
  doc.autoTable({
    startY: 34,
    head: [['Time','Temp','Humidity','Motion','Fan','Ventilation','Security']],
    body: rows,
    styles: { fontSize: 9, halign: 'center' },
    headStyles: { fillColor: [37, 99, 235] }
  });
  doc.save('environment_report.pdf');
}

setInterval(updateDashboard, 1000);
updateDashboard();
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

// ======================================================
// SETUP - Serial only used for WiFi setup
// ======================================================

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(2000);

  pinMode(PIR_PIN, INPUT);
  pinMode(FAN_LED, OUTPUT);
  pinMode(VENT_LED, OUTPUT);
  pinMode(SECURITY_LED, OUTPUT);

  digitalWrite(FAN_LED, LOW);
  digitalWrite(VENT_LED, LOW);
  digitalWrite(SECURITY_LED, LOW);

  getWiFiCredentials();

  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("============================================");
    Serial.println("          WiFi Connected!");
    Serial.print("  IP Address : ");
    Serial.println(WiFi.localIP());
    Serial.println("  Open this IP in your browser");
    Serial.println("  Serial monitor no longer needed");
    Serial.println("============================================");
  } else {
    Serial.println("============================================");
    Serial.println("  WiFi FAILED! Reset and try again.");
    Serial.print("  Status Code : ");
    Serial.println(WiFi.status());
    Serial.println("============================================");
    while (true) { delay(1000); }
  }

  server.on("/", handleDashboard);
  server.on("/data", handleData);
  server.on("/reset", handleReset);
  server.begin();

  Serial.println("Web Server Started!");
  Serial.println("You can close Serial Monitor now.");
}

// ======================================================
// LOOP - Web requests only, no serial output
// ======================================================

void loop() {
  server.handleClient();
}