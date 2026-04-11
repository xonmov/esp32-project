#include <WiFi.h>
#include "ota.h"
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "POCO";
const char* password = "123456789";

const int motorPin = D1; 

Adafruit_MPU6050 mpu;
bool mpuReady = false; // Flag to track if sensor is working

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  setupOTA();   // 🔥 just one line

  Wire.begin(D3, D2); // SDA = D3, SCL = D2
  delay(500);         // Give it a moment to wake up

  if (mpu.begin(0x68)) {
    Serial.println("MPU6050: OK");
    mpuReady = true;
  } else {
    Serial.println("MPU6050: NOT FOUND (Check wiring/power)");
    // No while loop here; code continues to loop()
  }

}

void loop() {
  handleOTA();  // 🔥 just one line
 if (mpuReady) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    Serial.print("X:"); Serial.print(a.acceleration.x);
    Serial.print(" Y:"); Serial.print(a.acceleration.y);
    Serial.print(" Z:"); Serial.println(a.acceleration.z);
  } else {
    // If not found, just print a warning every second
    Serial.println("Waiting for sensor...");
    delay(1000); 
  }
  
  delay(50); 


}
