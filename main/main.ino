#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

// WiFi AP
const char* ssid = "ESP_TEST";
const char* password = "12345678";

// D10 pin (GPIO9)
int testPin = 8;

// ===== HTML =====
String page = R"====(
<!DOCTYPE html>
<html>
<body style="text-align:center;">
<h2>ESP32 D10 TEST + OTA</h2>

<button onclick="fetch('/on')">ON</button>
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

void handleOn() {
  digitalWrite(testPin, HIGH);
  server.send(200, "text/plain", "ON");
}

void handleOff() {
  digitalWrite(testPin, LOW);
  server.send(200, "text/plain", "OFF");
}

// OTA
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

  pinMode(testPin, OUTPUT);
  digitalWrite(testPin, LOW);

  // Strong WiFi
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.softAP(ssid, password);

  Serial.println("WiFi Ready");
  Serial.println(WiFi.softAPIP());

  // Routes
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/update", HTTP_POST, handleUpdate, handleUpload);

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();
}