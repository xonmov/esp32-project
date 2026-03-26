#include <WiFi.h>
#include "ota.h"

const char* ssid = "ESP_TEST";
const char* password = "12345678";

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line
}

void loop() {
  handleOTA();  // 🔥 just one line

  // your project code here
}
