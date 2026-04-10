#include <WiFi.h>
#include "ota.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D1; 



void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line

Wire.begin(D3, D2); // Start I2C on your pins
  mpu.begin();        // Initialize the sensor



}

void loop() {
  handleOTA();  // 🔥 just one line
sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Show X and Y acceleration (Tilt)
  Serial.print("X: "); Serial.print(a.acceleration.x);
  Serial.print("  Y: "); Serial.println(a.acceleration.y);
  delay(100);


}
