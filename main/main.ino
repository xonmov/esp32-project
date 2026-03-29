#include <WiFi.h>
#include "ota.h"

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D0;   // Motor connected to D0
const int pwmChannel = 0;
const int pwmFreq = 10000; // 10 kHz
const int pwmResolution = 8; // 0–255


void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line

ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(motorPin, pwmChannel);

}

void loop() {
  handleOTA();  // 🔥 just one line
delay(1000);
 Serial.println("ok");
  
// Slow increase
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(pwmChannel, duty);
    delay(20);
  }

  // Slow decrease
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(pwmChannel, duty);
    delay(20);
  }


}
