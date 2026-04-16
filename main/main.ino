#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

// WiFi AP
const char* ssid = "ESP_TEST";
const char* password = "12345678";

// D10 = GPIO9
int pin = 9;
int ch = 0;
int speedVal = 0;

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;">
<h2>PWM TEST (D10) + OTA</h2>

<button onclick="fetch('/low')">LOW</button>
<button onclick="fetch('/mid')">MID</button>
<button onclick="fetch('/high')">HIGH</button>
<button onclick="fetch('/off')">OFF</button>

<hr>

<h3>Upload Firmware (.bin)</h3>
<form method='POST' action='/update' enctype='multipart/form-data'>
<input type='file' name='update'>
<input type='submit' value='Upload'>
</form>

</body>
</html>
)====";

// ===== HANDLERS =====
void handleRoot() {
  server.send(200, "text/html", page);
}

void handleLow() {
  speedVal = 60;
  server.send(200, "text/plain", "LOW");
}

void handleMid() {
  speedVal = 150;
  server.send(200, "text/plain", "MID");
}

void handleHigh() {
  speedVal = 255;
  server.send(200, "text/plain", "HIGH");
}

void handleOff() {
  speedVal = 0;
  server.send(200, "text/plain", "OFF");
}

// ===== OTA =====
void handleUpdate() {
  server.send(200, "text/plain", Update.hasError() ? "FAIL" : "SUCCESS");
  delay(1000);
  ESP.restart();
}

void handleUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    Update.begin(UPDATE_SIZE_UNKNOWN);
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    Update.write(upload.buf, upload.currentSize);
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    Update.end(true);
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  // PWM setup
  ledcSetup(ch, 20000, 8);
  ledcAttachPin(pin, ch);

  // WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());

  // Routes
  server.on("/", handleRoot);
  server.on("/low", handleLow);
  server.on("/mid", handleMid);
  server.on("/high", handleHigh);
  server.on("/off", handleOff);
  server.on("/update", HTTP_POST, handleUpdate, handleUpload);

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();

  // Apply PWM
  ledcWrite(ch, speedVal);
}