#ifndef OTA_H
#define OTA_H

#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

WebServer otaServer(80);

void setupOTA() {
  otaServer.on("/", HTTP_GET, []() {
    otaServer.send(200, "text/html",
      "<h2>OTA Update</h2>"
      "<form method='POST' action='/update' enctype='multipart/form-data'>"
      "<input type='file' name='update'>"
      "<input type='submit' value='Upload'>"
      "</form>");
  });

  otaServer.on("/update", HTTP_POST, []() {
    otaServer.send(200, "text/plain", Update.hasError() ? "FAIL" : "SUCCESS");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = otaServer.upload();

    if (upload.status == UPLOAD_FILE_START) {
      Serial.println("OTA Start");
      Update.begin(UPDATE_SIZE_UNKNOWN);
    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } 
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.println("OTA Success");
      } else {
        Serial.println("OTA Failed");
      }
    }
  });

  otaServer.begin();
  Serial.println("OTA Ready");
}

void handleOTA() {
  otaServer.handleClient();
}

#endif
