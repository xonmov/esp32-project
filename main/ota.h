#ifndef OTA_H
#define OTA_H

#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer otaServer(80);
String logs = "";

// 🔥 custom log function
void logPrint(String msg) {
  Serial.println(msg);
  logs += msg + "<br>";

  // limit size (important)
  if (logs.length() > 5000) {
    logs = logs.substring(logs.length() - 3000);
  }
}

void setupOTA() {

  // OTA page
  otaServer.on("/", HTTP_GET, []() {
    otaServer.send(200, "text/html",
      "<h2>OTA Update</h2>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'>"
      "<input type='submit' value='Upload'>"
      "</form>"
      "<br><a href='/logs'>View Logs</a>");
  });

  // Logs page
  otaServer.on("/logs", HTTP_GET, []() {
    otaServer.send(200, "text/html",
      "<h2>Logs</h2><div style='font-family:monospace'>" + logs + "</div>");
  });

  // OTA update handler
  otaServer.on("/update", HTTP_POST, []() {
    otaServer.send(200, "text/plain", Update.hasError() ? "FAIL" : "SUCCESS");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = otaServer.upload();

    if (upload.status == UPLOAD_FILE_START) {
      logPrint("OTA Start");
      Update.begin(UPDATE_SIZE_UNKNOWN);
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } 
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        logPrint("OTA Success");
      } else {
        logPrint("OTA Failed");
      }
    }
  });

  otaServer.begin();
  logPrint("OTA Ready");
}

void handleOTA() {
  otaServer.handleClient();
}

#endif
