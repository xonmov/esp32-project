#include <WiFi.h>
#include "ota.h"

const char* ssid = "POCO";
const char* password = "123456789";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  logPrint("WiFi Connected");
  logPrint(WiFi.localIP().toString());

  setupOTA();
}

void loop() {
  handleOTA();

  logPrint("Running...");
  delay(2000);
}
