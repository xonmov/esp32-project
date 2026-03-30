#include <WiFi.h>
#include "ota.h"

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D1; 



void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line


pinMode(motorPin, OUTPUT);


}

void loop() {
  handleOTA();  // 🔥 just one line
delay(1000);
 Serial.println("ok");
  
digitalWrite(motorPin, HIGH);
  delay(1000);
  digitalWrite(motorPin, LOW);
  delay(1000);


}
