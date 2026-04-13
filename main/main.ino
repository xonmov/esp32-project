#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "app.h"

// ===== WiFi =====
const char* ssid = "POCO";
const char* password = "123456789";

WebServer server(80);

// ===== LOG SYSTEM =====
String logs = "";

void logPrint(String msg) {
  Serial.println(msg);
  logs += msg + "<br>";

  if (logs.length() > 5000) {
    logs = logs.substring(logs.length() - 3000);
  }
}

// ===== OTA =====
void setupOTA() {

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html",
      "<h2>ESP32 OTA</h2>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'>"
      "<input type='submit' value='Upload'>"
      "</form>"
      "<br><a href='/logs'>View Logs</a>");
  });

  server.on("/logs", HTTP_GET, []() {
  String page = "<html><head>";

  // 🔥 auto refresh every 2 seconds
  page += "<meta http-equiv='refresh' content='1'>";

  page += "</head><body>";
  page += "<h2>Logs (Auto Refresh)</h2>";
  page += "<div style='font-family:monospace'>" + logs + "</div>";
  page += "</body></html>";

  server.send(200, "text/html", page);
});

  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", Update.hasError() ? "FAIL" : "SUCCESS");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
      logPrint("Update Start");

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        logPrint("Update Begin Failed");
      }
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } 
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        logPrint("Update Success");
      } else {
        logPrint("Update Failed");
      }
    }
  });

  server.begin();
  logPrint("OTA Ready");
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  logPrint("WiFi Connected");
  logPrint(WiFi.localIP().toString());

  setupOTA();
  appSetup();   // 🔥 from app.h
}

// ===== LOOP =====
void loop() {
  server.handleClient(); // OTA
  appLoop();             // 🔥 from app.h
}