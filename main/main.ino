#include <WiFi.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("Hello from OTA!");
  delay(2000);
}
